#define SDP_REPEAT 1000
#define POWER_REPEAT 1000
#define I_OFF 0.1           ///> Maximum current to open the charging inlet contactor

// #define CODICO
// #define SKIP_PRECHARGE_CHECK     ///< Required for using the EcoG/SwitchEV stack SECC implementation as counterpart. Unfortunately it does not provides the correct expected output voltage during the precharge process and output current within current demand. These values are just set to zero.

#include <cmath>

#include "cIsoProcessEVCC.h"
//#include "cExiIso15118_2013_openV2g.h"

using namespace Iso15118;
using namespace LibCpp;
using namespace std;

//stExiPysicalValue evseActualVoltage;

cIsoProcessEVCC::cIsoProcessEVCC(cExiCodec* pCodec, cUdpSocket* pUdp, cTcpClient* pClient, std::string clientInterface, cControlPilot* pControlPilot, cEvHardware* pEv) :
    dbg("cIsoProcessEVCC"),
    pCodec(pCodec),
    pUdp(pUdp),
    pClient(pClient),
    pControlPilot(pControlPilot),
    evccInfo(true),
    seccInfo(),
    sdpRepeatTimer(SDP_REPEAT),
    powerRepeatTimer(POWER_REPEAT),
    clientInterface(clientInterface)
{
    if (pEv)
        this->pEv = pEv;
    else
        this->pEv = &stdEv;

    //seccInfo.schemaId = 3;

    //evccInfo.energyTransferMode = iso1EnergyTransferModeType_DC_core;
    //energyTransferMode = iso1EnergyTransferModeType_DC_core;
    //evccInput.enable = false;

    clear();
}

void cIsoProcessEVCC::setupCommunication()
{
    if (pUdp) pUdp->setCallback(this);
    if (pClient) pClient->setCallback(this);
}

void cIsoProcessEVCC::clear()
{
    evccOutput.v2gState = Iso15118::enV2gEvState::idle;
    receivedMessage.clear();
    enteredV2gState = false;
    //sessionId = 0;
    sdpRepeatTimer.stop();
    seccInfo.clear();
    pClient->close();
    connecting = false;
}

bool cIsoProcessEVCC::isInitialized()
{
    bool ini = true;
    if (pControlPilot)
        ini = ini && pControlPilot->isInitialized();
    ini = ini && pEv->isInitialized();
    return ini;
}

bool cIsoProcessEVCC::isOperational()
{
    return !(evccOutput.v2gState == enV2gEvState::finished || evccOutput.v2gState == enV2gEvState::failure);
}

/**
 * @brief Returns a pointer to the registered EV Hardware instance.
 * @return Pointer to the EV Hardware, may be zero.
 */
cEvHardware* cIsoProcessEVCC::evHardware()
{
    return pEv;
}

bool cIsoProcessEVCC::responseCodeIsValid(enV2gResponse& code)
{
    return code == enV2gResponse::OK; // || code == enV2gResponse::OK_IsolationWarning (iso2-dialect)
}

bool cIsoProcessEVCC::responseIsValid(enV2gResponse& code, stV2gHeader& v2gHeader)
{
    return responseCodeIsValid(code) && v2gHeader.sessionId == seccInfo.sessionId;
}

void cIsoProcessEVCC::send(stV2gMessage& sendMsg)
{
    if (!pCodec)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Exi codec is undefined!");
        return;
    }

    int len = pCodec->encode(sendMessage, LibCpp_cIp_TCP_BUFFERSIZE, sendMsg);
    if (sendMsg.type == enV2gMsg::Sdp && pUdp && len>0)
    {
        pUdp->send(sendMessage, len);
    }
    else if (pClient)
    {
        pClient->send(sendMessage, len);
    }
    sendMsg.clear();
    responseTimer.start();
}

