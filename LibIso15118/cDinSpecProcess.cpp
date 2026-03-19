#include "main.h"
#include "cIsoProcessSECC.h"

using namespace Iso15118;
using namespace LibCpp;
using namespace std;

float evseMemorizedVoltage;

cIsoProcessSECC::cIsoProcessSECC(cControlPilot* pControlPilot, LibCpp::cPowerSource* pPowerSource) :
    dbg("cIso15118Process")
    //pControlPilot(pControlPilot),
    //pPowerSource(pPowerSource)
{
    this->pControlPilot = pControlPilot;
    this->pPowerSource = pPowerSource;
    seccId.set(7, "ZZ00001");
    sessionId = 0x0102030405060708;
    reset();
}

void cIsoProcessSECC::reset()
{
    v2gState = Iso15118::enV2gState_idle;
    nextV2gState = v2gState;
    chargingStatusPrinted = 0;
}

void cIsoProcessSECC::setServerIpAddress(LibCpp::stIpAddress serverIpAddress)
{
    this->serverIpAddress = serverIpAddress;
}

int cIsoProcessSECC::evaluate(char* receiveMessage, int receiveMessageLen)
{
    cDebug dbg("evaluate", &this->dbg);

    //responseTimes[v2gState] = responseTimer.get();
    //nextV2gState = v2gState;

    stStreamControl iStream = stStreamControl(receiveMessage, receiveMessageLen);
    stStreamControl oStream = stStreamControl(sendMessage, sizeof(sendMessage));

    int  len = 0;
    bool ok = false;

    cV2gTpHeader v2gTpHeader;
    ok = read_V2gTpHeader(v2gTpHeader, iStream);
    if (!ok)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received V2G message with invalid header!");
        return 0;
    }

    // ************ SDP Request **************
    if (v2gTpHeader.payloadType == V2GTP_TYPE_SDP_REQ)
    {
        cV2gSdpReq v2gSdpReq;
        cV2gSdpRes v2gSdpRes;

        dbg.printf(enDebugLevel_Info, "\nReceived SDP request.");

        ok = read_V2gSdpReq(v2gSdpReq, v2gTpHeader, iStream);
        if (!ok)
        {
            dbg.printf(enDebugLevel_Error, "Received SDP request with invalid format!");
            return 0;
        }
        dbg.printf(enDebugLevel_Debug, "Received SDP request with content %s and header %s", v2gSdpReq.toString().c_str(), v2gTpHeader.toString().c_str());

        if (v2gState == Iso15118::enV2gState_awaitSdp)
        {
            // ************ SDP Response **************
            v2gSdpRes.serverIpAddress = serverIpAddress;
            v2gSdpRes.security = V2G_SECURITY_TCP;
            v2gSdpRes.transportProtocol = V2G_TRANSPORT_TCP;

            len = write_V2gSdpRes(v2gSdpRes, oStream);

            dbg.printf(enDebugLevel_Info, "\nPrepared SDP response %s", v2gSdpRes.toString().c_str());

            nextV2gState = Iso15118::enV2gState_awaitSupportedAppProtocol;
            return len;
        }
        else
            dbg.printf(enDebugLevel_Error, "\nReceived unexpected SDP request with content %s and header %s", v2gSdpReq.toString().c_str(), v2gTpHeader.toString().c_str());
        return 0;
    }

    if (v2gTpHeader.payloadType != V2GTP_TYPE_EXI)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received V2G message without EXI encoding!");
        return 0;
    }

    switch(v2gState)
    {
        case enV2gState_idle: break;
        case enV2gState_awaitSdp: break;
        case enV2gState_awaitSupportedAppProtocol:
        {
            // ************ Supported App Protocol **************
            stV2gSupportedAppProtocolReq suppAppReq;
            stV2gSupportedAppProtocolRes suppAppRes;

            ok = read_V2gSupportedAppProtocolReq(suppAppReq, iStream);
            if (ok)
            {
                dbg.printf(enDebugLevel_Info, "\nReceived request %s", suppAppReq.toString().c_str());

                // ************ Supported App Protocol Response **************
                suppAppRes.responseCode = enSupportedAppProtocolResponseCode_Success;
#ifdef CODICO
                suppAppRes.schemaID = suppAppReq.list[2].schemaID;
#else
                suppAppRes.schemaID = suppAppReq.list[0].schemaID;
#endif

                iStream.set(sendMessage, sizeof(sendMessage));
                len = write_V2gSupportedAppProtocolRes(suppAppRes, iStream);

            }
            else
            {
                dbg.printf(enDebugLevel_Error, "Received message is not a valid 'SupportedAppProtocol' message!");
            }

            if (len)
            {
                dbg.printf(enDebugLevel_Info, "\nPrepared response %s", suppAppRes.toString().c_str());
                nextV2gState = enV2gState_awaitSessionSetup;
            }
        } break;
    case enV2gState_awaitSessionSetup:
    {
        // ************ Session Setup **************
        stV2gHeader header;
        stV2gSessionSetupReq sessionSetupReq;
        stV2gSessionSetupRes sessionSetupRes;

        read_V2gHeader(header, iStream, receivedExiDocument);
        if (header.messageType == enV2gMsg_SessionSetup)
        {
            if (read_V2gSessionSetupReq(sessionSetupReq, header, receivedExiDocument))
            {
                dbg.printf(enDebugLevel_Info, "\nReceived request %s", sessionSetupReq.toString().c_str());

                evccId = sessionSetupReq.evccId;
                sessionId++;

                // ************ Session Setup Response **************

//                            uint64_t session_id = sessionId;
//                            BigEndian(&session_id);
//                            sessionSetupRes.header.sessionId.set(8, (char*)&session_id);
                sessionSetupRes.header.sessionId.set(sessionId);
                sessionSetupRes.responseCode = enV2gResponse_OK_NewSessionEstablished;
                sessionSetupRes.seccId = seccId;
                //sessionSetupRes.seccTimeStamp = unixTime;

                iStream.set(sendMessage, sizeof(sendMessage));
                len = write_V2gSessionSetupRes(sessionSetupRes, iStream);
            }
        }
        else
        {
            dbg.printf(enDebugLevel_Error, "Received message %s is not an expected %s message!", header.toString().c_str(), enV2gMsg_toString(enV2gMsg_SessionSetup).c_str());
        }

        if (len)
        {
            dbg.printf(enDebugLevel_Info, "\nPrepared response %s", sessionSetupRes.toString().c_str());
            nextV2gState = enV2gState_awaitServiceDiscovery;
        }
    } break;
    case enV2gState_awaitServiceDiscovery:
    {
        // ************ Service Discovery **************
        stV2gHeader header;
        stV2gServiceDiscoveryReq serviceDiscoveryReq;
        stV2gServiceDiscoveryRes serviceDiscoveryRes;

        read_V2gHeader(header, iStream, receivedExiDocument);
        if (header.messageType == enV2gMsg_ServiceDiscovery)
        {
            if (read_V2gServiceDiscoveryReq(serviceDiscoveryReq, header, receivedExiDocument))
            {
                dbg.printf(enDebugLevel_Info, "\nReceived request %s", serviceDiscoveryReq.toString().c_str());

                // ************ Service Discovery Response **************

                serviceDiscoveryRes.header.sessionId.set(sessionId);
                // DC-core:
                if (energyTransferMode != iso1EnergyTransferModeType_AC_three_phase_core)
                {
                    serviceDiscoveryRes.energyTransferModes.clear();
                    serviceDiscoveryRes.energyTransferModes.push_back(energyTransferMode);
                }

                iStream.set(sendMessage, sizeof(sendMessage));
                len = write_V2gServiceDiscoveryRes(serviceDiscoveryRes, iStream);
            }
        }
        else
        {
            dbg.printf(enDebugLevel_Error, "Received message %s is not an expected %s message!", header.toString().c_str(), enV2gMsg_toString(enV2gMsg_ServiceDiscovery).c_str());
        }

        if (len)
        {
            dbg.printf(enDebugLevel_Info, "\nPrepared response %s", serviceDiscoveryRes.toString().c_str());
            nextV2gState = enV2gState_awaitPaymentServiceSelection;
        }
    } break;
    case enV2gState_awaitPaymentServiceSelection:
    {
        // ************ Payment Service Selection **************
        stV2gHeader header;

        read_V2gHeader(header, iStream, receivedExiDocument);
        if (header.messageType == enV2gMsg_PaymentServiceSelection)
        {
            stV2gPaymentServiceSelectionReq paymentServiceSelectionReq;
            stV2gPaymentServiceSelectionRes paymentServiceSelectionRes;
            if (read_V2gPaymentServiceSelectionReq(paymentServiceSelectionReq, header, receivedExiDocument))
            {
                dbg.printf(enDebugLevel_Info, "\nReceived request %s", paymentServiceSelectionReq.toString().c_str());

                // ************ Payment Service Selection Response **************

                paymentServiceSelectionRes.header.sessionId.set(sessionId);

                iStream.set(sendMessage, sizeof(sendMessage));
                len = write_V2gPaymentServiceSelectionRes(paymentServiceSelectionRes, iStream);
            }
            if (len)
            {
                dbg.printf(enDebugLevel_Info, "\nPrepared response %s", paymentServiceSelectionRes.toString().c_str());
                nextV2gState = enV2gState_awaitAuthorisation;
            }
        }
        // ************ Service Detail **************
        else if (header.messageType == enV2gMsg_ServiceDetail)
        {
            stV2gServiceDetailReq serviceDetailReq;
            stV2gServiceDetailRes serviceDetailRes;
            if (read_V2gServiceDetailReq(serviceDetailReq, header, receivedExiDocument))
            {
                dbg.printf(enDebugLevel_Info, "\nReceived request %s", serviceDetailReq.toString().c_str());

                // ************ Payment Service Detail Response **************

                serviceDetailRes.header.sessionId.set(sessionId);

                //serviceDetailRes.responseCode = enV2gResponse_FAILED_ServiceIDInvalid;
                serviceDetailRes.responseCode = enV2gResponse_OK;
                serviceDetailRes.serviceId = serviceDetailReq.serviceId;

                iStream.set(sendMessage, sizeof(sendMessage));
                len = write_V2gServiceDetailRes(serviceDetailRes, iStream);
            }
            if (len)
            {
                dbg.printf(enDebugLevel_Info, "\nPrepared response %s", serviceDetailRes.toString().c_str());
                nextV2gState = enV2gState_awaitPaymentServiceSelection;
            }
        }
        else
        {
            dbg.printf(enDebugLevel_Error, "Received message %s is not an expected %s or %s message!", header.toString().c_str(), enV2gMsg_toString(enV2gMsg_PaymentServiceSelection).c_str(), enV2gMsg_toString(enV2gMsg_ServiceDetail).c_str());
        }

    } break;
    case enV2gState_awaitAuthorisation:
    {
        // ************ Authorization **************
        stV2gHeader header;
        stV2gAuthorizationReq authorizationReq;
        stV2gAuthorizationRes authorizationRes;

        read_V2gHeader(header, iStream, receivedExiDocument);
        if (header.messageType == enV2gMsg_Authorization)
        {
            if (read_V2gAuthorizationReq(authorizationReq, header, receivedExiDocument))
            {
                dbg.printf(enDebugLevel_Info, "\nReceived request %s", authorizationReq.toString().c_str());

                // ************ Authorization Response **************

                authorizationRes.header.sessionId.set(sessionId);

                iStream.set(sendMessage, sizeof(sendMessage));
                len = write_V2gAuthorizationRes(authorizationRes, iStream);
            }
        }
        else
        {
            dbg.printf(enDebugLevel_Error, "Received message %s is not an expected %s message!", header.toString().c_str(), enV2gMsg_toString(enV2gMsg_Authorization).c_str());
        }

        if (len)
        {
            dbg.printf(enDebugLevel_Info, "\nPrepared response %s", authorizationRes.toString().c_str());
            nextV2gState = enV2gState_awaitChargeParameterDiscovery;
        }
    } break;
    case enV2gState_awaitChargeParameterDiscovery:
    {
        // ************ Charge Parameter Discovery **************
        stV2gHeader header;
        stV2gChargeParameterDiscoveryReq chargeParamsReq;
        stV2gChargeParameterDiscoveryRes chargeParamsRes;

        read_V2gHeader(header, iStream, receivedExiDocument);
        if (header.messageType == enV2gMsg_ChargeParameterDiscovery)
        {
            if (read_V2gChargeParameterDiscoveryReq(chargeParamsReq, header, receivedExiDocument))
            {
                dbg.printf(enDebugLevel_Info, "\nReceived request %s", chargeParamsReq.toString().c_str());

                // ************ Charge Parameter Discovery Response **************

                chargeParamsRes.header.sessionId.set(sessionId);
                if (energyTransferMode != iso1EnergyTransferModeType_AC_three_phase_core)
                {
                    chargeParamsRes.transferMode = energyTransferMode;
                }

                iStream.set(sendMessage, sizeof(sendMessage));
                len = write_V2gChargeParameterDiscoveryRes(chargeParamsRes, iStream);
            }
        }
        else
        {
            dbg.printf(enDebugLevel_Error, "Received message %s is not an expected %s message!", header.toString().c_str(), enV2gMsg_toString(enV2gMsg_ChargeParameterDiscovery).c_str());
        }

        if (len)
        {
            dbg.printf(enDebugLevel_Info, "\nPrepared response %s", chargeParamsRes.toString().c_str());
            nextV2gState = enV2gState_awaitPowerDelivery;
            if (energyTransferMode != iso1EnergyTransferModeType_AC_three_phase_core)
            {
                nextV2gState = enV2gState_awaitCableCheck;
            }
        }
    } break;
    case enV2gState_awaitPowerDelivery:
    {
        // ************ Power Delivery **************
        stV2gHeader header;
        stV2gPowerDeliveryReq powerDeliveryReq;
        stV2gPowerDeliveryRes powerDeliveryRes;

        read_V2gHeader(header, iStream, receivedExiDocument);
        if (header.messageType == enV2gMsg_PowerDelivery)
        {
            if (read_V2gPowerDeliveryReq(powerDeliveryReq, header, receivedExiDocument))
            {
                dbg.printf(enDebugLevel_Info, "\nReceived request %s", powerDeliveryReq.toString().c_str());

                // ************ Power Delivery Response **************

                powerDeliveryRes.header.sessionId.set(sessionId);
                if (energyTransferMode != iso1EnergyTransferModeType_AC_three_phase_core)
                {
                    powerDeliveryRes.transferMode = energyTransferMode;
                }

                iStream.set(sendMessage, sizeof(sendMessage));
                len = write_V2gPowerDeliveryRes(powerDeliveryRes, iStream);
                if (len)
                {
                    dbg.printf(enDebugLevel_Info, "\nPrepared response %s", powerDeliveryRes.toString().c_str());
                    nextV2gState = enV2gState_awaitChargingStatus;
                    if (energyTransferMode != iso1EnergyTransferModeType_AC_three_phase_core)
                    {
                        nextV2gState = enV2gState_awaitCurrentDemand;
                    }
                }
            }
        }
        else
        {
            dbg.printf(enDebugLevel_Error, "Received message %s is not an expected %s message!", header.toString().c_str(), enV2gMsg_toString(enV2gMsg_PowerDelivery).c_str());
        }
    } break;
    case enV2gState_awaitChargingStatus:
    {
        // ************ Charging Status **************
        stV2gHeader header;

        read_V2gHeader(header, iStream, receivedExiDocument);
        if (header.messageType == enV2gMsg_ChargingStatus)
        {
            stV2gChargingStatusReq chargingStatusReq;
            stV2gChargingStatusRes chargingStatusRes;
            if (read_V2gChargingStatusReq(chargingStatusReq, header, receivedExiDocument))
            {
                if (chargingStatusPrinted == 0 || dbg.getDebugLevel() <= enDebugLevel_Debug)
                    dbg.printf(enDebugLevel_Info, "\nReceived request %s", chargingStatusReq.toString().c_str());

                // ************ Charging Status Response **************

                chargingStatusRes.header.sessionId.set(sessionId);
                chargingStatusRes.seccId.set(seccId);

                iStream.set(sendMessage, sizeof(sendMessage));
                len = write_V2gChargingStatusRes(chargingStatusRes, iStream);
                if (len)
                {
                    if (chargingStatusPrinted == 0 || dbg.getDebugLevel() <= enDebugLevel_Debug)
                    {
                        dbg.printf(enDebugLevel_Info, "\nPrepared response %s", chargingStatusRes.toString().c_str());
                        chargingStatusPrinted = 1;
                    }
                    else if (chargingStatusPrinted == 1)
                    {
                        dbg.printf(enDebugLevel_Info, "\n...");
                        chargingStatusPrinted = 2;
                    }
                    nextV2gState = enV2gState_awaitChargingStatus;
                }
            }
        }
        else if (header.messageType == enV2gMsg_PowerDelivery)
        {
            stV2gPowerDeliveryReq powerDeliveryReq;
            stV2gPowerDeliveryRes powerDeliveryRes;

            if (read_V2gPowerDeliveryReq(powerDeliveryReq, header, receivedExiDocument))
            {
                dbg.printf(enDebugLevel_Info, "\nReceived request %s", powerDeliveryReq.toString().c_str());

                // ************ Charging Status Response **************

                powerDeliveryRes.header.sessionId.set(sessionId);

                iStream.set(sendMessage, sizeof(sendMessage));
                len = write_V2gPowerDeliveryRes(powerDeliveryRes, iStream);
                if (len)
                {
                    dbg.printf(enDebugLevel_Info, "\nPrepared response %s", powerDeliveryRes.toString().c_str());
                    nextV2gState = enV2gState_awaitChargingStatus;
                }
            }
            chargingStatusPrinted = 0;
        }
        else if (header.messageType == enV2gMsg_ChargeParameterDiscovery)
        {
            stV2gChargeParameterDiscoveryReq chargeParameterDiscoveryReq;
            stV2gChargeParameterDiscoveryRes chargeParameterDiscoveryRes;

            if (read_V2gChargeParameterDiscoveryReq(chargeParameterDiscoveryReq, header, receivedExiDocument))
            {
                dbg.printf(enDebugLevel_Info, "\nReceived request %s", chargeParameterDiscoveryReq.toString().c_str());

                // ************ ChargeParameterDiscovery Response **************

                chargeParameterDiscoveryRes.header.sessionId.set(sessionId);
                if (energyTransferMode != iso1EnergyTransferModeType_AC_three_phase_core)
                {
                    chargeParameterDiscoveryRes.transferMode = energyTransferMode;
                }

                iStream.set(sendMessage, sizeof(sendMessage));
                len = write_V2gChargeParameterDiscoveryRes(chargeParameterDiscoveryRes, iStream);
                if (len)
                {
                    dbg.printf(enDebugLevel_Info, "\nPrepared response %s", chargeParameterDiscoveryRes.toString().c_str());
                    nextV2gState = enV2gState_awaitPowerDelivery;
                    if (energyTransferMode != iso1EnergyTransferModeType_AC_three_phase_core)
                    {
                        nextV2gState = enV2gState_awaitCableCheck;
                    }
                }
            }
            chargingStatusPrinted = 0;
        }
//                    else if (header.messageType == enV2gMsg_WeldingDetection)
//                    {
//                        stV2gWeldingDetectionReq weldingDetectionReq;
//                        stV2gWeldingDetectionRes weldingDetectionRes;

//                        if (read_V2gWeldingDetectionReq(weldingDetectionReq, header))
//                        {
//                            dbg.printf(enDebugLevel_Info, "\nReceived request %s", weldingDetectionReq.toString().c_str());

//                            // ************ Welding Detection Response **************

//                            weldingDetectionRes.header.sessionId.set(sessionId);

//                            iStream.set(sendMessage, sizeof(sendMessage));
//                            len = write_V2gWeldingDetectionRes(weldingDetectionRes, iStream);
//                            if (len)
//                            {
//                                dbg.printf(enDebugLevel_Info, "\nPrepared response %s", weldingDetectionRes.toString().c_str());
//                                nextV2gState = enV2gState_awaitSessionStop;
//                            }
//                        }
//                        chargingStatusPrinted = 0;
//                    }
//                    else if (header.messageType == enV2gMsg_SessionStop)
//                    {
//                        stV2gSessionStopReq sessionStopReq;
//                        stV2gSessionStopRes sessionStopRes;

//                        if (read_V2gSessionStopReq(sessionStopReq, header))
//                        {
//                            dbg.printf(enDebugLevel_Info, "\nReceived request %s", sessionStopReq.toString().c_str());

//                            // ************ ChargeParameterDiscovery Response **************

//                            sessionStopRes.header.sessionId.set(sessionId);

//                            iStream.set(sendMessage, sizeof(sendMessage));
//                            len = write_V2gSessionStopRes(sessionStopRes, iStream);
//                            if (len)
//                            {
//                                dbg.printf(enDebugLevel_Info, "\nPrepared response %s", sessionStopRes.toString().c_str());
//                                nextV2gState = enV2gState_finished;
//                            }
//                            chargingStatusPrinted = 0;
//                        }
//                    }
        else
        {
            dbg.printf(enDebugLevel_Error, "Received message %s is not an expected %s, %s, %s or %s message!", header.toString().c_str(),
                       enV2gMsg_toString(enV2gMsg_ChargingStatus).c_str(),
                       enV2gMsg_toString(enV2gMsg_PowerDelivery).c_str(),
                       enV2gMsg_toString(enV2gMsg_ChargeParameterDiscovery).c_str());
            chargingStatusPrinted = 0;
        }
    } break;
    case enV2gState_awaitCableCheck:
    {
        // ************ Cable Check **************
        stV2gHeader header;
        stV2gCableCheckReq cableCheckReq;
        stV2gCableCheckRes cableCheckRes;

        read_V2gHeader(header, iStream, receivedExiDocument);
        if (header.messageType == enV2gMsg_CableCheck)
        {
            if (read_V2gCableCheckReq(cableCheckReq, header, receivedExiDocument))
            {
                dbg.printf(enDebugLevel_Info, "\nReceived request %s", cableCheckReq.toString().c_str());

                // ************ Cable Check Response **************

                cableCheckRes.header.sessionId.set(sessionId);

                iStream.set(sendMessage, sizeof(sendMessage));
                len = write_V2gCableCheckRes(cableCheckRes, iStream);
                if (len)
                {
                    dbg.printf(enDebugLevel_Info, "\nPrepared response %s", cableCheckRes.toString().c_str());
                    nextV2gState = enV2gState_awaitPreCharge;
                }
            }
        }
        else
        {
            dbg.printf(enDebugLevel_Error, "Received message %s is not an expected %s message!", header.toString().c_str(), enV2gMsg_toString(enV2gMsg_PowerDelivery).c_str());
        }
    } break;
    case enV2gState_awaitPreCharge:
    {
        // ************ Precharge **************
        stV2gHeader header;
        stV2gPreChargeReq preChargeReq;
        stV2gPreChargeRes preChargeRes;

        read_V2gHeader(header, iStream, receivedExiDocument);
        if (header.messageType == enV2gMsg_PreCharge)
        {
            if (read_V2gPreChargeReq(preChargeReq, header, receivedExiDocument))
            {
                if (chargingStatusPrinted == 0 || dbg.getDebugLevel() <= enDebugLevel_Debug)
                {
                    dbg.printf(enDebugLevel_Info, "\nReceived request %s", preChargeReq.toString().c_str());
                }
                if (chargingStatusPrinted == 1 && !(dbg.getDebugLevel() <= enDebugLevel_Debug))
                {
                    dbg.printf(enDebugLevel_Info, "\n...");
                }
                if (chargingStatusPrinted == 2 && pPowerSource)
                {
                    if (pControlPilot)
                        printf("%s, CP = %s\r", pPowerSource->toShortString().c_str(), enCpState_toString(pControlPilot->getState()).c_str());
                    else
                        printf("%s, CP = <not present>\r", pPowerSource->toShortString().c_str());
                    if (dbg.getDebugLevel() <= enDebugLevel_Debug)
                        printf("\n");
                }

                // ************ Precharge Response **************

                preChargeRes.header.sessionId.set(sessionId);

                if (pPowerSource)
                {
                    bool activatePower = true;
                    if (pControlPilot)
                        if (!(pControlPilot->getState()==enCpState::C_Ready || pControlPilot->getState()==enCpState::D_ReadyVentilation))
                            activatePower = false;
                    preChargeRes.evsePresentVoltage.fromFloat(pPowerSource->lastActual().voltage * 10);
                    pPowerSource->set(stPowerControl(activatePower, preChargeReq.evTargetVoltage.toFloat() / 10, 0));
                }
                else
                {
                    evseMemorizedVoltage = preChargeReq.evTargetVoltage.toFloat();
                    preChargeRes.evsePresentVoltage.fromFloat(evseMemorizedVoltage);
                }

                iStream.set(sendMessage, sizeof(sendMessage));
                len = write_V2gPreChargeRes(preChargeRes, iStream);
            }
            if (len)
            {
                if (chargingStatusPrinted == 0 || dbg.getDebugLevel() <= enDebugLevel_Debug)
                {
                    dbg.printf(enDebugLevel_Info, "\nPrepared response %s", preChargeRes.toString().c_str());
                    chargingStatusPrinted = 1;
                }
                else if (chargingStatusPrinted == 1)
                {
                    chargingStatusPrinted = 2;
                }
                nextV2gState = enV2gState_awaitPreCharge;
            }
        }
        else if (header.messageType == enV2gMsg_PowerDelivery)
        {
            stV2gPowerDeliveryReq powerDeliveryReq;
            stV2gPowerDeliveryRes powerDeliveryRes;

            if (read_V2gPowerDeliveryReq(powerDeliveryReq, header, receivedExiDocument))
            {
                chargingStatusPrinted = 0;
                if (!(dbg.getDebugLevel() <= enDebugLevel_Debug))
                    printf("\n");
                if (pPowerSource)
                {
                   pPowerSource->set(stPowerControl(false, 0, 0));
                }

                dbg.printf(enDebugLevel_Info, "\nReceived request %s", powerDeliveryReq.toString().c_str());

                // ************ Power Delivery Response **************

                powerDeliveryRes.header.sessionId.set(sessionId);
                if (energyTransferMode != iso1EnergyTransferModeType_AC_three_phase_core)
                {
                    powerDeliveryRes.transferMode = energyTransferMode;
                }

                iStream.set(sendMessage, sizeof(sendMessage));
                len = write_V2gPowerDeliveryRes(powerDeliveryRes, iStream);
                if (len)
                {
                    dbg.printf(enDebugLevel_Info, "\nPrepared response %s", powerDeliveryRes.toString().c_str());
                    chargingStatusPrinted = 0;
                    nextV2gState = enV2gState_awaitChargingStatus;
                    if (energyTransferMode != iso1EnergyTransferModeType_AC_three_phase_core)
                    {
                        nextV2gState = enV2gState_awaitCurrentDemand;
                    }
                }
            }
        }
        else
        {
            chargingStatusPrinted = 0;
            if (!(dbg.getDebugLevel() <= enDebugLevel_Debug))
                printf("\n");

            dbg.printf(enDebugLevel_Error, "Received message %s is not an expected %s message!", header.toString().c_str(), enV2gMsg_toString(enV2gMsg_PowerDelivery).c_str());
        }
    } break;
    case enV2gState_awaitCurrentDemand:
    {
        // ************ Current Demand **************
        stV2gHeader header;
        stV2gCurrentDemandReq currentDemandReq;
        stV2gCurrentDemandRes currentDemandRes;

        read_V2gHeader(header, iStream, receivedExiDocument);
        if (header.messageType == enV2gMsg_CurrentDemand)
        {
            if (read_V2gCurrentDemandReq(currentDemandReq, header, receivedExiDocument))
            {
                if (chargingStatusPrinted == 0 || dbg.getDebugLevel() <= enDebugLevel_Debug)
                {
                    dbg.printf(enDebugLevel_Info, "\nReceived request %s", currentDemandReq.toString().c_str());
                }
                if (chargingStatusPrinted == 1 && !(dbg.getDebugLevel() <= enDebugLevel_Debug))
                {
                    dbg.printf(enDebugLevel_Info, "\n...");
                }
                if (chargingStatusPrinted == 2 && pPowerSource)
                {
                    printf("%s\r", pPowerSource->toShortString().c_str());
                    if (dbg.getDebugLevel() <= enDebugLevel_Debug)
                        printf("\n");
                }

                float currentDemand = currentDemandReq.evTargetCurrent.toFloat();

                // ************ Current Demand Response **************

                currentDemandRes.header.sessionId.set(sessionId);
                currentDemandRes.evsePowerLimit.fromFloat(8000, iso1unitSymbolType_W);
                currentDemandRes.evseCurrentLimit.fromFloat(10, iso1unitSymbolType_A);
                currentDemandRes.evseVoltageLimit.fromFloat(600, iso1unitSymbolType_V);

                if (pPowerSource)
                {
                    bool activatePower = true;
                    if (pControlPilot)
                        if (!(pControlPilot->getState()==enCpState::C_Ready || pControlPilot->getState()==enCpState::D_ReadyVentilation))
                            activatePower = false;
                    currentDemandRes.evsePresentCurrent.fromFloat(pPowerSource->get().current, iso1unitSymbolType_A);
                    currentDemandRes.evsePresentVoltage.fromFloat(evseMemorizedVoltage, iso1unitSymbolType_V);
                    pPowerSource->set(stPowerControl(activatePower, currentDemandReq.evTargetVoltage.toFloat() / 10, currentDemandReq.evTargetCurrent.toFloat()));
                }
                else
                {
                    currentDemandRes.evsePresentCurrent.fromFloat(currentDemand);
                    currentDemandRes.evsePresentVoltage.fromFloat(evseMemorizedVoltage, iso1unitSymbolType_V);
                }

                iStream.set(sendMessage, sizeof(sendMessage));
                len = write_V2gCurrentDemandRes(currentDemandRes, iStream);
            }
            if (len)
            {
                if (chargingStatusPrinted == 0 || dbg.getDebugLevel() <= enDebugLevel_Debug)
                {
                    dbg.printf(enDebugLevel_Info, "\nPrepared response %s", currentDemandRes.toString().c_str());
                    chargingStatusPrinted = 1;
                }
                else if (chargingStatusPrinted == 1)
                {
                    chargingStatusPrinted = 2;
                }
                nextV2gState = enV2gState_awaitCurrentDemand;
            }
        }
        else if (header.messageType == enV2gMsg_PowerDelivery)
        {
            stV2gPowerDeliveryReq powerDeliveryReq;
            stV2gPowerDeliveryRes powerDeliveryRes;

            chargingStatusPrinted = 0;
            if (!(dbg.getDebugLevel() <= enDebugLevel_Debug))
                printf("\n");
            if (pPowerSource)
            {
                pPowerSource->set(stPowerControl(false, 0, 0));
            }

            if (read_V2gPowerDeliveryReq(powerDeliveryReq, header, receivedExiDocument))
            {

                dbg.printf(enDebugLevel_Info, "\nReceived request %s", powerDeliveryReq.toString().c_str());

                // ************ Power Delivery Response **************

                powerDeliveryRes.header.sessionId.set(sessionId);
                if (energyTransferMode != iso1EnergyTransferModeType_AC_three_phase_core)
                {
                    powerDeliveryRes.transferMode = energyTransferMode;
                }

                iStream.set(sendMessage, sizeof(sendMessage));
                len = write_V2gPowerDeliveryRes(powerDeliveryRes, iStream);
                if (len)
                {
                    dbg.printf(enDebugLevel_Info, "\nPrepared response %s", powerDeliveryRes.toString().c_str());
                    nextV2gState = enV2gState_awaitChargingStatus;
                    if (energyTransferMode != iso1EnergyTransferModeType_AC_three_phase_core)
                    {
                        nextV2gState = enV2gState_awaitWeldingDetection;
                    }
                }
                chargingStatusPrinted = 0;
            }
        }
        else if (header.messageType == enV2gMsg_ChargeParameterDiscovery)
        {
            stV2gChargeParameterDiscoveryReq chargeParameterDiscoveryReq;
            stV2gChargeParameterDiscoveryRes chargeParameterDiscoveryRes;

            chargingStatusPrinted = 0;
            if (!(dbg.getDebugLevel() <= enDebugLevel_Debug))
                printf("\n");
            if (pPowerSource)
            {
                pPowerSource->set(stPowerControl(false, 0, 0));
            }

            if (read_V2gChargeParameterDiscoveryReq(chargeParameterDiscoveryReq, header, receivedExiDocument))
            {
                chargingStatusPrinted = 0;
                if (!(dbg.getDebugLevel() <= enDebugLevel_Debug))
                    printf("\n");

                dbg.printf(enDebugLevel_Info, "\nReceived request %s", chargeParameterDiscoveryReq.toString().c_str());

                // ************ ChargeParameterDiscovery Response **************

                chargeParameterDiscoveryRes.header.sessionId.set(sessionId);
                if (energyTransferMode != iso1EnergyTransferModeType_AC_three_phase_core)
                {
                    chargeParameterDiscoveryRes.transferMode = energyTransferMode;
                }

                iStream.set(sendMessage, sizeof(sendMessage));
                len = write_V2gChargeParameterDiscoveryRes(chargeParameterDiscoveryRes, iStream);
                if (len)
                {
                    dbg.printf(enDebugLevel_Info, "\nPrepared response %s", chargeParameterDiscoveryRes.toString().c_str());
                    nextV2gState = enV2gState_awaitCableCheck;
                }
                chargingStatusPrinted = 0;
            }
        }
        else if (header.messageType == enV2gMsg_WeldingDetection)
        {
            stV2gWeldingDetectionReq weldingDetectionReq;
            stV2gWeldingDetectionRes weldingDetectionRes;

            chargingStatusPrinted = 0;
            if (!(dbg.getDebugLevel() <= enDebugLevel_Debug))
                printf("\n");
            if (pPowerSource)
            {
                pPowerSource->set(stPowerControl(false, 0, 0));
            }

            if (read_V2gWeldingDetectionReq(weldingDetectionReq, header, receivedExiDocument))
            {
                chargingStatusPrinted = 0;
                if (!(dbg.getDebugLevel() <= enDebugLevel_Debug))
                    printf("\n");

                dbg.printf(enDebugLevel_Info, "\nReceived request %s", weldingDetectionReq.toString().c_str());

                // ************ Welding Detection Response **************

                weldingDetectionRes.header.sessionId.set(sessionId);

                iStream.set(sendMessage, sizeof(sendMessage));
                len = write_V2gWeldingDetectionRes(weldingDetectionRes, iStream);
                if (len)
                {
                    dbg.printf(enDebugLevel_Info, "\nPrepared response %s", weldingDetectionRes.toString().c_str());
                    nextV2gState = enV2gState_awaitWeldingDetection;
                }
            }
            chargingStatusPrinted = 0;
        }
        else
        {
            chargingStatusPrinted = 0;
            if (!(dbg.getDebugLevel() <= enDebugLevel_Debug))
                printf("\n");
            if (pPowerSource)
            {
                pPowerSource->set(stPowerControl(false, 0, 0));
            }

            dbg.printf(enDebugLevel_Error, "Received message %s is not an expected %s, %s, %s, %s or %s message!", header.toString().c_str(),
                       enV2gMsg_toString(enV2gMsg_CurrentDemand).c_str(),
                       enV2gMsg_toString(enV2gMsg_PowerDelivery).c_str(),
                       enV2gMsg_toString(enV2gMsg_ChargeParameterDiscovery).c_str(),
                       enV2gMsg_toString(enV2gMsg_WeldingDetection).c_str());
            chargingStatusPrinted = 0;
        }
    } break;
    case enV2gState_awaitWeldingDetection:
    {
        // ************ Welding Detection **************
        stV2gHeader header;

        read_V2gHeader(header, iStream, receivedExiDocument);
        if (header.messageType == enV2gMsg_WeldingDetection)
        {
            stV2gWeldingDetectionReq weldingDetectionReq;
            stV2gWeldingDetectionRes weldingDetectionRes;

            if (read_V2gWeldingDetectionReq(weldingDetectionReq, header, receivedExiDocument))
            {
                dbg.printf(enDebugLevel_Info, "\nReceived request %s", weldingDetectionReq.toString().c_str());

                // ************ Welding Detection Response **************

                weldingDetectionRes.header.sessionId.set(sessionId);

                iStream.set(sendMessage, sizeof(sendMessage));
                len = write_V2gWeldingDetectionRes(weldingDetectionRes, iStream);
                if (len)
                {
                    dbg.printf(enDebugLevel_Info, "\nPrepared response %s", weldingDetectionRes.toString().c_str());
                    nextV2gState = enV2gState_awaitWeldingDetection;
                }
            }
        }
        else if (header.messageType == enV2gMsg_SessionStop)
        {
            stV2gSessionStopReq sessionStopReq;
            stV2gSessionStopRes sessionStopRes;
            if (read_V2gSessionStopReq(sessionStopReq, header, receivedExiDocument))
            {
                dbg.printf(enDebugLevel_Info, "\nReceived request %s", sessionStopReq.toString().c_str());

                // ************ Session Stop Response **************

                sessionStopRes.header.sessionId.set(sessionId);

                iStream.set(sendMessage, sizeof(sendMessage));
                len = write_V2gSessionStopRes(sessionStopRes, iStream);
                if (len)
                {
                    dbg.printf(enDebugLevel_Info, "\nPrepared response %s", sessionStopRes.toString().c_str());
                    nextV2gState = enV2gState_finished;
                }
            }
        }
        else
        {
            dbg.printf(enDebugLevel_Error, "Received message %s is not an expected %s or %s message!", header.toString().c_str(),
                       enV2gMsg_toString(enV2gMsg_WeldingDetection).c_str(),
                       enV2gMsg_toString(enV2gMsg_SessionStop).c_str());
        }
    } break;
    case enV2gState_finished: break;
    default:
        dbg.printf(LibCpp::enDebugLevel_Error, "Unexpected v2gState!");
    }

    if (!len)
        dbg.printf(LibCpp::enDebugLevel_Debug, "Received message is not expected or invalid!");

    responseTimer.start();
    return len;
}

