#include "cExiCodec.h"

using namespace LibCpp;
using namespace Iso15118;

cExiCodec::cExiCodec() :
    dbg("cExiCodec")
{
    printUnrepetitive();
}

void cExiCodec::printUnrepetitive(LibCpp::cDebug* pDbg, int* pPrintState, std::string output)
{
    if (!pDbg) return;

    int mem = 0;
    if (pPrintState) mem = *pPrintState;
    printedAuthorization = 0;
    printedCableCheck = 0;
    printedPrecharge = 0;
    printedWeldingDetection = 0;
    printedCurrentDemand = 0;
    printedChargingStatus = 0;
    if (pDbg->getDebugLevel() == LibCpp::enDebugLevel_Info)
    {
        if (pPrintState)
        {
            *pPrintState = mem;
            if (*pPrintState == 0)
            {
                pDbg->printf(enDebugLevel_Info, output.c_str());
                if (pDbg->getDebugLevel() > enDebugLevel::enDebugLevel_Debug)
                    (*pPrintState)++;
            }
            else if (*pPrintState == 1)
            {
                pDbg->printf(enDebugLevel_Info, "\n...");
                (*pPrintState)++;
            }
        }
    }
    else
    {
        pDbg->printf(enDebugLevel_Info, output.c_str());
    }
}

