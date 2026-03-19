//#include "main.h"   // Required for #define SCHEMAID

#include "../LibCpp/HAL/Tls/cTlsConnectedClient.h"

#include "cIsoProcessSECC.h"

using namespace Iso15118;
using namespace LibCpp;
using namespace std;

cIsoProcessSECC::cIsoProcessSECC(cExiCodec* pCodec, LibCpp::cUdpSocket* pUdp, LibCpp::cTcpServer* pTcp, int schemaId, cControlPilot* pControlPilot, cSeHardware* pSe) :
    dbg("cIsoProcessSECC"),
    pCodec(pCodec),
    pUdp(pUdp),
    pTcp(pTcp),
    pControlPilot(pControlPilot),
    pSe(pSe),
    heardBeatTimer(2000)
    //schemaId(schemaId)
{
    seccInfo.schemaId = schemaId;

    if (pSe)
        this->pSe = pSe;
    else
        this->pSe = &stdSe;

    provideTls = true;
    useTls = false;
    pClient = nullptr;
    // tlsConfig = cTls::stCertFiles("certificates/server-cert.pem", "certificates/server-private.key", "certificates/ca-cert.pem");
    tlsConfig = cTls::stCertFiles("pki/iso15118_2/certs/seccLeafCert.pem", "pki/iso15118_2/private_keys/seccLeaf.key", "pki/iso15118_2/certs/cpoCertChain.pem");
    restart();
}

void cIsoProcessSECC::setupCommunication()
{
    if (pUdp) pUdp->setCallback(this);
}

bool cIsoProcessSECC::isInitialized()
{
    bool ini = true;
    if (pControlPilot)
        ini = ini && pControlPilot->isInitialized();
    ini = ini && pSe->isInitialized();
    return ini;
}

bool cIsoProcessSECC::isOperational()
{
    return !(seccOutput.v2gState == enV2gSeState::finished || seccOutput.v2gState == enV2gSeState::failure);
}

void cIsoProcessSECC::clear()
{
    if (pClient)
    {
        delete pClient;
        pClient = nullptr;
    }
    restart();
}

void cIsoProcessSECC::restart()
{
    seccOutput.v2gState = Iso15118::enV2gSeState::idle;
    enteredV2gState = true;
    dbg.printf(LibCpp::enDebugLevel_Info, "Reset to state %s.", enV2gSeState_toString(seccOutput.v2gState).c_str());
}

void cIsoProcessSECC::setServerIpAddress(LibCpp::stIpAddress serverIpAddress)
{
    this->seccInfo.ipAddress = serverIpAddress;
}

/**
 * @brief Returns the V2G communication state.
 * @return
 */
enV2gSeState cIsoProcessSECC::state()
{
    return seccOutput.v2gState;
}

void cIsoProcessSECC::onFrameReceive(cFramePort* pFramePort)
{
    cDebug dbg("onFrameReceive", &this->dbg);

    char* recvMessage = nullptr;
    int recvMessageLen = 0;
    recvMessageLen = pFramePort->receiveBuffer(&recvMessage);
    if (recvMessageLen == -1)
    {
        dbg.printf(enDebugLevel_Info, "The vehicle closed the communication to the supply equipment.");
        if (pClient)
            delete pClient;
        pClient = nullptr;
    }
    else if (recvMessageLen>0 && (seccOutput.v2gState == enV2gSeState::idle || seccOutput.v2gState == Iso15118::enV2gSeState::finished))
    {
        dbg.printf(enDebugLevel_Error, "Unexpected request message from the vehicle.");
        pClient->receiveAcknowledge();
    }
    else if (recvMessageLen > 0)
    {
        if (pCodec)
            pCodec->decode(receivedMessage, recvMessage, recvMessageLen, seccOutput.v2gState == enV2gSeState::answeredSdp);

        evaluate();

        pFramePort->receiveAcknowledge();
    }
}

/**
 * @brief Stores payload content of all incoming messages being relevant for further processing or monitoring
 */