void cIsoProcessEVCC::onFrameReceive(cFramePort* pFramePort)
{
    cDebug dbg("onFrameReceive", &this->dbg);

    char* recvMessage = nullptr;
    int recvMessageLen = 0;
    recvMessageLen = pFramePort->receiveBuffer(&recvMessage);
    if (recvMessageLen == -1)
    {
        dbg.printf(enDebugLevel_Info, "The supply equipment closed the communication with the vehicle.");
    }
    else if (recvMessageLen>0 && (evccOutput.v2gState == enV2gEvState::idle || evccOutput.v2gState == Iso15118::enV2gEvState::finished))
    {
        dbg.printf(enDebugLevel_Error, "Unexpected response message from the supply equipment.");
        pClient->receiveAcknowledge();
    }
    else if (recvMessageLen > 0)
    {
        if (pCodec && evccOutput.v2gState == enV2gEvState::awaitChargeParameterDiscovery)
            pCodec->decode(receivedMessage, recvMessage, recvMessageLen, evccOutput.v2gState == enV2gEvState::awaitSupportedAppProtocol);

        else if (pCodec)
            pCodec->decode(receivedMessage, recvMessage, recvMessageLen, evccOutput.v2gState == enV2gEvState::awaitSupportedAppProtocol);

        evaluate();

        pClient->receiveAcknowledge();
    }
}

void cIsoProcessEVCC::evaluate()
{
    cDebug dbg("evaluate", &this->dbg);

    if (receivedMessage.mode != enV2gMode::Res)
    {
        dbg.printf(enDebugLevel_Error, "Received a request message, which should not be sent by a SECC!");
        receivedMessage.clear();
        return;
    }

    responseTimer.stop();

    // ************ SDP Response **************
    switch (receivedMessage.type)
    {
    case enV2gMsg::Sdp:
        break;
    case enV2gMsg::V2s:
        break;
    // ************ Supported App Protocol Response **************
    case enV2gMsg::SupportedAppProtocol:
        break;
    // ************ Session Setup Response **************
    case enV2gMsg::SessionSetup:
        if (receivedMessage.content.res.sessionSetup.responseCode == enV2gResponse::OK_NewSessionEstablished)
        {
            seccInfo.sessionId = receivedMessage.content.res.sessionSetup.header.sessionId;
            seccInfo.seccId = receivedMessage.content.res.sessionSetup.seccId;
        }
        break;
    // ************ Service Discovery Response **************
    case enV2gMsg::ServiceDiscovery:
        if (receivedMessage.content.res.serviceDiscovery.responseCode == enV2gResponse::OK)
        {
            seccInfo.chargeService = receivedMessage.content.res.serviceDiscovery.chargeServiceCategory;
            seccInfo.energyTransferModes = receivedMessage.content.res.serviceDiscovery.energyTransferModes;
            seccInfo.paymentOptions = receivedMessage.content.res.serviceDiscovery.paymentOptions;
            seccInfo.chargeServiceId = receivedMessage.content.res.serviceDiscovery.chargeServiceId;
        }
        break;
    // ************ Payment Service Selection Response **************
    case enV2gMsg::PaymentServiceSelection:
        break;
    // ************ Authorization Response **************
    case enV2gMsg::Authorization:
        if (receivedMessage.content.res.authorization.responseCode == enV2gResponse::OK)
            seccInfo.authorized = (receivedMessage.content.res.authorization.evseProcessing == iso1EVSEProcessingType_Finished);
        break;
    // ************ Charge Parameter Discovery Response **************
    case enV2gMsg::ChargeParameterDiscovery:
        if (receivedMessage.content.res.chargeParameterDiscovery.responseCode == enV2gResponse::OK)
        {
            seccInfo.maxVoltage = receivedMessage.content.res.chargeParameterDiscovery.maxVoltage.toFloat();
            seccInfo.nominalVoltage = receivedMessage.content.res.chargeParameterDiscovery.nominalVoltage.toFloat();
            seccInfo.maxCurrent = receivedMessage.content.res.chargeParameterDiscovery.maxCurrent.toFloat();
            seccInfo.minCurrent = receivedMessage.content.res.chargeParameterDiscovery.minCurrent.toFloat();
            seccInfo.maxPower = receivedMessage.content.res.chargeParameterDiscovery.maxPower.toFloat();
        }
        break;
    // ************ Power Delivery Response **************
    case enV2gMsg::PowerDelivery:
        break;
    // ************ Charging Status Response **************
    case enV2gMsg::ChargingStatus:
        break;
    // ************ Cable Check Response **************
    case enV2gMsg::CableCheck:
        if (receivedMessage.content.res.cableCheck.responseCode == enV2gResponse::OK)
        {
            seccInfo.processingState = receivedMessage.content.res.cableCheck.evseProcessing;
        }
        break;
    // ************ Precharge Response **************
    case enV2gMsg::PreCharge:
        if (receivedMessage.content.res.preCharge.responseCode == enV2gResponse::OK)
            evccInfo.memorizedVoltage = receivedMessage.content.res.preCharge.evsePresentVoltage.toFloat();
        break;
    // ************ Current Demand Response **************
    case enV2gMsg::CurrentDemand:
        if (receivedMessage.content.res.currentDemand.responseCode == enV2gResponse::OK)
            evccOutput.actualCurrent = receivedMessage.content.res.currentDemand.evsePresentCurrent.toFloat();
        break;
    // ************ Welding Detection Response **************
    case enV2gMsg::WeldingDetection:
        break;
    // ************ Session Stop Response **************
    case enV2gMsg::SessionStop:
        break;
    default:
        dbg.printf(LibCpp::enDebugLevel_Error, "Received message is not implemented!");
    }
}