int cExiCodec::encode(char* buffer, unsigned int bufferSize, stV2gMessage& message)
{
    cDebug dbg("encode", &this->dbg);
    stStreamControl oStream = stStreamControl(buffer, bufferSize);
    int len = 0;

    if (message.mode == enV2gMode::Req)
    {
        switch (message.type)
        {
        case enV2gMsg::Sdp:
            len = write_V2gSdpReq(message.content.req.sdp, oStream);
            if (len) dbg.printf(enDebugLevel_Info, "\nSending %s", message.content.req.sdp.toString().c_str());
            break;
        case enV2gMsg::V2s:
            len = write_V2s(oStream);
            if (len) dbg.printf(enDebugLevel_Info, "\nSending %s", "V2S message");
            break;
        case enV2gMsg::SupportedAppProtocol:
            len = write_V2gSupportedAppProtocolReq(message.content.req.supportedAppProtocol, oStream);
            if (len) dbg.printf(enDebugLevel_Info, "\nSending %s", message.content.req.supportedAppProtocol.toString().c_str());
            break;
        case enV2gMsg::SessionSetup:
            len = write_V2gSessionSetupReq(message.content.req.sessionSetup, oStream);
            if (len) dbg.printf(enDebugLevel_Info, "\nSending %s", message.content.req.sessionSetup.toString().c_str());
            break;
        case enV2gMsg::ServiceDiscovery:
            len = write_V2gServiceDiscoveryReq(message.content.req.serviceDiscovery, oStream);
            if (len) dbg.printf(enDebugLevel_Info, "\nSending %s", message.content.req.serviceDiscovery.toString().c_str());
            break;
        case enV2gMsg::PaymentServiceSelection:
            len = write_V2gPaymentServiceSelectionReq(message.content.req.paymentServiceSelection, oStream);
            if (len) dbg.printf(enDebugLevel_Info, "\nSending %s", message.content.req.paymentServiceSelection.toString().c_str());
            break;
        case enV2gMsg::Authorization:
            len = write_V2gAuthorizationReq(message.content.req.authorization, oStream);
            if (len) printUnrepetitive(&dbg, &printedAuthorization, "\nSending " + message.content.req.authorization.toString());
            break;
        case enV2gMsg::ChargeParameterDiscovery:
            len = write_V2gChargeParameterDiscoveryReq(message.content.req.chargeParameterDiscovery, oStream);
            if (len) dbg.printf(enDebugLevel_Info, "\nSending %s", message.content.req.chargeParameterDiscovery.toString().c_str());
            break;
        case enV2gMsg::PowerDelivery:
            len = write_V2gPowerDeliveryReq(message.content.req.powerDelivery, oStream);
            if (len) dbg.printf(enDebugLevel_Info, "\nSending %s", message.content.req.powerDelivery.toString().c_str());
            break;
        case enV2gMsg::ChargingStatus:
            len = write_V2gChargingStatusReq(message.content.req.chargingStatus, oStream);
            if (len) printUnrepetitive(&dbg, &printedChargingStatus, "\nSending " + message.content.req.chargingStatus.toString());
            break;
        case enV2gMsg::SessionStop:
            len = write_V2gSessionStopReq(message.content.req.sessionStop, oStream);
            if (len) dbg.printf(enDebugLevel_Info, "\nSending %s", message.content.req.sessionStop.toString().c_str());
            break;
        case enV2gMsg::CableCheck:
            len = write_V2gCableCheckReq(message.content.req.cableCheck, oStream);
            if (len) printUnrepetitive(&dbg, &printedCableCheck, "\nSending " + message.content.req.cableCheck.toString());
            break;
        case enV2gMsg::PreCharge:
            len = write_V2gPreChargeReq(message.content.req.preCharge, oStream);
            if (len) printUnrepetitive(&dbg, &printedPrecharge, "\nSending " + message.content.req.preCharge.toString());
            break;
        case enV2gMsg::CurrentDemand:
            len = write_V2gCurrentDemandReq(message.content.req.currentDemand, oStream);
            if (len) printUnrepetitive(&dbg, &printedCurrentDemand, "\nSending " + message.content.req.currentDemand.toString());
            break;
        case enV2gMsg::WeldingDetection:
            len = write_V2gWeldingDetectionReq(message.content.req.weldingDetection, oStream);
            if (len) printUnrepetitive(&dbg, &printedWeldingDetection, "\nSending " + message.content.req.weldingDetection.toString());
            break;
        case enV2gMsg::ServiceDetail:
            len = write_V2gServiceDetailReq(message.content.req.serviceDetail, oStream);
            if (len) dbg.printf(enDebugLevel_Info, "\nSending %s", message.content.req.serviceDetail.toString().c_str());
            break;
        case enV2gMsg::MeteringReceipt:
            break;
        case enV2gMsg::PaymentDetails:
            break;
        default:;
        }
    }
    else if (message.mode == enV2gMode::Res)
    {
        switch (message.type)
        {
        case enV2gMsg::Sdp:
            len = write_V2gSdpRes(message.content.res.sdp, oStream);
            if (len) dbg.printf(enDebugLevel_Info, "\nSending %s", message.content.res.sdp.toString().c_str());
            break;
        case enV2gMsg::V2s:
            len = write_V2s(oStream);
            if (len) dbg.printf(enDebugLevel_Info, "\nSending %s", "V2S message");
            break;
        case enV2gMsg::SupportedAppProtocol:
            len = write_V2gSupportedAppProtocolRes(message.content.res.supportedAppProtocol, oStream);
            if (len) dbg.printf(enDebugLevel_Info, "\nSending %s", message.content.res.supportedAppProtocol.toString().c_str());
            break;
        case enV2gMsg::SessionSetup:
            len = write_V2gSessionSetupRes(message.content.res.sessionSetup, oStream);
            if (len) dbg.printf(enDebugLevel_Info, "\nSending %s", message.content.res.sessionSetup.toString().c_str());
            break;
        case enV2gMsg::ServiceDiscovery:
            len = write_V2gServiceDiscoveryRes(message.content.res.serviceDiscovery, oStream);
            if (len) dbg.printf(enDebugLevel_Info, "\nSending %s", message.content.res.serviceDiscovery.toString().c_str());
            break;
        case enV2gMsg::PaymentServiceSelection:
            len = write_V2gPaymentServiceSelectionRes(message.content.res.paymentServiceSelection, oStream);
            if (len) dbg.printf(enDebugLevel_Info, "\nSending %s", message.content.res.paymentServiceSelection.toString().c_str());
            break;
        case enV2gMsg::Authorization:
            len = write_V2gAuthorizationRes(message.content.res.authorization, oStream);
            if (len) printUnrepetitive(&dbg, &printedAuthorization, "\nSending " + message.content.res.authorization.toString());
            break;
        case enV2gMsg::ChargeParameterDiscovery:
            len = write_V2gChargeParameterDiscoveryRes(message.content.res.chargeParameterDiscovery, oStream);
            if (len) dbg.printf(enDebugLevel_Info, "\nSending %s", message.content.res.chargeParameterDiscovery.toString().c_str());
            break;
        case enV2gMsg::PowerDelivery:
            len = write_V2gPowerDeliveryRes(message.content.res.powerDelivery, oStream);
            if (len) dbg.printf(enDebugLevel_Info, "\nSending %s", message.content.res.powerDelivery.toString().c_str());
            break;
        case enV2gMsg::ChargingStatus:
            len = write_V2gChargingStatusRes(message.content.res.chargingStatus, oStream);
            if (len) printUnrepetitive(&dbg, &printedChargingStatus, "\nSending " + message.content.res.chargingStatus.toString());
            break;
        case enV2gMsg::SessionStop:
            len = write_V2gSessionStopRes(message.content.res.sessionStop, oStream);
            if (len) dbg.printf(enDebugLevel_Info, "\nSending %s", message.content.res.sessionStop.toString().c_str());
            break;
        case enV2gMsg::CableCheck:
            len = write_V2gCableCheckRes(message.content.res.cableCheck, oStream);
            if (len) printUnrepetitive(&dbg, &printedCableCheck, "\nSending " + message.content.res.cableCheck.toString());
            break;
        case enV2gMsg::PreCharge:
            len = write_V2gPreChargeRes(message.content.res.preCharge, oStream);
            if (len) printUnrepetitive(&dbg, &printedPrecharge, "\nSending " + message.content.res.preCharge.toString());
            break;
        case enV2gMsg::CurrentDemand:
            len = write_V2gCurrentDemandRes(message.content.res.currentDemand, oStream);
            if (len) printUnrepetitive(&dbg, &printedCurrentDemand, "\nSending " + message.content.res.currentDemand.toString());
            break;
        case enV2gMsg::WeldingDetection:
            len = write_V2gWeldingDetectionRes(message.content.res.weldingDetection, oStream);
            if (len) printUnrepetitive(&dbg, &printedWeldingDetection, "\nSending " + message.content.res.weldingDetection.toString());
            break;
        case enV2gMsg::ServiceDetail:
            len = write_V2gServiceDetailRes(message.content.res.serviceDetail, oStream);
            if (len) dbg.printf(enDebugLevel_Info, "\nSending %s", message.content.res.serviceDetail.toString().c_str());
            break;
        case enV2gMsg::MeteringReceipt:
            break;
        case enV2gMsg::PaymentDetails:
            break;
        default:;
        }
    }

    if (!len)
        dbg.printf(enDebugLevel_Error, "Failed to encode message %s", enV2gMsg_toString(message.type).c_str());
    return len;
}