void cIsoProcessSECC::evaluate()
{
    cDebug dbg("evaluate", &this->dbg);

    if ((receivedMessage.mode != enV2gMode::Req) && (receivedMessage.type != enV2gMsg::V2s))
    {
        dbg.printf(enDebugLevel_Error, "Received a response message, which should not be sent by an EVCC!");
        receivedMessage.set(enV2gMsg::Empty);
        return;
    }

    //responseTimes[seccOutput.v2gState] = responseTimer.get();
    //nextV2gState = seccOutput.v2gState;

    switch (receivedMessage.type)
    {
    // ************ SDP Request **************
    case enV2gMsg::Sdp:
        useTls = (receivedMessage.content.req.sdp.security == V2G_SECURITY_TLS) && provideTls;
        break;
    // ************ V2s Request **************
    case enV2gMsg::V2s:
        break;
    // ************ Supported App Protocol **************
    case enV2gMsg::SupportedAppProtocol:
        evccInfo.schemaId = -1;
        for (stAppProtocolType& dialect : receivedMessage.content.req.supportedAppProtocol.list)
            if (dialect.protocolNamespace == "urn:iso:15118:2:2013:MsgDef")
                evccInfo.schemaId = dialect.schemaID;
        seccInfo.schemaId = evccInfo.schemaId;
        break;
    // ************ Session Setup **************
    case enV2gMsg::SessionSetup:
        evccInfo.evccId = receivedMessage.content.req.sessionSetup.evccId;
    // ************ Service Discovery **************
    case enV2gMsg::ServiceDiscovery:
        break;
    // ************ Payment Service Selection **************
    case enV2gMsg::PaymentServiceSelection:
        break;
    // ************ Service Detail **************
    case enV2gMsg::ServiceDetail:
        break;
    // ************ Authorization **************
    case enV2gMsg::Authorization:
        break;
    // ************ Charge Parameter Discovery **************
    case enV2gMsg::ChargeParameterDiscovery:
        evccInfo.energyTransferMode = receivedMessage.content.req.chargeParameterDiscovery.transferMode;
        evccInfo.energyCapacity = receivedMessage.content.req.chargeParameterDiscovery.energyCapacity.toFloat();
        evccInfo.energyRequest = receivedMessage.content.req.chargeParameterDiscovery.energyRequest.toFloat();
        evccInfo.departureTime = receivedMessage.content.req.chargeParameterDiscovery.departureTime;
        evccInfo.minCurrent = receivedMessage.content.req.chargeParameterDiscovery.minCurrent.toFloat();
        evccInfo.maxCurrent = receivedMessage.content.req.chargeParameterDiscovery.maxCurrent.toFloat();
        evccInfo.maxVoltage = receivedMessage.content.req.chargeParameterDiscovery.maxVoltage.toFloat();
        evccInfo.maxPower = receivedMessage.content.req.chargeParameterDiscovery.maxPower.toFloat();
        break;
    // ************ Power Delivery **************
    case enV2gMsg::PowerDelivery:
        break;
    // ************ Charging Status **************
    case enV2gMsg::ChargingStatus:
        break;
    // ************ Session Stop **************
    case enV2gMsg::SessionStop:
        break;
    // ************ Cable Check **************
    case enV2gMsg::CableCheck:
        break;
    // ************ Precharge **************
    case enV2gMsg::PreCharge:
        evccInfo.targetVoltage = receivedMessage.content.req.preCharge.evTargetVoltage.toFloat();
        evccInfo.memorizedVoltage = evccInfo.targetVoltage;
        break;
    // ************ Current Demand **************
    case enV2gMsg::CurrentDemand:
        seccOutput.demandedCurrent = receivedMessage.content.req.currentDemand.evTargetCurrent.toFloat();
        //float targetVoltage = receivedMessage.content.req.currentDemand.evTargetVoltage.toFloat();
        break;
    // ************ Welding Detection **************
    case enV2gMsg::WeldingDetection:
        break;
    default:
        dbg.printf(LibCpp::enDebugLevel_Error, "Received message is not implemented!");
    }

    responseTimer.start();
}

void cIsoProcessSECC::send(stV2gMessage& sendMsg)
{
    if (!pCodec)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Exi codec is undefined!");
        return;
    }

    int len = pCodec->encode(sendMessage, LibCpp_cIp_TCP_BUFFERSIZE, sendMsg);
    if (sendMsg.type == enV2gMsg::Sdp && pUdp && len>0)
    {
        pUdp->setDestination();
        pUdp->send(sendMessage, len);
    }
    if (sendMsg.type == enV2gMsg::V2s && pUdp && len>0)
    {
        pUdp->setDestination();
        pUdp->send(sendMessage, len);
    }
    else if (pClient)
    {
        pClient->send(sendMessage, len);
    }
    receivedMessage.clear();    // indicate the last received message has been answered
}