bool cIsoProcessSECC::operate(bool run)
{
    cDebug dbg("operate", &this->dbg);
    bool result = false;

    if (v2gState == enV2gState_awaitPreCharge || nextV2gState == enV2gState_awaitPreCharge ||
        v2gState == enV2gState_awaitPreCharge || nextV2gState == enV2gState_awaitPreCharge ||
        v2gState == enV2gState_awaitPreCharge || nextV2gState == enV2gState_awaitPreCharge )
    {
    }
    else
    {
        //        if (pPowerSource)
        //            pPowerSource->set(stPowerControl(false, 0, 0));
    }
//    switch (v2gState)
//    {
//    case enV2gState_awaitPreCharge:
//    case enV2gState_awaitPowerDelivery:
//    case enV2gState_awaitCurrentDemand:
//        break;
//    default:;
//    }

    switch (v2gState)
    {
    case enV2gState_idle:
        if(run)
            nextV2gState = enV2gState_awaitSdp;
        break;
    case enV2gState_finished:
        nextV2gState = enV2gState_idle;
        result = true;
        break;
//    case enV2gState_failure:
//        nextV2gState = enV2gState_idle;
//        result = true;
//        break;
//    case enV2gState_awaitSdp:
//    case enV2gState_awaitSupportedAppProtocol:
//        break;
//    case enV2gState_awaitCurrentDemand:
//        if (responseTimer.passed(V2G_POWER_TIMEOUT))
//            nextV2gState = enV2gState_failure;
//        break;
//    default:
//        if (responseTimer.passed(V2G_TIMEOUT))
//            nextV2gState = enV2gState_failure;
    default:;
    }

    if (nextV2gState != v2gState)
        dbg.printf(LibCpp::enDebugLevel_Info, "\nEntered state %s.", enV2gState_toString(nextV2gState).c_str());
    v2gState = nextV2gState;
    return result;
}