void cExiCodec::decode(stV2gMessage& message, char* pBuffer, int length, bool expectSuppAppProt)
{
    cDebug dbg("decode", &this->dbg);

    message.type = enV2gMsg::Empty;
    stStreamControl iStream = stStreamControl(pBuffer, length);

    bool ok = false;

    stV2gTpHeader v2gTpHeader;
    ok = read_V2gTpHeader(v2gTpHeader, iStream);
    if (ok)
    {
        // ************ SDP Request **************
        if (v2gTpHeader.payloadType == V2GTP_TYPE_SDP_REQ)
        {
            message.set(enV2gMsg::Sdp, enV2gMode::Req);
            ok = read_V2gSdpReq(message.content.req.sdp, v2gTpHeader, iStream);
            if (!ok)
                dbg.printf(enDebugLevel_Error, "Unable to decode SDP request message");
            else
                dbg.printf(enDebugLevel_Info, "\nReceived %s", message.content.req.sdp.toString().c_str());
        }

        // ************ SDP Response **************
        else if (v2gTpHeader.payloadType == V2GTP_TYPE_SDP_RES)
        {
            message.set(enV2gMsg::Sdp, enV2gMode::Res);
            ok = read_V2gSdpRes(message.content.res.sdp, v2gTpHeader, iStream);
            if (!ok)
                dbg.printf(enDebugLevel_Error, "Unable to decode SDP response message");
            else
                dbg.printf(enDebugLevel_Info, "\nReceived %s", message.content.res.sdp.toString().c_str());
        }

        // ************ V2S Request / Response **************
        else if (v2gTpHeader.payloadType == V2GTP_TYPE_V2S)
        {
            message.set(enV2gMsg::V2s, enV2gMode::Res);
            dbg.printf(enDebugLevel_Info, "\nReceived %s", "V2S Message");
        }

        // ------------- EXI Message ---------------
        else if (v2gTpHeader.payloadType == V2GTP_TYPE_EXI)
        {   // Check for codec dependent messages first (as they are usual).
            stStreamControl appStream = iStream;
            stV2gHeader header;
            header.tpHeader = v2gTpHeader;
            ok = read_V2gHeader(header, iStream, expectSuppAppProt);
            if (ok)
            {
                switch (header.messageType)
                {
                // ************ Session Setup **************
                case enV2gMsg::SessionSetup:
                    if (header.messageMode == enV2gMode::Req)
                    {
                        message.set(enV2gMsg::SessionSetup, enV2gMode::Req);
                        ok = read_V2gSessionSetupReq(message.content.req.sessionSetup, header);
                        if (ok) dbg.printf(enDebugLevel_Info, "\nReceived %s", message.content.req.sessionSetup.toString().c_str());
                    }
                    else
                    {
                        message.set(enV2gMsg::SessionSetup, enV2gMode::Res);
                        ok = read_V2gSessionSetupRes(message.content.res.sessionSetup, header);
                        if (ok) dbg.printf(enDebugLevel_Info, "\nReceived %s", message.content.res.sessionSetup.toString().c_str());
                    }
                    break;
                // ************ Service Discovery **************
                case  enV2gMsg::ServiceDiscovery:
                    if (header.messageMode == enV2gMode::Req)
                    {
                        message.set(enV2gMsg::ServiceDiscovery, enV2gMode::Req);
                        ok = read_V2gServiceDiscoveryReq(message.content.req.serviceDiscovery, header);
                        if (ok) dbg.printf(enDebugLevel_Info, "\nReceived %s", message.content.req.serviceDiscovery.toString().c_str());
                    }
                    else
                    {
                        message.set(enV2gMsg::ServiceDiscovery, enV2gMode::Res);
                        ok = read_V2gServiceDiscoveryRes(message.content.res.serviceDiscovery, header);
                        if (ok) dbg.printf(enDebugLevel_Info, "\nReceived %s", message.content.res.serviceDiscovery.toString().c_str());
                    }
                    break;
                // ************ Payment Service Selection **************
                case enV2gMsg::PaymentServiceSelection:
                    if (header.messageMode == enV2gMode::Req)
                    {
                        message.set(enV2gMsg::PaymentServiceSelection, enV2gMode::Req);
                        ok = read_V2gPaymentServiceSelectionReq(message.content.req.paymentServiceSelection, header);
                        if (ok) dbg.printf(enDebugLevel_Info, "\nReceived %s", message.content.req.paymentServiceSelection.toString().c_str());
                    }
                    else
                    {
                        message.set(enV2gMsg::PaymentServiceSelection, enV2gMode::Res);
                        ok = read_V2gPaymentServiceSelectionRes(message.content.res.paymentServiceSelection, header);
                        if (ok) dbg.printf(enDebugLevel_Info, "\nReceived %s", message.content.res.paymentServiceSelection.toString().c_str());
                    }
                    break;
                // ************ Service Detail **************
                case enV2gMsg::ServiceDetail:
                    if (header.messageMode == enV2gMode::Req)
                    {
                        message.set(enV2gMsg::ServiceDetail, enV2gMode::Req);
                        ok = read_V2gServiceDetailReq(message.content.req.serviceDetail, header);
                        if (ok) dbg.printf(enDebugLevel_Info, "\nReceived %s", message.content.req.serviceDetail.toString().c_str());
                    }
                    else
                    {
                        message.set(enV2gMsg::ServiceDetail, enV2gMode::Res);
                        ok = read_V2gServiceDetailRes(message.content.res.serviceDetail, header);
                        if (ok) dbg.printf(enDebugLevel_Info, "\nReceived %s", message.content.res.serviceDetail.toString().c_str());
                    }
                    break;
                // ************ Authorization **************
                case enV2gMsg::Authorization:
                    if (header.messageMode == enV2gMode::Req)
                    {
                        message.set(enV2gMsg::Authorization, enV2gMode::Req);
                        ok = read_V2gAuthorizationReq(message.content.req.authorization, header);
                        if (ok && printedAuthorization<1) dbg.printf(enDebugLevel_Info, "\nReceived %s", message.content.req.authorization.toString().c_str());
                    }
                    else
                    {
                        message.set(enV2gMsg::Authorization, enV2gMode::Res);
                        ok = read_V2gAuthorizationRes(message.content.res.authorization, header);
                        if (ok && printedAuthorization<=1) dbg.printf(enDebugLevel_Info, "\nReceived %s", message.content.res.authorization.toString().c_str());
                    }
                    break;
                // ************ Charge Parameter Discovery **************
                case enV2gMsg::ChargeParameterDiscovery:
                    if (header.messageMode == enV2gMode::Req)
                    {
                        message.set(enV2gMsg::ChargeParameterDiscovery, enV2gMode::Req);
                        ok = read_V2gChargeParameterDiscoveryReq(message.content.req.chargeParameterDiscovery, header);
                        if (ok) dbg.printf(enDebugLevel_Info, "\nReceived %s", message.content.req.chargeParameterDiscovery.toString().c_str());
                    }
                    else
                    {
                        message.set(enV2gMsg::ChargeParameterDiscovery, enV2gMode::Res);
                        ok = read_V2gChargeParameterDiscoveryRes(message.content.res.chargeParameterDiscovery, header);
                        if (ok) dbg.printf(enDebugLevel_Info, "\nReceived %s", message.content.res.chargeParameterDiscovery.toString().c_str());
                    }
                    break;
                // ************ Power Delivery **************
                case enV2gMsg::PowerDelivery:
                    if (header.messageMode == enV2gMode::Req)
                    {
                        message.set(enV2gMsg::PowerDelivery, enV2gMode::Req);
                        ok = read_V2gPowerDeliveryReq(message.content.req.powerDelivery, header);
                        if (ok) dbg.printf(enDebugLevel_Info, "\nReceived %s", message.content.req.powerDelivery.toString().c_str());
                    }
                    else
                    {
                        message.set(enV2gMsg::PowerDelivery, enV2gMode::Res);
                        ok = read_V2gPowerDeliveryRes(message.content.res.powerDelivery, header);
                        if (ok) dbg.printf(enDebugLevel_Info, "\nReceived %s", message.content.res.powerDelivery.toString().c_str());
                    }
                    break;
                // ************ Charging Status **************
                case enV2gMsg::ChargingStatus:
                    if (header.messageMode == enV2gMode::Req)
                    {
                        message.set(enV2gMsg::ChargingStatus, enV2gMode::Req);
                        ok = read_V2gChargingStatusReq(message.content.req.chargingStatus, header);
                        if (ok && printedChargingStatus<1) dbg.printf(enDebugLevel_Info, "\nReceived %s", message.content.req.chargingStatus.toString().c_str());
                    }
                    else
                    {
                        message.set(enV2gMsg::ChargingStatus, enV2gMode::Res);
                        ok = read_V2gChargingStatusRes(message.content.res.chargingStatus, header);
                        if (ok && printedChargingStatus<=1) dbg.printf(enDebugLevel_Info, "\nReceived %s", message.content.res.chargingStatus.toString().c_str());
                    }
                    break;
                // ************ Session Stop **************
                case enV2gMsg::SessionStop:
                    if (header.messageMode == enV2gMode::Req)
                    {
                        message.set(enV2gMsg::SessionStop, enV2gMode::Req);
                        ok = read_V2gSessionStopReq(message.content.req.sessionStop, header);
                        if (ok) dbg.printf(enDebugLevel_Info, "\nReceived %s", message.content.req.sessionStop.toString().c_str());
                    }
                    else
                    {
                        message.set(enV2gMsg::SessionStop, enV2gMode::Res);
                        ok = read_V2gSessionStopRes(message.content.res.sessionStop, header);
                        if (ok) dbg.printf(enDebugLevel_Info, "\nReceived %s", message.content.res.sessionStop.toString().c_str());
                    }
                    break;
                // ************ Cable Check **************
                case enV2gMsg::CableCheck:
                    if (header.messageMode == enV2gMode::Req)
                    {
                        message.set(enV2gMsg::CableCheck, enV2gMode::Req);
                        ok = read_V2gCableCheckReq(message.content.req.cableCheck, header);
                        if (ok && printedCableCheck<1) dbg.printf(enDebugLevel_Info, "\nReceived %s", message.content.req.cableCheck.toString().c_str());
                    }
                    else
                    {
                        message.set(enV2gMsg::CableCheck, enV2gMode::Res);
                        ok = read_V2gCableCheckRes(message.content.res.cableCheck, header);
                        if (ok && printedCableCheck<=1) dbg.printf(enDebugLevel_Info, "\nReceived %s", message.content.res.cableCheck.toString().c_str());
                    }
                    break;
                // ************ Precharge **************
                case enV2gMsg::PreCharge:
                    if (header.messageMode == enV2gMode::Req)
                    {
                        message.set(enV2gMsg::PreCharge, enV2gMode::Req);
                        ok = read_V2gPreChargeReq(message.content.req.preCharge, header);
                        if (ok && printedPrecharge<1) dbg.printf(enDebugLevel_Info, "\nReceived %s", message.content.req.preCharge.toString().c_str());
                    }
                    else
                    {
                        message.set(enV2gMsg::PreCharge, enV2gMode::Res);
                        ok = read_V2gPreChargeRes(message.content.res.preCharge, header);
                        if (ok && printedPrecharge<=1) dbg.printf(enDebugLevel_Info, "\nReceived %s", message.content.res.preCharge.toString().c_str());
                    }
                    break;
                // ************ Current Demand **************
                case enV2gMsg::CurrentDemand:
                    if (header.messageMode == enV2gMode::Req)
                    {
                        message.set(enV2gMsg::CurrentDemand, enV2gMode::Req);
                        ok = read_V2gCurrentDemandReq(message.content.req.currentDemand, header);
                        if (ok && printedCurrentDemand<1) dbg.printf(enDebugLevel_Info, "\nReceived %s", message.content.req.currentDemand.toString().c_str());
                    }
                    else
                    {
                        message.set(enV2gMsg::CurrentDemand, enV2gMode::Res);
                        ok = read_V2gCurrentDemandRes(message.content.res.currentDemand, header);
                        if (ok && printedCurrentDemand<=1) dbg.printf(enDebugLevel_Info, "\nReceived %s", message.content.res.currentDemand.toString().c_str());
                    }
                    break;
                // ************ Welding Detection **************
                case enV2gMsg::WeldingDetection:
                    if (header.messageMode == enV2gMode::Req)
                    {
                        message.set(enV2gMsg::WeldingDetection, enV2gMode::Req);
                        ok = read_V2gWeldingDetectionReq(message.content.req.weldingDetection, header);
                        if (ok && printedWeldingDetection<1) dbg.printf(enDebugLevel_Info, "\nReceived %s", message.content.req.weldingDetection.toString().c_str());
                    }
                    else
                    {
                        message.set(enV2gMsg::WeldingDetection, enV2gMode::Res);
                        ok = read_V2gWeldingDetectionRes(message.content.res.weldingDetection, header);
                        if (ok && printedWeldingDetection<=1) dbg.printf(enDebugLevel_Info, "\nReceived %s", message.content.res.weldingDetection.toString().c_str());
                    }
                    break;
                default:
                    message.set(enV2gMsg::Empty);
                    dbg.printf(enDebugLevel_Error, "Received unimplemented message %s!", header.toString().c_str());
                }

                if (!ok) dbg.printf(enDebugLevel_Error, "\nDecoding of %s is invalid or failed!", header.toString().c_str());
            }
            else
            {   // Check for non codec dependent Supported App messages
                // ************ Supported App Protocol Request **************
                iStream = appStream;
                message.set(enV2gMsg::SupportedAppProtocol, enV2gMode::Req);
                ok = read_V2gSupportedAppProtocolReq(message.content.req.supportedAppProtocol, iStream);
                if (ok)
                {
                    dbg.printf(enDebugLevel_Info, "\nReceived request %s", message.content.req.supportedAppProtocol.toString().c_str());
                }
                else
                {
                    // ************ Supported App Protocol Response **************
                    iStream = appStream;
                    message.set(enV2gMsg::SupportedAppProtocol, enV2gMode::Res);
                    ok = read_V2gSupportedAppProtocolRes(message.content.res.supportedAppProtocol, iStream);
                    if (ok)
                    {
                        dbg.printf(enDebugLevel_Info, "\nReceived response %s", message.content.res.supportedAppProtocol.toString().c_str());
                    }
                }

                if (!ok)
                    dbg.printf(enDebugLevel_Error, "Unable to decode EXI message!");
            }
        }
        else
            dbg.printf(enDebugLevel_Error, "Received V2G-TP message %s is not of SDP or EXI type!", v2gTpHeader.toString().c_str());
    }
    else
    {
        dbg.printf(enDebugLevel_Error, "Received a non V2G message due to an invalid V2G-TP header!");
        return;
    }

    if (!ok)
        message.set(enV2gMsg::Empty, enV2gMode::Req);
}