void cIsoProcessSECC::prepareCurrentDemandRes(stV2gMessage& msg)
{
    msg.set(enV2gMsg::CurrentDemand, enV2gMode::Res);
    msg.content.res.currentDemand.header.sessionId = seccInfo.sessionId;
    msg.content.res.currentDemand.seccId = seccInfo.seccId;
    msg.content.res.currentDemand.evsePowerLimit.fromFloat(8000, iso1unitSymbolType_W);
    msg.content.res.currentDemand.evseCurrentLimit.fromFloat(3, iso1unitSymbolType_A);
    msg.content.res.currentDemand.evseVoltageLimit.fromFloat(600, iso1unitSymbolType_V);

    if (pSe)
    {
        bool activatePower = true;
        if (pControlPilot)
            if (!(pControlPilot->getState()==enCpState::C_Ready || pControlPilot->getState()==enCpState::D_ReadyVentilation))
                activatePower = false;
        msg.content.res.currentDemand.evsePresentCurrent.fromFloat(pSe->powerSource().get().current, iso1unitSymbolType_A);
        msg.content.res.currentDemand.evsePresentVoltage.fromFloat(pSe->powerSource().get().voltage, iso1unitSymbolType_V);
        pSe->powerSource().set(stPowerControl(activatePower, receivedMessage.content.req.currentDemand.evTargetVoltage.toFloat(), receivedMessage.content.req.currentDemand.evTargetCurrent.toFloat(), true));
    }
    else
    {
        msg.content.res.currentDemand.evsePresentCurrent.fromFloat(seccOutput.demandedCurrent, iso1unitSymbolType_A);
        msg.content.res.currentDemand.evsePresentVoltage.fromFloat(evccInfo.memorizedVoltage, iso1unitSymbolType_V);
    }
}

void cIsoProcessSECC::preparePreChargeRes(stV2gMessage& msg)
{
    msg.set(enV2gMsg::PreCharge, enV2gMode::Res);
    msg.content.res.preCharge.header.sessionId = seccInfo.sessionId;
    //msg.content.res.preCharge.evseIsolationStatus = iso1isolationLevelType_Valid;

    if (pSe)
    {
        bool activatePower = true;
        if (pControlPilot)
            if (!(pControlPilot->getState()==enCpState::C_Ready || pControlPilot->getState()==enCpState::D_ReadyVentilation))
                activatePower = false;
        pSe->powerSource().set(stPowerControl(activatePower, evccInfo.targetVoltage));
        msg.content.res.preCharge.evsePresentVoltage.fromFloat(pSe->powerSource().lastActual().voltage, iso1unitSymbolType_V, -1);
    }
    else
    {
        msg.content.res.preCharge.evsePresentVoltage.fromFloat(evccInfo.memorizedVoltage, iso1unitSymbolType_V);
    }
}

void cIsoProcessSECC::prepareWeldingDetectionRes(stV2gMessage& msg)
{
    msg.set(enV2gMsg::WeldingDetection, enV2gMode::Res);
    msg.content.res.weldingDetection.header.sessionId = seccInfo.sessionId;

    if (pSe)
    {
        pSe->powerSource().set(stPowerControl(false, 0, 0, true));
    }
}