bool cIsoProcessEVCC::operate(bool run)
{
    cDebug dbg("operate", &this->dbg);
    
    enV2gEvState nextV2gState = evccOutput.v2gState;
    bool reenterV2gState = false;

    switch (evccOutput.v2gState)
    {
    case enV2gEvState::idle:
        if(run)
        {
            clear();
            nextV2gState = enV2gEvState::awaitSdp;
        }
        break;
    // ************ await SDP Response **************
    case enV2gEvState::awaitSdp:
        if (enteredV2gState) connecting = false;
        if (enteredV2gState || sdpRepeatTimer.elapsed()>0)
        {
            stV2gMessage message(enV2gMsg::Sdp, enV2gMode::Req);
            send(message);
            sdpRepeatTimer.start();
        }
        if (receivedMessage.type == enV2gMsg::Sdp)
        {
            if (!connecting)
            {
                if (receivedMessage.content.res.sdp.security == V2G_SECURITY_TCP && receivedMessage.content.res.sdp.transportProtocol == V2G_TRANSPORT_TCP)
                {
                    dbg.printf(enDebugLevel_Info, "Received SDP response with matching content %s.", receivedMessage.content.res.sdp.toString().c_str());
                    seccInfo.ipAddress = receivedMessage.content.res.sdp.serverIpAddress;
                    connecting = true;
                }
                else
                    dbg.printf(enDebugLevel_Error, "Received non matching SDP response with content %s!", receivedMessage.content.res.sdp.toString().c_str());
                if (connecting)
                {
                    sdpRepeatTimer.stop();
                    pClient->open(seccInfo.ipAddress.port, seccInfo.ipAddress.toString(false, false), 0, clientInterface);
                }
            }
            receivedMessage.clear();
        }
        else if (receivedMessage.type == enV2gMsg::V2s)
        {
            dbg.printf(enDebugLevel_Info, "Received V2S message response.");
            receivedMessage.clear();
        }
        if (connecting)
        {
            if (pClient->isClosed())
                nextV2gState = enV2gEvState::failure;
            else if (pClient->isConnected())
                nextV2gState = enV2gEvState::awaitSupportedAppProtocol;
        }
        break;
    // ************ await Supported App Protocol **************
    case enV2gEvState::awaitSupportedAppProtocol:
        if (enteredV2gState)
        {
            stV2gMessage message(enV2gMsg::SupportedAppProtocol, enV2gMode::Req);
            send(message);
        }
        if (receivedMessage.type == enV2gMsg::SupportedAppProtocol)
        {
            if (receivedMessage.content.res.supportedAppProtocol.responseCode == enSupportedAppProtocolResponseCode_Success)
            {
                seccInfo.schemaId = receivedMessage.content.res.supportedAppProtocol.schemaID;
                // Getting the standard schema ID to verify the schemaID chosen by the supply equipment
                // It will be required later to check according to the implented codecs (each codec nees a stAppProtocolType member).
                stAppProtocolType stdProt;
                if (seccInfo.schemaId != stdProt.schemaID)
                {
                    dbg.printf(enDebugLevel_Info, "\nReceived the choice of an unproposed protocol dialect (schemaID = %i)!", seccInfo.schemaId);
                    nextV2gState = enV2gEvState::failure;
                }
                else
                    nextV2gState = enV2gEvState::awaitSessionSetup;
            }
            else
            {
                dbg.printf(enDebugLevel_Error, "Received a Supported App Protocol message with an unsuccessful response code %i!", receivedMessage.content.res.supportedAppProtocol.responseCode);
                nextV2gState = enV2gEvState::failure;
            }
            receivedMessage.clear();
        }
        break;
    // ************ await Session Setup **************
    case enV2gEvState::awaitSessionSetup:
        if (enteredV2gState)
        {
            stV2gMessage message(enV2gMsg::SessionSetup, enV2gMode::Req);

            message.content.req.sessionSetup.header.sessionId.set((uint64_t)0);

            send(message);
        }
        if (receivedMessage.type == enV2gMsg::SessionSetup)
        {
            if (receivedMessage.content.res.sessionSetup.responseCode == enV2gResponse::OK_NewSessionEstablished)
                nextV2gState = enV2gEvState::awaitServiceDiscovery;
            else
            {
                dbg.printf(enDebugLevel_Error, "Unsuccessful session establishment! Received a Session Setup message with an unsuccessful response code %s!", enV2gResponse_toString(receivedMessage.content.res.sessionSetup.responseCode).c_str());
                nextV2gState = enV2gEvState::failure;
            }
            receivedMessage.clear();
        }
        break;
    // ************ await Service Discovery **************
    case enV2gEvState::awaitServiceDiscovery:
        if (enteredV2gState)
        {
            stV2gMessage message(enV2gMsg::ServiceDiscovery, enV2gMode::Req);

            message.content.req.serviceDiscovery.header.sessionId = seccInfo.sessionId;

            send(message);
        }
        if (receivedMessage.type == enV2gMsg::ServiceDiscovery)
        {
            if (responseIsValid(receivedMessage.content.res.serviceDiscovery.responseCode, receivedMessage.content.res.serviceDiscovery.header))
            {
                bool hasDemandedEnergyTransferMode = false;
                for (iso1EnergyTransferModeType& type : seccInfo.energyTransferModes)
                {
                    if (evccInfo.energyTransferMode == iso1EnergyTransferModeType_DC_core)
                    {
                        if (type == iso1EnergyTransferModeType_DC_core || type == iso1EnergyTransferModeType_DC_extended)
                        {
                            hasDemandedEnergyTransferMode = true;
                            evccInfo.energyTransferMode = type;
                        }
                    }
                    else if (evccInfo.energyTransferMode == type)
                        hasDemandedEnergyTransferMode = true;
                }
                if (hasDemandedEnergyTransferMode)
                    nextV2gState = enV2gEvState::awaitPaymentServiceSelection;
                else
                {
                    dbg.printf(enDebugLevel_Error, "Desired energy transfer mode '%s' is not supported by the supply equipment!", iso1EnergyTransferModeType_toString(evccInfo.energyTransferMode).c_str());
                    nextV2gState = enV2gEvState::failure;
                }
            }
            else
            {
                dbg.printf(enDebugLevel_Error, "Received a Service Discovery message with an unsuccessful response code %s!", enV2gResponse_toString(receivedMessage.content.res.serviceDiscovery.responseCode).c_str());
                nextV2gState = enV2gEvState::failure;
            }
            receivedMessage.clear();
        }
        break;
    // ************ await Payment Service Selection **************
    case enV2gEvState::awaitPaymentServiceSelection:
        if (enteredV2gState)
        {
            stV2gMessage message(enV2gMsg::PaymentServiceSelection, enV2gMode::Req);

            message.content.req.paymentServiceSelection.header.sessionId = seccInfo.sessionId;

            send(message);
        }
        if (receivedMessage.type == enV2gMsg::PaymentServiceSelection)
        {
            if (responseIsValid(receivedMessage.content.res.paymentServiceSelection.responseCode, receivedMessage.content.res.paymentServiceSelection.header))
                nextV2gState = enV2gEvState::awaitAuthorization;
            else
            {
                dbg.printf(enDebugLevel_Error, "Received a Payment Service Selecton message with an unsuccessful response code %s!", enV2gResponse_toString(receivedMessage.content.res.paymentServiceSelection.responseCode).c_str());
                nextV2gState = enV2gEvState::failure;
            }
            receivedMessage.clear();
        }
        break;
    // ************ await Authorization **************
    case enV2gEvState::awaitAuthorization:
        if (enteredV2gState || sdpRepeatTimer.elapsed()>0)
        {
            sdpRepeatTimer.stop();

            stV2gMessage message(enV2gMsg::Authorization, enV2gMode::Req);

            message.content.req.authorization.header.sessionId = seccInfo.sessionId;

            send(message);
        }
        if (receivedMessage.type == enV2gMsg::Authorization)
        {
            sdpRepeatTimer.stop();
            if (responseIsValid(receivedMessage.content.res.authorization.responseCode, receivedMessage.content.res.authorization.header))
            {
                if (seccInfo.authorized)
                {
                    nextV2gState = enV2gEvState::awaitChargeParameterDiscovery;
                }
                else
                    sdpRepeatTimer.start();
            }
            else
            {
                dbg.printf(enDebugLevel_Error, "Received a Authorization message with an unsuccessful response code %s!", enV2gResponse_toString(receivedMessage.content.res.authorization.responseCode).c_str());
                nextV2gState = enV2gEvState::failure;
            }
            receivedMessage.clear();
        }
        break;
    // ************ await Charge Parameter Discovery **************
    case enV2gEvState::awaitChargeParameterDiscovery:
        if (enteredV2gState)
        {
            stV2gMessage message(enV2gMsg::ChargeParameterDiscovery, enV2gMode::Req);

            message.content.req.chargeParameterDiscovery.header.sessionId = seccInfo.sessionId;
            message.content.req.chargeParameterDiscovery.transferMode = evccInfo.energyTransferMode;

            message.content.req.chargeParameterDiscovery.maxVoltage.fromFloat(evccInfo.maxVoltage, iso1unitSymbolType_V);
            message.content.req.chargeParameterDiscovery.maxCurrent.fromFloat(evccInfo.maxCurrent, iso1unitSymbolType_A);
            message.content.req.chargeParameterDiscovery.minCurrent.fromFloat(evccInfo.minCurrent, iso1unitSymbolType_A);
            message.content.req.chargeParameterDiscovery.energyCapacity.fromFloat(evccInfo.energyCapacity, iso1unitSymbolType_Wh);
            //message.content.req.chargeParameterDiscovery.energyRequest.fromFloat(evccInfo.energyRequest, iso1unitSymbolType_W);
            message.content.req.chargeParameterDiscovery.energyRequest.fromFloat(5000, iso1unitSymbolType_Wh);
            message.content.req.chargeParameterDiscovery.dcEvStatus.ressSOC = 50;
            message.content.req.chargeParameterDiscovery.departureTime = evccInfo.departureTime;

            send(message);
        }
        if (receivedMessage.type == enV2gMsg::ChargeParameterDiscovery)
        {
            if (responseIsValid(receivedMessage.content.res.chargeParameterDiscovery.responseCode, receivedMessage.content.res.chargeParameterDiscovery.header))
            {
                if (evccInfo.energyTransferMode == iso1EnergyTransferModeType_AC_three_phase_core)
                    nextV2gState = enV2gEvState::awaitPowerDelivery;
                else
                    nextV2gState = enV2gEvState::awaitCableCheck;

            }
            else
            {
                dbg.printf(enDebugLevel_Error, "Received a Payment Charge Parameter Discovery message with an unsuccessful response code %s!", enV2gResponse_toString(receivedMessage.content.res.chargeParameterDiscovery.responseCode).c_str());
                nextV2gState = enV2gEvState::failure;
            }
            receivedMessage.clear();
        }
        break;
    // ************ await Power Delivery **************
    case enV2gEvState::awaitPowerDelivery:
        if (enteredV2gState)
        {
            stV2gMessage message(enV2gMsg::PowerDelivery, enV2gMode::Req);

            message.content.req.powerDelivery.header.sessionId = seccInfo.sessionId;
            message.content.req.powerDelivery.transferMode = evccInfo.energyTransferMode;

            message.content.req.powerDelivery.saScheduleTupleId = 1;
            message.content.req.powerDelivery.chargingProfile_isUsed = 1;
            // Profiles müssen noch in einem Vektor definiert und gesetzt werden!
            message.content.req.powerDelivery.evStatus.EVRESSSOC = 50;
            if (!evccInput.enable)
                message.content.req.powerDelivery.chargeProgress = iso1chargeProgressType_Stop;

            send(message);
        }
        if (receivedMessage.type == enV2gMsg::PowerDelivery)
        {
            if (responseIsValid(receivedMessage.content.res.powerDelivery.responseCode, receivedMessage.content.res.powerDelivery.header))
            {
                if (evccInput.enable)
                {
                    if (evccInfo.energyTransferMode == iso1EnergyTransferModeType_AC_three_phase_core)
                        nextV2gState = enV2gEvState::awaitChargingStatus;
                    else
                        nextV2gState = enV2gEvState::awaitCurrentDemand;
                }
                else
                {
                    dbg.printf(enDebugLevel_Info, "Charging is not enabled, entering session stop / welding detection.");
                    if (evccInfo.energyTransferMode == iso1EnergyTransferModeType_AC_three_phase_core)
                        nextV2gState = enV2gEvState::awaitSessionStop;
                    else
                        nextV2gState = enV2gEvState::awaitWeldingDetection;
                }
            }
            else
            {
                dbg.printf(enDebugLevel_Error, "Received a Power Delivery message with an unsuccessful response code %s!", enV2gResponse_toString(receivedMessage.content.res.powerDelivery.responseCode).c_str());
                nextV2gState = enV2gEvState::failure;
            }
            receivedMessage.clear();
        }
        break;
    // ************ await Charging Status **************
    case enV2gEvState::awaitChargingStatus:
        if (enteredV2gState || sdpRepeatTimer.elapsed()>0)
        {
            sdpRepeatTimer.stop();

            stV2gMessage message(enV2gMsg::ChargingStatus, enV2gMode::Req);

            message.content.req.chargingStatus.header.sessionId = seccInfo.sessionId;

            send(message);
        }
        if (receivedMessage.type == enV2gMsg::ChargingStatus)
        {
            sdpRepeatTimer.stop();
            if (responseIsValid(receivedMessage.content.res.chargingStatus.responseCode, receivedMessage.content.res.chargingStatus.header))
            {
                if (!evccInput.enable)
                    nextV2gState = enV2gEvState::awaitPowerDelivery;
                else
                    sdpRepeatTimer.start();
            }
            else
            {
                dbg.printf(enDebugLevel_Error, "Received a Charging Status message with an unsuccessful response code %s!", enV2gResponse_toString(receivedMessage.content.res.chargingStatus.responseCode).c_str());
                nextV2gState = enV2gEvState::failure;
            }
            receivedMessage.clear();
        }
        break;
    // ************ await Cable check **************
    case enV2gEvState::awaitCableCheck:
        if (enteredV2gState || sdpRepeatTimer.elapsed()>0)
        {
            sdpRepeatTimer.stop();

            if (pControlPilot)
                pControlPilot->setState(enCpState::C_Ready);

            stV2gMessage message(enV2gMsg::CableCheck, enV2gMode::Req);

            message.content.req.cableCheck.header.sessionId = seccInfo.sessionId;

            message.content.req.cableCheck.evRessSoc = 50;

            send(message);
        }
        if (receivedMessage.type == enV2gMsg::CableCheck)
        {
            sdpRepeatTimer.stop();
            if (responseIsValid(receivedMessage.content.res.cableCheck.responseCode, receivedMessage.content.res.cableCheck.header))
            {
                if (seccInfo.processingState == iso1EVSEProcessingType_Finished)
                {
                    nextV2gState = enV2gEvState::awaitPreCharge;
                }
                else
                    sdpRepeatTimer.start(200);

            }
            else
            {
                dbg.printf(enDebugLevel_Error, "Received a Cable Check message with an unsuccessful response code %s!", enV2gResponse_toString(receivedMessage.content.res.cableCheck.responseCode).c_str());
                nextV2gState = enV2gEvState::failure;
            }
            receivedMessage.clear();
        }
        break;
    // ************ await Precharge **************
    case enV2gEvState::awaitPreCharge:

        if (enteredV2gState && pControlPilot)
            pControlPilot->setState(enCpState::C_Ready);

        if (enteredV2gState || sdpRepeatTimer.elapsed()>0)
        {
            sdpRepeatTimer.stop();

            stV2gMessage message(enV2gMsg::PreCharge, enV2gMode::Req);

            message.content.req.preCharge.header.sessionId = seccInfo.sessionId;

            stExiPysicalValue value;
            value.fromFloat(evccInfo.memorizedVoltage, iso1unitSymbolType_V);
            message.content.req.preCharge.evTargetVoltage = value;
            value.fromFloat(1, iso1unitSymbolType_A);
            message.content.req.preCharge.evTargetCurrent = value;
            message.content.req.preCharge.evRessSoc = 50;

            send(message);
        }
        if (receivedMessage.type == enV2gMsg::PreCharge)
        {
            sdpRepeatTimer.stop();

            if (responseIsValid(receivedMessage.content.res.preCharge.responseCode, receivedMessage.content.res.preCharge.header))
            {
                if (pEv)
                {
                    pEv->socketVoltage().value = (uint16_t)evccInfo.memorizedVoltage;    // Necessary in case the EV has not its own socket voltage measurement. If it has, this command will have no effect.
                    #ifdef SKIP_PRECHARGE_CHECK         // Necessary to function with secc Switch python stack
                    if (pEv == &stdEv)
                        pEv->socketVoltage().value = evccInfo.actualVoltage;    // Necessary in case the EV has not its own socket voltage measurement. If it has, this command will have no effect.
                    #endif
                    if (fabs(evccInfo.memorizedVoltage - pEv->socketVoltage().get()) < 5)
                    {
                        nextV2gState = enV2gEvState::awaitPowerDelivery;
                    }
                    else
                        sdpRepeatTimer.start();
                }
                else
                    nextV2gState = enV2gEvState::awaitPowerDelivery;
            }
            else
            {
                dbg.printf(enDebugLevel_Error, "Received a Precharge message with an unsuccessful response code %s!", enV2gResponse_toString(receivedMessage.content.res.preCharge.responseCode).c_str());
                nextV2gState = enV2gEvState::failure;
            }
            receivedMessage.clear();
        }
        break;
    // ************ await Current Demand **************
    case enV2gEvState::awaitCurrentDemand:

        if (pEv && enteredV2gState)
            pEv->socketContactor().on();

        if (enteredV2gState || powerRepeatTimer.elapsed() > 0)
        {
            powerRepeatTimer.stop();

            stV2gMessage message(enV2gMsg::CurrentDemand, enV2gMode::Req);

            message.content.req.currentDemand.header.sessionId = seccInfo.sessionId;

            if (evccInput.enable && pEv->socketContactor().get())
                message.content.req.currentDemand.evTargetCurrent.fromFloat(10, iso1unitSymbolType_A);
            else
                message.content.req.currentDemand.evTargetCurrent.fromFloat(0, iso1unitSymbolType_A);
            message.content.req.currentDemand.evTargetVoltage.fromFloat(evccInfo.maxVoltage, iso1unitSymbolType_V);
            message.content.req.currentDemand.evVoltageLimit.fromFloat(evccInfo.maxVoltage, iso1unitSymbolType_V);
            message.content.req.currentDemand.evCurrentLimit.fromFloat(evccInfo.maxCurrent, iso1unitSymbolType_A);
            message.content.req.currentDemand.evPowerLimit.fromFloat(10000, iso1unitSymbolType_W);
            message.content.req.currentDemand.evRessSoc = 50;

            send(message);
        }
        if (receivedMessage.type == enV2gMsg::CurrentDemand)
        {
            powerRepeatTimer.stop();

            if (responseIsValid(receivedMessage.content.res.currentDemand.responseCode, receivedMessage.content.res.currentDemand.header))
            {
                if (pEv)
                {
                    pEv->socketCurrent().value = (uint16_t)evccOutput.actualCurrent;    // Necessary in case the EV has not its own charge current measurement. If it has, this command will have no effect.
                    #ifdef SKIP_PRECHARGE_CHECK         // Necessary to function with secc Switch python stack. It does not send back the demanded current.
                    if (!evccInput.enable && pEv == &stdEv)
                        pEv->socketCurrent().value = 0;    // Necessary in case the EV has not its own socket voltage measurement. If it has, this command will have no effect.
                    #endif
                }

                if (evccInput.enable || (!evccInput.enable && fabs(pEv->socketCurrent().get()) >= I_OFF))
                    powerRepeatTimer.start();
                else
                {
                    dbg.printf(enDebugLevel_Info, "Application demanded to stop charging.");
                    nextV2gState = enV2gEvState::awaitPowerDelivery;
                }
            }
            else
            {
                    dbg.printf(enDebugLevel_Error, "Received a Current Demand message with an unsuccessful response code %s!", enV2gResponse_toString(receivedMessage.content.res.currentDemand.responseCode).c_str());
                    nextV2gState = enV2gEvState::failure;
            }
            receivedMessage.clear();
        }
        break;
    // ************ await Welding Detection **************
    case enV2gEvState::awaitWeldingDetection:
        if (enteredV2gState)
        {
            stV2gMessage message(enV2gMsg::WeldingDetection, enV2gMode::Req);

            message.content.req.weldingDetection.header.sessionId = seccInfo.sessionId;

            send(message);
        }
        if (receivedMessage.type == enV2gMsg::WeldingDetection)
        {
            if (responseIsValid(receivedMessage.content.res.weldingDetection.responseCode, receivedMessage.content.res.weldingDetection.header))
            {
                nextV2gState = enV2gEvState::awaitSessionStop;
            }
            else
            {
                dbg.printf(enDebugLevel_Error, "Received a Welding Detection message with an unsuccessful response code %s!", enV2gResponse_toString(receivedMessage.content.res.weldingDetection.responseCode).c_str());
                nextV2gState = enV2gEvState::failure;
            }
            receivedMessage.clear();
        }
        break;
    // ************ await Session Stop **************
    case enV2gEvState::awaitSessionStop:
        if (enteredV2gState)
        {
            stV2gMessage message(enV2gMsg::SessionStop, enV2gMode::Req);

            message.content.req.sessionStop.header.sessionId = seccInfo.sessionId;

            send(message);
        }
        if (receivedMessage.type == enV2gMsg::SessionStop)
        {
            if (responseIsValid(receivedMessage.content.res.paymentServiceSelection.responseCode, receivedMessage.content.res.paymentServiceSelection.header))
            {
                nextV2gState = enV2gEvState::awaitDisconnection;
            }
            else
            {
                dbg.printf(enDebugLevel_Error, "Received a Session Stop message with an unsuccessful response code %s!", enV2gResponse_toString(receivedMessage.content.res.sessionStop.responseCode).c_str());
                nextV2gState = enV2gEvState::failure;
            }
            receivedMessage.clear();
        }
        break;
    // ************ await Disconnection **************
    case enV2gEvState::awaitDisconnection:
        if (pClient)
        {
            if (!pClient->isConnected())
                nextV2gState = enV2gEvState::finished;
            else
                pClient->close();
        }
        else
            nextV2gState = enV2gEvState::finished;
        break;
    // ************ failure **************
    case enV2gEvState::failure:
        if (enteredV2gState)
        {
            if (pClient)
                pClient->close();
        }
        if (!run)
            nextV2gState = enV2gEvState::idle;
        break;
    // ************ finished **************
    case enV2gEvState::finished:
        if (!run)
            nextV2gState = enV2gEvState::idle;
        break;
    default:;
    }

    if (receivedMessage.type != enV2gMsg::Empty)
    {
        receivedMessage.clear();
        dbg.printf(enDebugLevel_Error, "Received unexpected message!");
    }

    if (!(evccOutput.v2gState == enV2gEvState::awaitCableCheck || evccOutput.v2gState == enV2gEvState::awaitPreCharge || evccOutput.v2gState == enV2gEvState::awaitPowerDelivery || evccOutput.v2gState == enV2gEvState::awaitCurrentDemand))
    {
        if (pControlPilot)
            if (pControlPilot->getDemandState() == enCpState::C_Ready || pControlPilot->getDemandState() == enCpState::D_ReadyVentilation)
                pControlPilot->setState(enCpState::B_Connected);
        if (pEv)
        {
            if (pEv->socketCurrent().get() < I_OFF && pEv->socketContactor().getDesired() == true)
                pEv->socketContactor().off();
        }
    }

    if (!pClient->isConnected() &&
        !(evccOutput.v2gState == enV2gEvState::idle ||
          evccOutput.v2gState == enV2gEvState::awaitSdp ||
          evccOutput.v2gState == enV2gEvState::awaitDisconnection ||
          evccOutput.v2gState == enV2gEvState::failure ||
          evccOutput.v2gState == enV2gEvState::finished)
       )
    {
        dbg.printf(enDebugLevel_Error, "State machine is running without a connected client!");
        nextV2gState = enV2gEvState::failure;
    }
    enteredV2gState = reenterV2gState;
    if (nextV2gState != evccOutput.v2gState)
    {
        enteredV2gState = true;
        dbg.printf(LibCpp::enDebugLevel_Info, "Entered state %s.", enV2gEvState_toString(nextV2gState).c_str());
    }
    bool result = (evccOutput.v2gState == enV2gEvState::finished) || (evccOutput.v2gState == enV2gEvState::failure);
    evccOutput.v2gState = nextV2gState;

    return result;
}