bool cExiCodec::read_V2gSdpReq(stV2gSdpReq& result, stV2gTpHeader& headerCheck, stStreamControl& stream)
{
    return ::read_V2gSdpReq(result, headerCheck, stream);
}

bool cExiCodec::read_V2gSdpRes(stV2gSdpRes& result, stV2gTpHeader& headerCheck, stStreamControl& stream)
{
    return ::read_V2gSdpRes(result, headerCheck, stream);
}

bool cExiCodec::read_V2gSupportedAppProtocolReq(stV2gSupportedAppProtocolReq& result, stStreamControl& stream)
{
    return ::read_V2gSupportedAppProtocolReq(result, stream);
}

bool cExiCodec::read_V2gSupportedAppProtocolRes(stV2gSupportedAppProtocolRes& result, stStreamControl& stream)
{
    return ::read_V2gSupportedAppProtocolRes(result, stream);
}

int cExiCodec::write_V2gSdpReq(stV2gSdpReq& data, stStreamControl& stream)
{
    return ::write_V2gSdpReq(data, stream);
}

int cExiCodec::write_V2gSdpRes(stV2gSdpRes& data, stStreamControl& stream)
{
    return ::write_V2gSdpRes(data, stream);
}

int cExiCodec::write_V2s(stStreamControl& stream)
{
    return ::write_V2s(stream);
}

int cExiCodec::write_V2gSupportedAppProtocolReq(stV2gSupportedAppProtocolReq& data, stStreamControl& stream)
{
    return ::write_V2gSupportedAppProtocolReq(data, stream);
}

int cExiCodec::write_V2gSupportedAppProtocolRes(stV2gSupportedAppProtocolRes& data, stStreamControl& stream)
{
    return ::write_V2gSupportedAppProtocolRes(data, stream);
}