bool cIsoProcessSECC::operate(bool run)
{
    cDebug dbg("operate", &this->dbg);

    if (pClient) pClient->operate();

    if (!(seccOutput.v2gState == enV2gSeState::answeredPreCharge ||
          seccOutput.v2gState == enV2gSeState::answeredPowerDelivery ||
          seccOutput.v2gState == enV2gSeState::answeredCurrentDemand)
       )
    {
        if (pSe)
            pSe->powerSource().set(stPowerControl(false, 0, 0, true));
    }

    bool result = false;
    bool repeat = false;
    do
    {
//        if (repeat)
//            dbg.printf(enDebugLevel_Debug, "Started repeat.");

        enV2gSeState nextV2gState = seccOutput.v2gState;
        bool reenterV2gState = false;

        switch (seccOutput.v2gState)
        {
        case enV2gSeState::idle:
            if(run)
            {
                clear();
                nextV2gState = enV2gSeState::awaitSdp;
            }
            break;
        // ************* await Service Discovery **************
        case enV2gSeState::awaitSdp:
            if(receivedMessage.type == enV2gMsg::Sdp)
            {
                receivedMessage.clear();
                nextV2gState = enV2gSeState::answeredSdp;
            }
            if(receivedMessage.type == enV2gMsg::V2s)
            {
                stV2gMessage message(enV2gMsg::V2s, enV2gMode::Res);
                send(message);
                receivedMessage.clear();
            }
            break;
        // ************* answered Service Discovery **************
        case enV2gSeState::answeredSdp:
            if (enteredV2gState)
            {
                // ----------- SDP Response --------------
                stV2gMessage message(enV2gMsg::Sdp, enV2gMode::Res);

                message.content.res.sdp.serverIpAddress = pTcp->getIpAddressLocal(); // serverIpAddress;
                if (useTls)
                    message.content.res.sdp.security = V2G_SECURITY_TLS;
                else
                    message.content.res.sdp.security = V2G_SECURITY_TCP;
                message.content.res.sdp.transportProtocol = V2G_TRANSPORT_TCP;

                send(message);
            }
            if(receivedMessage.type == enV2gMsg::SupportedAppProtocol)
                nextV2gState = enV2gSeState::answeredSupportedAppProtocol;
            break;
        // ************* answered Supported App Protocol **************
        case enV2gSeState::answeredSupportedAppProtocol:
            if (enteredV2gState)
            {
                // ----------- Supported App Protocol Response --------------
                stV2gMessage message(enV2gMsg::SupportedAppProtocol, enV2gMode::Res);

                if (evccInfo.schemaId != (unsigned int)-1)
                {
                    message.content.res.supportedAppProtocol.responseCode = enSupportedAppProtocolResponseCode_Success;
                    message.content.res.supportedAppProtocol.schemaID = evccInfo.schemaId;
                }
                else
                {
                    dbg.printf(LibCpp::enDebugLevel_Error, "Vehicle does not support available dialects!");
                    message.content.res.supportedAppProtocol.responseCode = enSupportedAppProtocolResponseCode_Failure;
                    message.content.res.supportedAppProtocol.schemaID = 0;
                    nextV2gState = enV2gSeState::failure;
                }

                send(message);
            }
            if(receivedMessage.type == enV2gMsg::SessionSetup)
                nextV2gState = enV2gSeState::answeredSessionSetup;
            break;
        // ************ answered Session Setup **************
        case enV2gSeState::answeredSessionSetup:
            if (enteredV2gState)
            {
                // ----------- Session Setup Response -----------
                uint64_t sessionId = seccInfo.sessionId.asUInt64(true);
                sessionId++;
                seccInfo.sessionId.set(sessionId, true);

                stV2gMessage message(enV2gMsg::SessionSetup, enV2gMode::Res);

                message.content.res.sessionSetup.header.sessionId = seccInfo.sessionId;
                message.content.res.sessionSetup.responseCode = enV2gResponse::OK_NewSessionEstablished;
                message.content.res.sessionSetup.seccId = seccInfo.seccId;
                //message.content.res.sessionSetup.seccTimeStamp = unixTime;

                send(message);
            }
            if(receivedMessage.type == enV2gMsg::ServiceDiscovery)
                nextV2gState = enV2gSeState::answeredServiceDiscovery;
            break;
        // ************ answered Service Discovery **************
        case enV2gSeState::answeredServiceDiscovery:
            if (enteredV2gState)
            {
                // ----------- Service Discovery Response -----------
                stV2gMessage message(enV2gMsg::ServiceDiscovery, enV2gMode::Res);

                message.content.res.serviceDiscovery.header.sessionId = seccInfo.sessionId;

                // DC-core:
    //            if (evccInfo.energyTransferMode != iso1EnergyTransferModeType_AC_three_phase_core)
    //            {
    //                // evccInfo.energyTransferMode = iso1EnergyTransferModeType_DC_extended;
    //                // message.content.res.serviceDiscovery.energyTransferModes.clear();
    //                // message.content.res.serviceDiscovery.energyTransferModes.push_back(evccInfo.energyTransferMode);
    //            }
                message.content.res.serviceDiscovery.energyTransferModes = seccInfo.energyTransferModes;

                send(message);
            }
            if(receivedMessage.type == enV2gMsg::PaymentServiceSelection)
                nextV2gState = enV2gSeState::answeredPaymentServiceSelection;
            if(receivedMessage.type == enV2gMsg::ServiceDetail)
                nextV2gState = enV2gSeState::answeredServiceDetail;
            break;
        // ************ answered Service Detail **************
        case enV2gSeState::answeredServiceDetail:
            if (enteredV2gState)
            {
                // -----------  Service Detail Response -----------
                stV2gMessage message(enV2gMsg::ServiceDetail, enV2gMode::Res);

                message.content.res.serviceDetail.header.sessionId = seccInfo.sessionId;

                send(message);
            }
            if(receivedMessage.type == enV2gMsg::PaymentServiceSelection)
                nextV2gState = enV2gSeState::answeredPaymentServiceSelection;
            break;
        // ************ answered Payment Service Selection **************
        case enV2gSeState::answeredPaymentServiceSelection:
            if (enteredV2gState)
            {
                // ----------- Payment Service Selection Response -----------
                stV2gMessage message(enV2gMsg::PaymentServiceSelection, enV2gMode::Res);

                message.content.res.paymentServiceSelection.header.sessionId = seccInfo.sessionId;

                send(message);
            }
            if(receivedMessage.type == enV2gMsg::Authorization)
                nextV2gState = enV2gSeState::answeredAuthorization;
            break;
        // ************ answered Authorization **************
        case enV2gSeState::answeredAuthorization:
            if(enteredV2gState)
            {
                // -----------  Authorization Response -----------
                stV2gMessage message(enV2gMsg::Authorization, enV2gMode::Res);

                message.content.res.authorization.header.sessionId = seccInfo.sessionId;
                message.content.res.authorization.evseProcessing = iso1EVSEProcessingType_Finished;

    //            if (seccInput.authorized)
    //                message.content.res.authorization.evseProcessing = iso1EVSEProcessingType_Finished;
    //            else
    //                message.content.res.authorization.evseProcessing = iso1EVSEProcessingType_Ongoing_WaitingForCustomerInteraction;

                send(message);
            }
            if(receivedMessage.type == enV2gMsg::Authorization)
            {
                reenterV2gState = true;
            }
            if(receivedMessage.type == enV2gMsg::ChargeParameterDiscovery)
                nextV2gState = enV2gSeState::answeredChargeParameterDiscovery;
            break;
        // ************ answered Charge Parameter Discovery **************
        case enV2gSeState::answeredChargeParameterDiscovery:
            if(enteredV2gState)
            {
                // -----------  Charge Parameter Discovery Response -----------
                stV2gMessage message(enV2gMsg::ChargeParameterDiscovery, enV2gMode::Res);

                message.content.res.chargeParameterDiscovery.header.sessionId = seccInfo.sessionId;
                message.content.res.chargeParameterDiscovery.maxCurrent.fromFloat(seccInfo.maxCurrent, iso1unitSymbolType_A);
                message.content.res.chargeParameterDiscovery.maxVoltage.fromFloat(seccInfo.maxVoltage, iso1unitSymbolType_V);

                if (evccInfo.energyTransferMode != iso1EnergyTransferModeType_AC_three_phase_core)
                {
                    message.content.res.chargeParameterDiscovery.transferMode = evccInfo.energyTransferMode;
                }

                send(message);

                if (pClient)
                    evccInfo.ipAddress = pClient->getIpAddressRemote();
                dbg.printf(LibCpp::enDebugLevel_Info, "\n################################\n%s################################", evccInfo.toString().c_str());
            }
            if(receivedMessage.type == enV2gMsg::PowerDelivery)
                nextV2gState = enV2gSeState::answeredPowerDelivery;
            else if(receivedMessage.type == enV2gMsg::CableCheck)
                nextV2gState = enV2gSeState::answeredCableCheck;
            break;
        // ************ answered Power Delivery **************
        case enV2gSeState::answeredPowerDelivery:
            if(enteredV2gState)
            {
                // ----------- Power Delivery Response -----------
                stV2gMessage message(enV2gMsg::PowerDelivery, enV2gMode::Res);

                message.content.res.powerDelivery.header.sessionId = seccInfo.sessionId;

                if (evccInfo.energyTransferMode != iso1EnergyTransferModeType_AC_three_phase_core)
                {
                    message.content.res.powerDelivery.transferMode = evccInfo.energyTransferMode;
                }

                send(message);
            }
            if(receivedMessage.type == enV2gMsg::ChargeParameterDiscovery)
                nextV2gState = enV2gSeState::answeredChargeParameterDiscovery;
            if(receivedMessage.type == enV2gMsg::ChargingStatus)
                nextV2gState = enV2gSeState::answeredChargingStatus;
            else if(receivedMessage.type == enV2gMsg::SessionStop)
                nextV2gState = enV2gSeState::answeredSessionStop;
            else if(receivedMessage.type == enV2gMsg::CurrentDemand)
                nextV2gState = enV2gSeState::answeredCurrentDemand;
            else if(receivedMessage.type == enV2gMsg::WeldingDetection)
                nextV2gState = enV2gSeState::answeredWeldingDetection;
            break;
        // ************ answered Charging Status **************
        case enV2gSeState::answeredChargingStatus:
            if(enteredV2gState)
            {
                // ----------- Charging Status Response -----------
                stV2gMessage message(enV2gMsg::ChargingStatus, enV2gMode::Res);

                message.content.res.chargingStatus.header.sessionId = seccInfo.sessionId;
                message.content.res.chargingStatus.seccId = seccInfo.seccId;
                message.content.res.chargingStatus.maxCurrent.fromFloat(seccInfo.maxCurrent, iso1unitSymbolType_A);

                send(message);
                nextV2gState = enV2gSeState::answeredChargingStatus;
            }
            if(receivedMessage.type == enV2gMsg::ChargingStatus)
                reenterV2gState = true;
            if(receivedMessage.type == enV2gMsg::PowerDelivery)
                nextV2gState = enV2gSeState::answeredPowerDelivery;
            break;
        // ************ answered Session Stop **************
        case enV2gSeState::answeredSessionStop:
            if(enteredV2gState)
            {
                // ----------- Session Stop Response -----------
                stV2gMessage message(enV2gMsg::SessionStop, enV2gMode::Res);

                message.content.res.sessionStop.header.sessionId = seccInfo.sessionId;

                send(message);
            }
            if (!pClient)
                nextV2gState = enV2gSeState::finished;
            break;
        // ************ answered Cable Check **************
        case enV2gSeState::answeredCableCheck:
            if(enteredV2gState)
            {
                // ----------- Cable Check Response -----------
                stV2gMessage message(enV2gMsg::CableCheck, enV2gMode::Res);

                message.content.res.cableCheck.header.sessionId = seccInfo.sessionId;

                send(message);
                nextV2gState = enV2gSeState::answeredCableCheck;
            }
            if(receivedMessage.type == enV2gMsg::CableCheck)
                reenterV2gState = true;
            else if(receivedMessage.type == enV2gMsg::PreCharge)
                nextV2gState = enV2gSeState::answeredPreCharge;
            break;
        // ************ answered Precharge **************
        case enV2gSeState::answeredPreCharge:
            if(enteredV2gState)
            {
                // ----------- Precharge Response -----------
                stV2gMessage message;

                preparePreChargeRes(message);

                send(message);
            }
            if(receivedMessage.type == enV2gMsg::PreCharge)
                reenterV2gState = true;
            else if(receivedMessage.type == enV2gMsg::PowerDelivery)
                nextV2gState = enV2gSeState::answeredPowerDelivery;
            break;
        // ************ answered Current Demand **************
        case enV2gSeState::answeredCurrentDemand:
            if(enteredV2gState)
            {
                // ----------- Current Demand Response -----------
                stV2gMessage message;

                prepareCurrentDemandRes(message);

                send(message);
            }
            if(receivedMessage.type == enV2gMsg::CurrentDemand)
                reenterV2gState = true;
            else if(receivedMessage.type == enV2gMsg::PowerDelivery)
                nextV2gState = enV2gSeState::answeredPowerDelivery;
            break;
        // ************ answered Welding Detection **************
        case enV2gSeState::answeredWeldingDetection:
            if(enteredV2gState)
            {
                // ----------- Welding Detection Response -----------
                stV2gMessage message;

                prepareWeldingDetectionRes(message);

                send(message);
            }
            if(receivedMessage.type == enV2gMsg::WeldingDetection)
                reenterV2gState = true;
            else if(receivedMessage.type == enV2gMsg::SessionStop)
                nextV2gState = enV2gSeState::answeredSessionStop;
            break;
        // ************ failure **************
        case enV2gSeState::failure:
            if (enteredV2gState)
            {
                if (pClient)
                    delete pClient;
                pClient = nullptr;
            }
            result = true;
            if (!run)
                nextV2gState = enV2gSeState::idle;
            break;
        // ************ finished **************
        case enV2gSeState::finished:
            result = true;
            if (!run)
                nextV2gState = enV2gSeState::idle;
            break;
        default:;
        }

        if (nextV2gState == seccOutput.v2gState && !reenterV2gState && receivedMessage.type != enV2gMsg::Empty)
        {
            receivedMessage.clear();
            dbg.printf(enDebugLevel_Error, "Received unexpected message!");
        }

        if (!pClient &&
            !(seccOutput.v2gState == enV2gSeState::idle ||
              seccOutput.v2gState == enV2gSeState::awaitSdp ||
              seccOutput.v2gState == enV2gSeState::answeredSdp ||
              seccOutput.v2gState == enV2gSeState::answeredSessionStop ||
              seccOutput.v2gState == enV2gSeState::failure ||
              seccOutput.v2gState == enV2gSeState::finished)
           )
        {
            dbg.printf(enDebugLevel_Error, "State machine is running without a client connection!");
            nextV2gState = enV2gSeState::failure;
        }

        enteredV2gState = reenterV2gState;
        if (nextV2gState != seccOutput.v2gState)
        {
            enteredV2gState = true;
            seccOutput.v2gState = nextV2gState;
            dbg.printf(LibCpp::enDebugLevel_Info, "Entered state %s.", enV2gSeState_toString(seccOutput.v2gState).c_str());
        }

//        if (repeat)
//            dbg.printf(enDebugLevel_Debug, "Ended repeat.");

        // Repeat state machine to immediately enter the next state to send the response
        if (enteredV2gState && !repeat)
        {
            repeat = true;
            receivedMessage.clear();
        }
        else
            repeat = false;
    } while(repeat);

    // Heard beat check
//    if (enteredV2gState)
//    {
//        if (seccOutput.v2gState != enV2gSeState::idle && seccOutput.v2gState != enV2gSeState::failure && seccOutput.v2gState != enV2gSeState::finished)
//            heardBeatTimer.start();
//        else
//            heardBeatTimer.stop();
//    }
//    if(heardBeatTimer.elapsed())
//        nextV2gState = enV2gSeState::failure;


    // Main contactor control
    if (pSe && pControlPilot)
    {
        if (evccInfo.energyTransferMode == iso1EnergyTransferModeType_AC_three_phase_core || evccInfo.energyTransferMode == iso1EnergyTransferModeType_AC_single_phase_core)
        {
            if (pControlPilot->getState() == enCpState::C_Ready || pControlPilot->getState() == enCpState::D_ReadyVentilation)
                pSe->mainContactor().set(true);
            else
                pSe->mainContactor().set(false);
        }
    }

    // TCP Server operation
    // - Accepting clients
    cConnectedClient* pNewClient = pTcp->newClient();
    if (pNewClient)
    {
        stIpAddress addr = pNewClient->getIpAddressRemote();
        dbg.printf(enDebugLevel_Info, "Vehicle connection request from %s.", addr.toString(true, true).c_str());

        if (!pClient)
        {
            if (!useTls)
            {   // Unencrypted version:
                pClient = pNewClient;
                pClient->open();             // As the TCP server is configured not to automaticall open new clients due to enabling TLS operation, it needs to be opened manually
            }
            else
            {   // Encrypted version:
                pClient = (cConnectedClient*)cTlsConnectedClient::newClient(pNewClient, tlsConfig);     // pClient takes control over pNewTcpClient and its deletion.
                if (pClient)
                {   // TLS connection has been accepted by client and server
                    dbg.printf(enDebugLevel_Info, "Vehicle established TLS communication.");
                }
                else
                    dbg.printf(enDebugLevel_Error, "TLS handshake between vehicle and supply equipment failed.");
            }
            pClient->setCallback(this);
        }
        else
        {
            dbg.printf(enDebugLevel_Error, "Unexpected connection request to TCP server! Server is already connected to another client.");
            delete pNewClient;
        }
    }
    return result;
}
