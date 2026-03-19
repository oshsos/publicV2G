// It is required to set #define DEPLOY_ISO2_CODEC SUPPORT_YES within openV2G/iso1/iso1EXIdatatypes.h

#include <cstdio>
#include <cmath>
//#include <sec_api/string_s.h>
#include <string>

#include "cExiIso15118_2013_openV2g.h"

#include "../LibCpp/HAL/Tools.h"

#include "openV2G/codec/EXITypes.h"
#include "openV2G/iso1/iso1EXIDatatypes.h"
#include "openV2G/iso1/iso1EXIDatatypesDecoder.h"
#include "openV2G/iso1/iso1EXIDatatypesEncoder.h"
//#include "openV2G/iso2/iso2EXIDatatypes.h"
//#include "openV2G/iso2/iso2EXIDatatypesDecoder.h"
//#include "openV2G/iso2/iso2EXIDatatypesEncoder.h"

using namespace std;
using namespace LibCpp;
using namespace Iso15118;

cExiIso15118_2013_openV2g::cExiIso15118_2013_openV2g()
{
}

//bool cExiIso15118_2013_openV2g::read_V2gHeaderTest(stV2gHeader& result, stStreamControl& stream)
//{
//    cDebug dbg("read_V2gHeader", &this->dbg);

//    struct iso2EXIDocument receivedExi2Document;

//    size_t pos = 0;
//    bitstream_t iStream = inStream(stream, pos);

//    int error = decode_iso2ExiDocument(&iStream, &receivedExi2Document);
//    if (error)
//    {
//        dbg.printf(LibCpp::enDebugLevel_Error, "EXI decoding failed with %i!", error);
//        stream.length = 0;
//        return false;
//    }
//    return false;
//}

bool cExiIso15118_2013_openV2g::read_V2gHeader(stV2gHeader& result, stStreamControl& stream, bool expectSuppAppProt)
{
    cDebug dbg("read_V2gHeader", &this->dbg);

    size_t pos = 0;
    bitstream_t iStream = inStream(stream, pos);

//    size_t pos = V2GTP_HEADER_LENGTH;
//    bitstream_t iStream = inStream(stream, pos);

//    bool ok = read_V2gTpHeader(result.tpHeader, stream);
//    if (!ok)
//    {
//        dbg.printf(LibCpp::enDebugLevel_Error, "Failed to decode TP header!");
//        stream.length = 0;
//        return false;
//    }
//    if (result.tpHeader.payloadType != V2GTP_TYPE_EXI)
//    {
//        dbg.printf(LibCpp::enDebugLevel_Error, "No EXI type in TP header!");
//        stream.length = 0;
//        return false;
//    }

    int error = decode_iso1ExiDocument(&iStream, &receivedExiDocument);
    if (error)
    {
        if (!expectSuppAppProt)
        {
            dbg.printf(LibCpp::enDebugLevel_Error, "EXI decoding using codec ISO15118-2013 failed with %i!", error);
        }
        stream.length = 0;
        return false;
    }

    if(!receivedExiDocument.V2G_Message_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "EXI decoded message using codec ISO15118-2013 contains no V2G message!");
        stream.length = 0;
        return false;
    }

    enV2gMsg messageType = enV2gMsg::Unimplemented;
    enV2gMode messageMode = enV2gMode::Req;

    if (receivedExiDocument.V2G_Message.Body.SessionSetupReq_isUsed)
        {messageType = enV2gMsg::SessionSetup; messageMode = enV2gMode::Req;}
    else if (receivedExiDocument.V2G_Message.Body.ServiceDiscoveryReq_isUsed)
        {messageType = enV2gMsg::ServiceDiscovery; messageMode = enV2gMode::Req;}
    else if (receivedExiDocument.V2G_Message.Body.PaymentServiceSelectionReq_isUsed)
        {messageType = enV2gMsg::PaymentServiceSelection; messageMode = enV2gMode::Req;}
    else if (receivedExiDocument.V2G_Message.Body.AuthorizationReq_isUsed)
        {messageType = enV2gMsg::Authorization; messageMode = enV2gMode::Req;}
    else if (receivedExiDocument.V2G_Message.Body.ChargeParameterDiscoveryReq_isUsed)
        {messageType = enV2gMsg::ChargeParameterDiscovery; messageMode = enV2gMode::Req;}
    else if (receivedExiDocument.V2G_Message.Body.PowerDeliveryReq_isUsed)
        {messageType = enV2gMsg::PowerDelivery; messageMode = enV2gMode::Req;}
    else if (receivedExiDocument.V2G_Message.Body.ChargingStatusReq_isUsed)
        {messageType = enV2gMsg::ChargingStatus; messageMode = enV2gMode::Req;}
    else if (receivedExiDocument.V2G_Message.Body.SessionStopReq_isUsed)
        {messageType = enV2gMsg::SessionStop; messageMode = enV2gMode::Req;}
    else if (receivedExiDocument.V2G_Message.Body.CableCheckReq_isUsed)
        {messageType = enV2gMsg::CableCheck; messageMode = enV2gMode::Req;}
    else if (receivedExiDocument.V2G_Message.Body.PreChargeReq_isUsed)
        {messageType = enV2gMsg::PreCharge; messageMode = enV2gMode::Req;}
    else if (receivedExiDocument.V2G_Message.Body.CurrentDemandReq_isUsed)
        {messageType = enV2gMsg::CurrentDemand; messageMode = enV2gMode::Req;}
    else if (receivedExiDocument.V2G_Message.Body.WeldingDetectionReq_isUsed)
        {messageType = enV2gMsg::WeldingDetection; messageMode = enV2gMode::Req;}
    else if (receivedExiDocument.V2G_Message.Body.MeteringReceiptReq_isUsed)
        {messageType = enV2gMsg::MeteringReceipt; messageMode = enV2gMode::Req;}
    else if (receivedExiDocument.V2G_Message.Body.ServiceDetailReq_isUsed)
        {messageType = enV2gMsg::ServiceDetail; messageMode = enV2gMode::Req;}
    else if (receivedExiDocument.V2G_Message.Body.PaymentDetailsReq_isUsed)
        {messageType = enV2gMsg::PaymentDetails; messageMode = enV2gMode::Req;}
    else if (receivedExiDocument.V2G_Message.Body.SessionSetupRes_isUsed)
        {messageType = enV2gMsg::SessionSetup; messageMode = enV2gMode::Res;}
    else if (receivedExiDocument.V2G_Message.Body.ServiceDiscoveryRes_isUsed)
        {messageType = enV2gMsg::ServiceDiscovery; messageMode = enV2gMode::Res;}
    else if (receivedExiDocument.V2G_Message.Body.PaymentServiceSelectionRes_isUsed)
        {messageType = enV2gMsg::PaymentServiceSelection; messageMode = enV2gMode::Res;}
    else if (receivedExiDocument.V2G_Message.Body.AuthorizationRes_isUsed)
        {messageType = enV2gMsg::Authorization; messageMode = enV2gMode::Res;}
    else if (receivedExiDocument.V2G_Message.Body.ChargeParameterDiscoveryRes_isUsed)
        {messageType = enV2gMsg::ChargeParameterDiscovery; messageMode = enV2gMode::Res;}
    else if (receivedExiDocument.V2G_Message.Body.PowerDeliveryRes_isUsed)
        {messageType = enV2gMsg::PowerDelivery; messageMode = enV2gMode::Res;}
    else if (receivedExiDocument.V2G_Message.Body.ChargingStatusRes_isUsed)
        {messageType = enV2gMsg::ChargingStatus; messageMode = enV2gMode::Res;}
    else if (receivedExiDocument.V2G_Message.Body.SessionStopRes_isUsed)
        {messageType = enV2gMsg::SessionStop; messageMode = enV2gMode::Res;}
    else if (receivedExiDocument.V2G_Message.Body.CableCheckRes_isUsed)
        {messageType = enV2gMsg::CableCheck; messageMode = enV2gMode::Res;}
    else if (receivedExiDocument.V2G_Message.Body.PreChargeRes_isUsed)
        {messageType = enV2gMsg::PreCharge; messageMode = enV2gMode::Res;}
    else if (receivedExiDocument.V2G_Message.Body.CurrentDemandRes_isUsed)
        {messageType = enV2gMsg::CurrentDemand; messageMode = enV2gMode::Res;}
    else if (receivedExiDocument.V2G_Message.Body.WeldingDetectionRes_isUsed)
        {messageType = enV2gMsg::WeldingDetection; messageMode = enV2gMode::Res;}
//        else if (receivedExiDocument.V2G_Message.Body.DisconnectChargingDeviceRes_isUsed)
//            {messageType = enV2gMsg::DisconnectChargingDevice; messageMode = enV2gMode::Res;}
    else if (receivedExiDocument.V2G_Message.Body.MeteringReceiptRes_isUsed)
        {messageType = enV2gMsg::MeteringReceipt; messageMode = enV2gMode::Res;}
    else if (receivedExiDocument.V2G_Message.Body.ServiceDetailRes_isUsed)
        {messageType = enV2gMsg::ServiceDetail; messageMode = enV2gMode::Res;}
    else if (receivedExiDocument.V2G_Message.Body.PaymentDetailsRes_isUsed)
        {messageType = enV2gMsg::PaymentDetails; messageMode = enV2gMode::Res;}

    result.messageType = messageType;
    result.messageMode = messageMode;
    result.sessionId.set((char*)receivedExiDocument.V2G_Message.Header.SessionID.bytes, receivedExiDocument.V2G_Message.Header.SessionID.bytesLen);

    if (messageType == enV2gMsg::Unimplemented)
        dbg.printf(LibCpp::enDebugLevel_Debug, "Unimplemented V2G message type received!");
    return true;
}

bool cExiIso15118_2013_openV2g::read_V2gSessionSetupReq(stV2gSessionSetupReq& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gSessionSetupReq", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::SessionSetup ||
        !receivedExiDocument.V2G_Message.Body.SessionSetupReq_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    result.header = headerCheck;
    result.evccId.set((char*)receivedExiDocument.V2G_Message.Body.SessionSetupReq.EVCCID.bytes, receivedExiDocument.V2G_Message.Body.SessionSetupReq.EVCCID.bytesLen);
    return true;
}

bool cExiIso15118_2013_openV2g::read_V2gSessionSetupRes(stV2gSessionSetupRes& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gSessionSetupRes", &this->dbg);

    result.header = headerCheck;
    if (headerCheck.messageType != enV2gMsg::SessionSetup ||
        !receivedExiDocument.V2G_Message.Body.SessionSetupRes_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    cExiByteArray id;
    id.setFromExiCharacters(receivedExiDocument.V2G_Message.Body.SessionSetupRes.EVSEID.charactersLen, receivedExiDocument.V2G_Message.Body.SessionSetupRes.EVSEID.characters);
    result.seccId = (cByteArray)id;
    if (receivedExiDocument.V2G_Message.Body.SessionSetupRes.EVSETimeStamp_isUsed)
        result.seccTimeStamp = receivedExiDocument.V2G_Message.Body.SessionSetupRes.EVSETimeStamp;
    result.responseCode = (enV2gResponse)receivedExiDocument.V2G_Message.Body.SessionSetupRes.ResponseCode;
    return true;
}

bool cExiIso15118_2013_openV2g::read_V2gServiceDiscoveryReq(stV2gServiceDiscoveryReq& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gServiceDiscoveryReq", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::ServiceDiscovery ||
        !receivedExiDocument.V2G_Message.Body.ServiceDiscoveryReq_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    result.header = headerCheck;
    result.serviceCategory = receivedExiDocument.V2G_Message.Body.ServiceDiscoveryReq.ServiceCategory;
    return true;
}

bool cExiIso15118_2013_openV2g::read_V2gServiceDiscoveryRes(stV2gServiceDiscoveryRes& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gServiceDiscoveryRes", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::ServiceDiscovery ||
        !receivedExiDocument.V2G_Message.Body.ServiceDiscoveryRes_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    iso1ServiceDiscoveryResType* pDat = &receivedExiDocument.V2G_Message.Body.ServiceDiscoveryRes;

    result.header = headerCheck;

    result.paymentOptions.clear();
    for (int i=0; i<pDat->PaymentOptionList.PaymentOption.arrayLen; i++)
        result.paymentOptions.push_back(pDat->PaymentOptionList.PaymentOption.array[i]);

    result.chargeServiceId = pDat->ChargeService.ServiceID;
    result.chargeServiceFree = pDat->ChargeService.FreeService;
    result.chargeServiceCategory = pDat->ChargeService.ServiceCategory;

    result.energyTransferModes.clear();
    for (int i=0; i<pDat->ChargeService.SupportedEnergyTransferMode.EnergyTransferMode.arrayLen; i++)
        result.energyTransferModes.push_back(pDat->ChargeService.SupportedEnergyTransferMode.EnergyTransferMode.array[i]);

    pDat->ChargeService.ServiceName_isUsed = false;
    pDat->ChargeService.ServiceScope_isUsed = false;

    result.responseCode = (enV2gResponse)pDat->ResponseCode;
    return true;
}

bool cExiIso15118_2013_openV2g::read_V2gPaymentServiceSelectionReq(stV2gPaymentServiceSelectionReq& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gPaymentServiceSelectionReq", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::PaymentServiceSelection ||
        !receivedExiDocument.V2G_Message.Body.PaymentServiceSelectionReq_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    result.header = headerCheck;
    result.selectedPaymentOption = receivedExiDocument.V2G_Message.Body.PaymentServiceSelectionReq.SelectedPaymentOption;
    result.selectedServices.clear();
    for (int i=0; i<receivedExiDocument.V2G_Message.Body.PaymentServiceSelectionReq.SelectedServiceList.SelectedService.arrayLen; i++)
        result.selectedServices.push_back(receivedExiDocument.V2G_Message.Body.PaymentServiceSelectionReq.SelectedServiceList.SelectedService.array[i]);

    return true;
}

bool cExiIso15118_2013_openV2g::read_V2gPaymentServiceSelectionRes(stV2gPaymentServiceSelectionRes& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gPaymentServiceSelectionRes", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::PaymentServiceSelection ||
        !receivedExiDocument.V2G_Message.Body.PaymentServiceSelectionRes_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    iso1PaymentServiceSelectionResType* pDat = &receivedExiDocument.V2G_Message.Body.PaymentServiceSelectionRes;

    result.header = headerCheck;

    result.responseCode = (enV2gResponse)pDat->ResponseCode;
    return true;
}

bool cExiIso15118_2013_openV2g::read_V2gAuthorizationReq(stV2gAuthorizationReq& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gAuthorizationReq", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::Authorization ||
        !receivedExiDocument.V2G_Message.Body.AuthorizationReq_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    iso1AuthorizationReqType* pDat = &receivedExiDocument.V2G_Message.Body.AuthorizationReq;

    result.header = headerCheck;

    result.genChallenge.set();
    if (pDat->GenChallenge_isUsed)
    {
        result.genChallenge.set((char*)pDat->GenChallenge.bytes, (unsigned int)pDat->GenChallenge.bytesLen);
    }

    cExiByteArray id;
    if (pDat->Id_isUsed)
    {
        id.setFromExiCharacters(pDat->Id.charactersLen, pDat->Id.characters);
    }
    result.id = id;

    return true;
}

bool cExiIso15118_2013_openV2g::read_V2gAuthorizationRes(stV2gAuthorizationRes& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gAuthorizationRes", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::Authorization ||
        !receivedExiDocument.V2G_Message.Body.AuthorizationRes_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    iso1AuthorizationResType* pDat = &receivedExiDocument.V2G_Message.Body.AuthorizationRes;

    result.header = headerCheck;
    result.responseCode = (enV2gResponse)pDat->ResponseCode;

    result.evseProcessing = pDat->EVSEProcessing;

    return true;
}

bool cExiIso15118_2013_openV2g::read_V2gChargeParameterDiscoveryReq(stV2gChargeParameterDiscoveryReq& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gChargeParameterDiscoveryReq", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::ChargeParameterDiscovery ||
        !receivedExiDocument.V2G_Message.Body.ChargeParameterDiscoveryReq_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    iso1ChargeParameterDiscoveryReqType* pDat = &receivedExiDocument.V2G_Message.Body.ChargeParameterDiscoveryReq;

    result.header = headerCheck;

    result.transferMode = pDat->RequestedEnergyTransferMode;
    if (pDat->AC_EVChargeParameter_isUsed)
    {
        result.energyRequest.read(&pDat->AC_EVChargeParameter.EAmount);
        result.maxVoltage.read(&pDat->AC_EVChargeParameter.EVMaxVoltage);
        result.maxCurrent.read(&pDat->AC_EVChargeParameter.EVMaxCurrent);
        result.minCurrent.read(&pDat->AC_EVChargeParameter.EVMinCurrent);
        if (pDat->AC_EVChargeParameter.DepartureTime_isUsed)
        {
            result.departureTime = pDat->AC_EVChargeParameter.DepartureTime;
        }
    }
    if (pDat->DC_EVChargeParameter_isUsed)
    {
        result.maxVoltage.read(&pDat->DC_EVChargeParameter.EVMaximumVoltageLimit);
        result.maxCurrent.read(&pDat->DC_EVChargeParameter.EVMaximumCurrentLimit);
        result.dcEvStatus.errorCode = pDat->DC_EVChargeParameter.DC_EVStatus.EVErrorCode;
        result.dcEvStatus.ressSOC = pDat->DC_EVChargeParameter.DC_EVStatus.EVRESSSOC;
        result.dcEvStatus.ready = pDat->DC_EVChargeParameter.DC_EVStatus.EVReady;
        if (pDat->DC_EVChargeParameter.DepartureTime_isUsed)
        {
            result.departureTime = pDat->DC_EVChargeParameter.DepartureTime;
        }
        if (pDat->DC_EVChargeParameter.EVMaximumPowerLimit_isUsed)
        {
            result.maxPower.read(&pDat->DC_EVChargeParameter.EVMaximumPowerLimit);
        }
        if (pDat->DC_EVChargeParameter.BulkSOC_isUsed)
        {
            result.bulkSOC = pDat->DC_EVChargeParameter.BulkSOC;
        }
        if (pDat->DC_EVChargeParameter.EVEnergyCapacity_isUsed)
        {
            result.energyCapacity.read(&pDat->DC_EVChargeParameter.EVEnergyCapacity);
        }
        if (pDat->DC_EVChargeParameter.EVEnergyRequest_isUsed)
        {
            result.energyRequest.read(&pDat->DC_EVChargeParameter.EVEnergyRequest);
        }
        if (pDat->DC_EVChargeParameter.FullSOC_isUsed)
        {
            result.fullSOC = pDat->DC_EVChargeParameter.FullSOC;
        }
    }
    if (pDat->EVChargeParameter_isUsed)
    {
        if (pDat->EVChargeParameter.DepartureTime_isUsed)
        {
            result.departureTime = pDat->EVChargeParameter.DepartureTime;
        }
    }
    if (pDat->MaxEntriesSAScheduleTuple_isUsed)
    {
        result.maxEntriesSAScheduleTuple = pDat->MaxEntriesSAScheduleTuple;
    }

    return true;
}

bool cExiIso15118_2013_openV2g::read_V2gChargeParameterDiscoveryRes(stV2gChargeParameterDiscoveryRes& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gChargeParameterDiscoveryRes", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::ChargeParameterDiscovery ||
        !receivedExiDocument.V2G_Message.Body.ChargeParameterDiscoveryRes_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    iso1ChargeParameterDiscoveryResType* pDat = &receivedExiDocument.V2G_Message.Body.ChargeParameterDiscoveryRes;

    result.header = headerCheck;
    result.responseCode = (enV2gResponse)pDat->ResponseCode;

    result.evseProcessing = pDat->EVSEProcessing;
    //    if (pDat->EVSEChargeParameter_isUsed)
    //        result.evseChargeParameter = pDat->EVSEChargeParameter;  // no content is defined
    if (pDat->AC_EVSEChargeParameter_isUsed)
    {
        result.evseNotification = (enV2gNotification)pDat->AC_EVSEChargeParameter.AC_EVSEStatus.EVSENotification;
        result.notificationMaxDelay = pDat->AC_EVSEChargeParameter.AC_EVSEStatus.NotificationMaxDelay;
        result.rcd = pDat->AC_EVSEChargeParameter.AC_EVSEStatus.RCD;
        result.maxCurrent.read(&pDat->AC_EVSEChargeParameter.EVSEMaxCurrent);
        result.nominalVoltage.read(&pDat->AC_EVSEChargeParameter.EVSENominalVoltage);
    }
    if (pDat->DC_EVSEChargeParameter_isUsed)
    {
        result.evseStatusCode = (enDcEvseStatusCode)pDat->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEStatusCode;
        result.evseNotification = (enV2gNotification)pDat->DC_EVSEChargeParameter.DC_EVSEStatus.EVSENotification;
        result.notificationMaxDelay = pDat->DC_EVSEChargeParameter.DC_EVSEStatus.NotificationMaxDelay;
        if (pDat->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEIsolationStatus_isUsed)
            result.evseIsolationStatus = (enIsolationLevel)pDat->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEIsolationStatus;
        else
            result.evseIsolationStatus = enIsolationLevel::invalid;
        result.maxCurrent.read(&pDat->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit);
        result.maxVoltage.read(&pDat->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit);
        result.maxPower.read(&pDat->DC_EVSEChargeParameter.EVSEMaximumPowerLimit);
        result.minCurrent.read(&pDat->DC_EVSEChargeParameter.EVSEMinimumCurrentLimit);
    }

    stSaSchedules saSchedules;
    if (pDat->SAScheduleList_isUsed)
    {
        result.saScheduleListUsed = true;

        int num = pDat->SAScheduleList.SAScheduleTuple.arrayLen;
        for (int i=0; i<num; i++)
        {
            stPmaxSchedules powerSchedules;
            int len = pDat->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.arrayLen;
            for (int j=0; j<len; j++)
            {
                stPmaxSchedule powerSchedule;
                powerSchedule.maxP.read(&pDat->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j].PMax);
                if (pDat->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j].RelativeTimeInterval_isUsed)
                {
                    powerSchedule.relativeStartTime = pDat->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j].RelativeTimeInterval.start;
                    pDat->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j].RelativeTimeInterval_isUsed = true;
                    if (pDat->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j].RelativeTimeInterval.duration_isUsed)
                        powerSchedule.relativeDuration = pDat->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j].RelativeTimeInterval.duration;
                }
                powerSchedules.list.push_back(powerSchedule);
            }
            stSaScheduleTuple scheduleTuple;
            scheduleTuple.id = pDat->SAScheduleList.SAScheduleTuple.array[i].SAScheduleTupleID;
            scheduleTuple.maxPSchedules = powerSchedules;
            saSchedules.list.push_back(scheduleTuple);
        }
    }
    else
        result.saScheduleListUsed = false;

    if (pDat->SASchedules_isUsed)
        result.saSchedulesUsed = true;
    else
        result.saSchedulesUsed = false;

    return true;
}

bool cExiIso15118_2013_openV2g::read_V2gPowerDeliveryReq(stV2gPowerDeliveryReq& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gPowerDeliveryReq", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::PowerDelivery ||
        !receivedExiDocument.V2G_Message.Body.PowerDeliveryReq_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    iso1PowerDeliveryReqType* pDat = &receivedExiDocument.V2G_Message.Body.PowerDeliveryReq;

    result.header = headerCheck;

    result.chargeProgress = pDat->ChargeProgress;
    result.saScheduleTupleId = pDat->SAScheduleTupleID;
    result.chargingProfile_isUsed = pDat->ChargingProfile_isUsed;
    result.evPowerDeliveryParameter_isUsed = pDat->EVPowerDeliveryParameter_isUsed;

    if (pDat->DC_EVPowerDeliveryParameter.BulkChargingComplete_isUsed)
    {
        result.chargingComplete = pDat->DC_EVPowerDeliveryParameter.ChargingComplete;
        result.evStatus = pDat->DC_EVPowerDeliveryParameter.DC_EVStatus;
        if (pDat->DC_EVPowerDeliveryParameter.BulkChargingComplete_isUsed)
            result.bulkChargingComplete = pDat->DC_EVPowerDeliveryParameter.BulkChargingComplete;
    }

    return true;
}

bool cExiIso15118_2013_openV2g::read_V2gPowerDeliveryRes(stV2gPowerDeliveryRes& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gPowerDeliveryRes", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::PowerDelivery ||
        !receivedExiDocument.V2G_Message.Body.PowerDeliveryRes_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    iso1PowerDeliveryResType* pDat = &receivedExiDocument.V2G_Message.Body.PowerDeliveryRes;

    result.header = headerCheck;
    result.responseCode = (enV2gResponse)pDat->ResponseCode;

    result.transferMode = iso1EnergyTransferModeType_AC_three_phase_core;
    if (pDat->EVSEStatus_isUsed)
    {
        result.evseNotification = (enV2gNotification)pDat->EVSEStatus.EVSENotification;
        result.notificationMaxDelay = pDat->EVSEStatus.NotificationMaxDelay;
    }
    if (pDat->DC_EVSEStatus_isUsed)
    {
        result.evseNotification = (enV2gNotification)pDat->DC_EVSEStatus.EVSENotification;
        result.notificationMaxDelay = pDat->DC_EVSEStatus.NotificationMaxDelay;
        result.evseStatusCode = (enDcEvseStatusCode)pDat->DC_EVSEStatus.EVSEStatusCode;
        if (pDat->DC_EVSEStatus.EVSEIsolationStatus_isUsed)
            result.evseIsolationStatus = (enIsolationLevel)pDat->DC_EVSEStatus.EVSEIsolationStatus;
    }
    if (pDat->AC_EVSEStatus_isUsed)
    {
        result.evseNotification = (enV2gNotification)pDat->AC_EVSEStatus.EVSENotification;
        result.notificationMaxDelay = pDat->AC_EVSEStatus.NotificationMaxDelay;
        result.rcd = pDat->AC_EVSEStatus.RCD;
    }

    return true;
}

bool cExiIso15118_2013_openV2g::read_V2gChargingStatusReq(stV2gChargingStatusReq& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gChargingStatusReq", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::ChargingStatus ||
        !receivedExiDocument.V2G_Message.Body.ChargingStatusReq_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    iso1ChargingStatusReqType* pDat = &receivedExiDocument.V2G_Message.Body.ChargingStatusReq;

    result.header = headerCheck;

    result.noContent = pDat->noContent;

    return true;
}

bool cExiIso15118_2013_openV2g::read_V2gChargingStatusRes(stV2gChargingStatusRes& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gChargingStatusRes", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::ChargingStatus ||
        !receivedExiDocument.V2G_Message.Body.ChargingStatusRes_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    iso1ChargingStatusResType* pDat = &receivedExiDocument.V2G_Message.Body.ChargingStatusRes;

    result.header = headerCheck;
    result.responseCode = (enV2gResponse)pDat->ResponseCode;

    cExiByteArray id;
    id.setFromExiCharacters(pDat->EVSEID.charactersLen, pDat->EVSEID.characters);
    result.seccId = id;
    result.saScheduleTupleId = pDat->SAScheduleTupleID;
    if (pDat->EVSEMaxCurrent_isUsed)
        result.maxCurrent.read(&pDat->EVSEMaxCurrent);
    result.meterInfo_isUsed = pDat->MeterInfo_isUsed;
    result.receiptRequired_isUsed = pDat->ReceiptRequired_isUsed;

    result.evseNotification = (enV2gNotification)pDat->AC_EVSEStatus.EVSENotification;
    result.notificationMaxDelay = pDat->AC_EVSEStatus.NotificationMaxDelay;
    result.rcd = pDat->AC_EVSEStatus.RCD;

    return true;
}

bool cExiIso15118_2013_openV2g::read_V2gSessionStopReq(stV2gSessionStopReq& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gSessionStopReq", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::SessionStop ||
        !receivedExiDocument.V2G_Message.Body.SessionStopReq_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    iso1SessionStopReqType* pDat = &receivedExiDocument.V2G_Message.Body.SessionStopReq;

    result.header = headerCheck;

    result.stopType = pDat->ChargingSession;

    return true;
}

bool cExiIso15118_2013_openV2g::read_V2gSessionStopRes(stV2gSessionStopRes& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gSessionStopRes", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::SessionStop ||
        !receivedExiDocument.V2G_Message.Body.SessionStopRes_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    iso1SessionStopResType* pDat = &receivedExiDocument.V2G_Message.Body.SessionStopRes;

    result.header = headerCheck;
    result.responseCode = (enV2gResponse)pDat->ResponseCode;

    return true;
}

bool cExiIso15118_2013_openV2g::read_V2gCableCheckReq(stV2gCableCheckReq& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gCableCheckReq", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::CableCheck ||
        !receivedExiDocument.V2G_Message.Body.CableCheckReq_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    iso1CableCheckReqType* pDat = &receivedExiDocument.V2G_Message.Body.CableCheckReq;

    result.header = headerCheck;

    result.evErrorCode = pDat->DC_EVStatus.EVErrorCode;
    result.evRessSoc = pDat->DC_EVStatus.EVRESSSOC;
    result.evReady = pDat->DC_EVStatus.EVReady;

    return true;
}

bool cExiIso15118_2013_openV2g::read_V2gCableCheckRes(stV2gCableCheckRes& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gCableCheckRes", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::CableCheck ||
        !receivedExiDocument.V2G_Message.Body.CableCheckRes_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    iso1CableCheckResType* pDat = &receivedExiDocument.V2G_Message.Body.CableCheckRes;

    result.header = headerCheck;
    result.responseCode = (enV2gResponse)pDat->ResponseCode;

    result.evseStatusCode = (enDcEvseStatusCode)pDat->DC_EVSEStatus.EVSEStatusCode;
    result.evseNotification = (enV2gNotification)pDat->DC_EVSEStatus.EVSENotification;
    result.notificationMaxDelay = pDat->DC_EVSEStatus.NotificationMaxDelay;
    result.evseIsolationStatus = enIsolationLevel::invalid;
    if (pDat->DC_EVSEStatus.EVSEIsolationStatus_isUsed)
        result.evseIsolationStatus =  (enIsolationLevel)pDat->DC_EVSEStatus.EVSEIsolationStatus;
    result.evseProcessing = pDat->EVSEProcessing;

    return true;
}

bool cExiIso15118_2013_openV2g::read_V2gPreChargeReq(stV2gPreChargeReq& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gPreChargeReq", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::PreCharge ||
        !receivedExiDocument.V2G_Message.Body.PreChargeReq_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    iso1PreChargeReqType* pDat = &receivedExiDocument.V2G_Message.Body.PreChargeReq;

    result.header = headerCheck;

    result.evErrorCode = pDat->DC_EVStatus.EVErrorCode;
    result.evRessSoc = pDat->DC_EVStatus.EVRESSSOC;
    result.evReady = pDat->DC_EVStatus.EVReady;
    result.evTargetVoltage.read(&pDat->EVTargetVoltage);
    result.evTargetCurrent.read(&pDat->EVTargetCurrent);

    return true;
}

bool cExiIso15118_2013_openV2g::read_V2gPreChargeRes(stV2gPreChargeRes& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gPreChargeRes", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::PreCharge ||
        !receivedExiDocument.V2G_Message.Body.PreChargeRes_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    iso1PreChargeResType* pDat = &receivedExiDocument.V2G_Message.Body.PreChargeRes;

    result.header = headerCheck;
    result.responseCode = (enV2gResponse)pDat->ResponseCode;

    result.evseStatusCode = (enDcEvseStatusCode)pDat->DC_EVSEStatus.EVSEStatusCode;
    result.evseNotification = (enV2gNotification)pDat->DC_EVSEStatus.EVSENotification;
    result.notificationMaxDelay = pDat->DC_EVSEStatus.NotificationMaxDelay;
    result.evseIsolationStatus = enIsolationLevel::invalid;
    if (pDat->DC_EVSEStatus.EVSEIsolationStatus_isUsed)
        result.evseIsolationStatus = (enIsolationLevel)pDat->DC_EVSEStatus.EVSEIsolationStatus;
    result.evsePresentVoltage.read(&pDat->EVSEPresentVoltage);

    return true;
}

bool cExiIso15118_2013_openV2g::read_V2gCurrentDemandReq(stV2gCurrentDemandReq& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gCurrentDemandReq", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::CurrentDemand ||
        !receivedExiDocument.V2G_Message.Body.CurrentDemandReq_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    iso1CurrentDemandReqType* pDat = &receivedExiDocument.V2G_Message.Body.CurrentDemandReq;

    result.header = headerCheck;

    result.bulkChargingComplete = false;
    if (pDat->BulkChargingComplete_isUsed)
        result.bulkChargingComplete = pDat->ChargingComplete;
    result.chargingComplete = pDat->ChargingComplete;
    result.remainingTimeToBulkSoC.fromFloat(0);
    if (pDat->RemainingTimeToBulkSoC_isUsed)
        result.remainingTimeToBulkSoC.read(&pDat->RemainingTimeToBulkSoC);
    result.remainingTimeToFullSoC.fromFloat(0);
    if (pDat->RemainingTimeToFullSoC_isUsed)
        result.remainingTimeToFullSoC.read(&pDat->RemainingTimeToFullSoC);
    result.evCurrentLimit.fromFloat(0);
    if (pDat->EVMaximumCurrentLimit_isUsed)
        result.evCurrentLimit.read(&pDat->EVMaximumCurrentLimit);
    result.evVoltageLimit.fromFloat(0);
    if (pDat->EVMaximumVoltageLimit_isUsed)
        result.evVoltageLimit.read(&pDat->EVMaximumVoltageLimit);
    result.evPowerLimit.fromFloat(0);
    if (pDat->EVMaximumPowerLimit_isUsed)
        result.evPowerLimit.read(&pDat->EVMaximumPowerLimit);

    result.evErrorCode = pDat->DC_EVStatus.EVErrorCode;
    result.evRessSoc = pDat->DC_EVStatus.EVRESSSOC;
    result.evReady = pDat->DC_EVStatus.EVReady;

    result.evTargetCurrent.read(&pDat->EVTargetCurrent);
    result.evTargetVoltage.read(&pDat->EVTargetVoltage);

    return true;
}

bool cExiIso15118_2013_openV2g::read_V2gCurrentDemandRes(stV2gCurrentDemandRes& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gCurrentDemandRes", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::CurrentDemand ||
        !receivedExiDocument.V2G_Message.Body.CurrentDemandRes_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    iso1CurrentDemandResType* pDat = &receivedExiDocument.V2G_Message.Body.CurrentDemandRes;

    result.header = headerCheck;
    result.responseCode = (enV2gResponse)pDat->ResponseCode;

    result.evsePresentCurrent.read(&pDat->EVSEPresentCurrent);
    result.evsePresentVoltage.read(&pDat->EVSEPresentCurrent);

    result.evseCurrentLimit.fromFloat(0);
    if (pDat->EVSEMaximumCurrentLimit_isUsed)
        result.evseCurrentLimit.read(&pDat->EVSEMaximumCurrentLimit);
    result.evseVoltageLimit.fromFloat(0);
    if (pDat->EVSEMaximumVoltageLimit_isUsed)
        result.evseVoltageLimit.read(&pDat->EVSEMaximumVoltageLimit);
    result.evsePowerLimit.fromFloat(0);
    if (pDat->EVSEMaximumPowerLimit_isUsed)
        result.evsePowerLimit.read(&pDat->EVSEMaximumPowerLimit);

    result.evseStatusCode = (enDcEvseStatusCode)pDat->DC_EVSEStatus.EVSEStatusCode;
    result.evseNotification = (enV2gNotification)pDat->DC_EVSEStatus.EVSENotification;
    result.notificationMaxDelay = pDat->DC_EVSEStatus.NotificationMaxDelay;
    result.evseIsolationStatus = enIsolationLevel::invalid;
    if (pDat->DC_EVSEStatus.EVSEIsolationStatus_isUsed)
        result.evseIsolationStatus = (enIsolationLevel)pDat->DC_EVSEStatus.EVSEIsolationStatus;

    result.evseCurrentLimitAchieved = pDat->EVSECurrentLimitAchieved;
    result.evseVoltageLimitAchieved = pDat->EVSEVoltageLimitAchieved;
    result.evsePowerLimitAchieved = pDat->EVSEPowerLimitAchieved;

    result.saScheduleTupleId = pDat->SAScheduleTupleID;

    result.meterInfo_isUsed = pDat->MeterInfo_isUsed;
    result.receiptRequired_isUsed = pDat->ReceiptRequired_isUsed;

    return true;
}

bool cExiIso15118_2013_openV2g::read_V2gWeldingDetectionReq(stV2gWeldingDetectionReq& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gWeldingDetectionReq", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::WeldingDetection ||
        !receivedExiDocument.V2G_Message.Body.WeldingDetectionReq_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    iso1WeldingDetectionReqType* pDat = &receivedExiDocument.V2G_Message.Body.WeldingDetectionReq;

    result.header = headerCheck;

    result.evErrorCode = pDat->DC_EVStatus.EVErrorCode;
    result.evRessSoc = pDat->DC_EVStatus.EVRESSSOC;
    result.evReady = pDat->DC_EVStatus.EVReady;

    return true;
}

bool cExiIso15118_2013_openV2g::read_V2gWeldingDetectionRes(stV2gWeldingDetectionRes& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gWeldingDetectionRes", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::WeldingDetection ||
        !receivedExiDocument.V2G_Message.Body.WeldingDetectionRes_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    iso1WeldingDetectionResType* pDat = &receivedExiDocument.V2G_Message.Body.WeldingDetectionRes;

    result.header = headerCheck;
    result.responseCode = (enV2gResponse)pDat->ResponseCode;

    result.evseStatusCode = (enDcEvseStatusCode)pDat->DC_EVSEStatus.EVSEStatusCode;
    result.evseNotification = (enV2gNotification)pDat->DC_EVSEStatus.EVSENotification;
    result.notificationMaxDelay = pDat->DC_EVSEStatus.NotificationMaxDelay;
    result.evseIsolationStatus = enIsolationLevel::invalid;
    if (pDat->DC_EVSEStatus.EVSEIsolationStatus_isUsed)
        result.evseIsolationStatus = (enIsolationLevel)pDat->DC_EVSEStatus.EVSEIsolationStatus;
    result.evsePresentVoltage.read(&pDat->EVSEPresentVoltage);

    return true;
}

bool cExiIso15118_2013_openV2g::read_V2gServiceDetailReq(stV2gServiceDetailReq& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gServiceDetailReq", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::ServiceDetail ||
        !receivedExiDocument.V2G_Message.Body.ServiceDetailReq_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    result.header = headerCheck;
    result.serviceId = receivedExiDocument.V2G_Message.Body.ServiceDetailReq.ServiceID;
    return true;
}

bool cExiIso15118_2013_openV2g::read_V2gServiceDetailRes(stV2gServiceDetailRes& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gServiceDetailRes", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::ServiceDiscovery ||
        !receivedExiDocument.V2G_Message.Body.ServiceDiscoveryRes_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    iso1ServiceDiscoveryResType* pDat = &receivedExiDocument.V2G_Message.Body.ServiceDiscoveryRes;

    result.header = headerCheck;

//    result.paymentOptions.clear();
//    for (int i=0; i<pDat->PaymentOptionList.PaymentOption.arrayLen; i++)
//        result.paymentOptions.push_back(pDat->PaymentOptionList.PaymentOption.array[i]);

//    result.chargeServiceId = pDat->ChargeService.ServiceID;
//    result.chargeServiceFree = pDat->ChargeService.FreeService;
//    result.chargeServiceCategory = pDat->ChargeService.ServiceCategory;

//    result.energyTransferModes.clear();
//    for (int i=0; i<pDat->ChargeService.SupportedEnergyTransferMode.EnergyTransferMode.arrayLen; i++)
//        result.energyTransferModes.push_back(pDat->ChargeService.SupportedEnergyTransferMode.EnergyTransferMode.array[i]);

//    pDat->ChargeService.ServiceName_isUsed = false;
//    pDat->ChargeService.ServiceScope_isUsed = false;

    result.responseCode = (enV2gResponse)pDat->ResponseCode;
    return true;
}

int cExiIso15118_2013_openV2g::write_ExiDocument(struct iso1EXIDocument& exi, stStreamControl& stream)
{
    cDebug dbg("write_ExiDocument", &this->dbg);

    size_t pos = V2GTP_HEADER_LENGTH;
    bitstream_t oStream = outStream(stream, pos);

    int errn = 0;
    if( (errn = encode_iso1ExiDocument(&oStream, &exi)) == 0)
    {
        stV2gTpHeader tpHeader(V2GTP_TYPE_EXI, (unsigned int)pos-V2GTP_HEADER_LENGTH);
        if (write_V2gTpHeader(tpHeader, stream))
        {
            stream.move((unsigned int)pos-V2GTP_HEADER_LENGTH);
            return (int)pos;
        }
        dbg.printf(LibCpp::enDebugLevel_Fatal, "Failed to write V2gTpHeader!");
        stream.length = 0;
        return 0;
    }
    dbg.printf(LibCpp::enDebugLevel_Fatal, "Exi encoding failed with %i! Also check send buffer size.", errn);
    stream.length = 0;
    return 0;
}

int cExiIso15118_2013_openV2g::write_V2gSessionSetupReq(stV2gSessionSetupReq& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gSessionSetupReq", &this->dbg);

    iso1EXIDocument exiOut;
    init_iso1EXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_iso1MessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_iso1BodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.SessionSetupReq_isUsed = 1u;

    init_iso1SessionSetupReqType(&exiOut.V2G_Message.Body.SessionSetupReq);

    cExiByteArray evcc_Id(data.evccId);
    evcc_Id.asChar((char*)exiOut.V2G_Message.Body.SessionSetupReq.EVCCID.bytes, 6);
    exiOut.V2G_Message.Body.SessionSetupReq.EVCCID.bytesLen = evcc_Id.size();

    return write_ExiDocument(exiOut, stream);
}

int cExiIso15118_2013_openV2g::write_V2gSessionSetupRes(stV2gSessionSetupRes& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gSessionSetupRes", &this->dbg);

    iso1EXIDocument exiOut;
    init_iso1EXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_iso1MessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_iso1BodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.SessionSetupRes_isUsed = 1u;

    init_iso1SessionSetupResType(&exiOut.V2G_Message.Body.SessionSetupRes);
    exiOut.V2G_Message.Body.SessionSetupRes.ResponseCode = (iso1responseCodeType)data.responseCode;
    cExiByteArray secc_Id(data.seccId);
    secc_Id.asExiCharacters(8, exiOut.V2G_Message.Body.SessionSetupRes.EVSEID.charactersLen, exiOut.V2G_Message.Body.SessionSetupRes.EVSEID.characters);
    //if (data.seccTimeStamp != 0x80000000)
    //{
        exiOut.V2G_Message.Body.SessionSetupRes.EVSETimeStamp_isUsed = 1u;
        exiOut.V2G_Message.Body.SessionSetupRes.EVSETimeStamp = 123456789;
    //}
    //else
    //{
    //    exiOut.V2G_Message.Body.SessionSetupRes.EVSETimeStamp_isUsed = 0u;
    //    //exiOut.V2G_Message.Body.SessionSetupRes.EVSETimeStamp = 123456789;
    //}

    return write_ExiDocument(exiOut, stream);
}

int cExiIso15118_2013_openV2g::write_V2gServiceDiscoveryReq(stV2gServiceDiscoveryReq& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gServiceDiscoveryReq", &this->dbg);

    iso1EXIDocument exiOut;
    init_iso1EXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_iso1MessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_iso1BodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.ServiceDiscoveryReq_isUsed = 1u;

    init_iso1ServiceDiscoveryReqType(&exiOut.V2G_Message.Body.ServiceDiscoveryReq);
    iso1ServiceDiscoveryReqType* pDat = &exiOut.V2G_Message.Body.ServiceDiscoveryReq;

    pDat->ServiceCategory_isUsed = true;
    pDat->ServiceCategory = data.serviceCategory;
    pDat->ServiceScope_isUsed = false;

    return write_ExiDocument(exiOut, stream);
}

int cExiIso15118_2013_openV2g::write_V2gServiceDiscoveryRes(stV2gServiceDiscoveryRes& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gServiceDiscoveryRes", &this->dbg);

    unsigned int i;

    iso1EXIDocument exiOut;
    init_iso1EXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_iso1MessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_iso1BodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.ServiceDiscoveryRes_isUsed = 1u;

    init_iso1ServiceDiscoveryResType(&exiOut.V2G_Message.Body.ServiceDiscoveryRes);
    iso1ServiceDiscoveryResType* pDat = &exiOut.V2G_Message.Body.ServiceDiscoveryRes;

    init_iso1PaymentOptionListType(&pDat->PaymentOptionList);
    pDat->PaymentOptionList.PaymentOption.arrayLen = (uint16_t)data.paymentOptions.size();
    for (i=0; i< (unsigned int)data.paymentOptions.size(); i++)
        pDat->PaymentOptionList.PaymentOption.array[i] = data.paymentOptions[i];

    init_iso1ChargeServiceType(&pDat->ChargeService);
    pDat->ChargeService.ServiceID = data.chargeServiceId;
    pDat->ChargeService.FreeService = data.chargeServiceFree;
    pDat->ChargeService.ServiceCategory = data.chargeServiceCategory;

    init_iso1SupportedEnergyTransferModeType(&pDat->ChargeService.SupportedEnergyTransferMode);
    pDat->ChargeService.SupportedEnergyTransferMode.EnergyTransferMode.arrayLen = (uint16_t)data.energyTransferModes.size();
    for (i=0; i<data.energyTransferModes.size(); i++)
        pDat->ChargeService.SupportedEnergyTransferMode.EnergyTransferMode.array[i] = data.energyTransferModes[i];

    pDat->ChargeService.ServiceName_isUsed = false;
    pDat->ChargeService.ServiceScope_isUsed = false;
    // pDat->ServiceList_isUsed = false;

    pDat->ResponseCode = iso1responseCodeType_OK;

    return write_ExiDocument(exiOut, stream);
}

int cExiIso15118_2013_openV2g::write_V2gPaymentServiceSelectionReq(stV2gPaymentServiceSelectionReq& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gPaymentServiceSelectionReq", &this->dbg);

    iso1EXIDocument exiOut;
    init_iso1EXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_iso1MessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_iso1BodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.PaymentServiceSelectionReq_isUsed = 1u;

    init_iso1PaymentServiceSelectionReqType(&exiOut.V2G_Message.Body.PaymentServiceSelectionReq);
    iso1PaymentServiceSelectionReqType* pDat = &exiOut.V2G_Message.Body.PaymentServiceSelectionReq;

    pDat->SelectedPaymentOption = data.selectedPaymentOption;

    init_iso1SelectedServiceListType(&pDat->SelectedServiceList);
    pDat->SelectedServiceList.SelectedService.arrayLen = (uint16_t)data.selectedServices.size();
    for (unsigned int i=0; i<data.selectedServices.size(); i++)
    {
        init_iso1SelectedServiceType(&pDat->SelectedServiceList.SelectedService.array[i]);
        pDat->SelectedServiceList.SelectedService.array[i].ServiceID = data.selectedServices[i].ServiceID;
        if (data.selectedServices[i].ParameterSetID != 0)
        {
            pDat->SelectedServiceList.SelectedService.array[i].ParameterSetID_isUsed = true;
            pDat->SelectedServiceList.SelectedService.array[i].ParameterSetID = data.selectedServices[i].ParameterSetID;
        }
        else
            pDat->SelectedServiceList.SelectedService.array[i].ParameterSetID_isUsed = false;
    }

    return write_ExiDocument(exiOut, stream);
}

int cExiIso15118_2013_openV2g::write_V2gPaymentServiceSelectionRes(stV2gPaymentServiceSelectionRes& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gPaymentServiceSelectionRes", &this->dbg);

    iso1EXIDocument exiOut;
    init_iso1EXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_iso1MessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_iso1BodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.PaymentServiceSelectionRes_isUsed = 1u;

    init_iso1PaymentServiceSelectionResType(&exiOut.V2G_Message.Body.PaymentServiceSelectionRes);
    iso1PaymentServiceSelectionResType* pDat = &exiOut.V2G_Message.Body.PaymentServiceSelectionRes;

    pDat->ResponseCode = (iso1responseCodeType)data.responseCode;

    return write_ExiDocument(exiOut, stream);
}

int cExiIso15118_2013_openV2g::write_V2gAuthorizationReq(stV2gAuthorizationReq& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gAuthorizationReq", &this->dbg);

    iso1EXIDocument exiOut;
    init_iso1EXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_iso1MessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_iso1BodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.AuthorizationReq_isUsed = 1u;

    init_iso1AuthorizationReqType(&exiOut.V2G_Message.Body.AuthorizationReq);
    iso1AuthorizationReqType* pDat = &exiOut.V2G_Message.Body.AuthorizationReq;

    if (data.genChallenge.size())
    {
        pDat->GenChallenge_isUsed = true;
        pDat->GenChallenge.bytesLen = data.genChallenge.size();
        data.genChallenge.asChar((char*)pDat->GenChallenge.bytes, 16);
    }
    else
        pDat->GenChallenge_isUsed = false;

    cExiByteArray exiId(data.id);
    if (data.id.size())
    {
        pDat->Id_isUsed = true;
        exiId.asExiCharacters(50, pDat->Id.charactersLen, pDat->Id.characters);
    }
    else
        pDat->GenChallenge_isUsed = false;

    return write_ExiDocument(exiOut, stream);
}

int cExiIso15118_2013_openV2g::write_V2gAuthorizationRes(stV2gAuthorizationRes& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gAuthorizationRes", &this->dbg);

    iso1EXIDocument exiOut;
    init_iso1EXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_iso1MessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_iso1BodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.AuthorizationRes_isUsed = 1u;

    init_iso1AuthorizationResType(&exiOut.V2G_Message.Body.AuthorizationRes);
    iso1AuthorizationResType* pDat = &exiOut.V2G_Message.Body.AuthorizationRes;

    pDat->ResponseCode = (iso1responseCodeType)data.responseCode;

    pDat->EVSEProcessing = data.evseProcessing;

    return write_ExiDocument(exiOut, stream);
}

int cExiIso15118_2013_openV2g::write_V2gChargeParameterDiscoveryReq(stV2gChargeParameterDiscoveryReq& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gChargeParameterDiscoveryReq", &this->dbg);

    iso1EXIDocument exiOut;
    init_iso1EXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_iso1MessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_iso1BodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.ChargeParameterDiscoveryReq_isUsed = 1u;

    init_iso1ChargeParameterDiscoveryReqType(&exiOut.V2G_Message.Body.ChargeParameterDiscoveryReq);
    iso1ChargeParameterDiscoveryReqType* pDat = &exiOut.V2G_Message.Body.ChargeParameterDiscoveryReq;

    pDat->AC_EVChargeParameter_isUsed = false;
    pDat->DC_EVChargeParameter_isUsed = false;
    pDat->RequestedEnergyTransferMode = data.transferMode;
    switch (data.transferMode)
    {
    case iso1EnergyTransferModeType_AC_single_phase_core:
    case iso1EnergyTransferModeType_AC_three_phase_core:
    {
        pDat->AC_EVChargeParameter_isUsed = true;
        init_iso1AC_EVChargeParameterType(&pDat->AC_EVChargeParameter);
        data.maxVoltage.write(&pDat->AC_EVChargeParameter.EVMaxVoltage);
        data.maxCurrent.write(&pDat->AC_EVChargeParameter.EVMaxCurrent);
        data.minCurrent.write(&pDat->AC_EVChargeParameter.EVMinCurrent);
        data.energyRequest.write(&pDat->AC_EVChargeParameter.EAmount);
        if (data.departureTime)
        {
            pDat->AC_EVChargeParameter.DepartureTime_isUsed = true;
            pDat->AC_EVChargeParameter.DepartureTime = data.departureTime;
        }
        else
            pDat->AC_EVChargeParameter.DepartureTime_isUsed = false;
        } break;
    case iso1EnergyTransferModeType_DC_core:
    case iso1EnergyTransferModeType_DC_combo_core:
    case iso1EnergyTransferModeType_DC_unique:
    case iso1EnergyTransferModeType_DC_extended:
    {
        pDat->DC_EVChargeParameter_isUsed = true;
        init_iso1DC_EVChargeParameterType(&pDat->DC_EVChargeParameter);
        data.maxVoltage.write(&pDat->DC_EVChargeParameter.EVMaximumVoltageLimit);
        data.maxCurrent.write(&pDat->DC_EVChargeParameter.EVMaximumCurrentLimit);
        if (data.maxPower.value)
        {
            pDat->DC_EVChargeParameter.EVMaximumPowerLimit_isUsed = true;
            data.maxPower.write(&pDat->DC_EVChargeParameter.EVMaximumPowerLimit);
        }
        else
            pDat->DC_EVChargeParameter.EVMaximumPowerLimit_isUsed = false;
        if (data.bulkSOC)
        {
            pDat->DC_EVChargeParameter.BulkSOC_isUsed = true;
            pDat->DC_EVChargeParameter.BulkSOC = data.bulkSOC;
        }
        else
            pDat->DC_EVChargeParameter.BulkSOC_isUsed = false;
        if (data.energyCapacity.value)
        {
            pDat->DC_EVChargeParameter.EVEnergyCapacity_isUsed = true;
            data.energyCapacity.write(&pDat->DC_EVChargeParameter.EVEnergyCapacity);
        }
        else
            pDat->DC_EVChargeParameter.EVEnergyCapacity_isUsed = false;
        if (data.energyRequest.value)
        {
            pDat->DC_EVChargeParameter.EVEnergyRequest_isUsed = true;
            data.energyRequest.write(&pDat->DC_EVChargeParameter.EVEnergyRequest);
        }
        else
            pDat->DC_EVChargeParameter.EVEnergyRequest_isUsed = false;
        if (data.fullSOC)
        {
            pDat->DC_EVChargeParameter.FullSOC_isUsed = true;
            pDat->DC_EVChargeParameter.FullSOC = data.fullSOC;
        }
        else
            pDat->DC_EVChargeParameter.FullSOC_isUsed = false;
        init_iso1DC_EVStatusType(&pDat->DC_EVChargeParameter.DC_EVStatus);
        pDat->DC_EVChargeParameter.DC_EVStatus.EVErrorCode = data.dcEvStatus.errorCode;
        pDat->DC_EVChargeParameter.DC_EVStatus.EVRESSSOC = data.dcEvStatus.ressSOC;
        pDat->DC_EVChargeParameter.DC_EVStatus.EVReady = data.dcEvStatus.ready;
        if (data.departureTime)
        {
            pDat->DC_EVChargeParameter.DepartureTime_isUsed = true;
            pDat->DC_EVChargeParameter.DepartureTime = data.departureTime;
        }
        else
            pDat->DC_EVChargeParameter.DepartureTime_isUsed = false;
    } break;
    }

    if (data.departureTime)
    {
        pDat->EVChargeParameter_isUsed = true;
        init_iso1EVChargeParameterType(&pDat->EVChargeParameter);
        pDat->EVChargeParameter.DepartureTime_isUsed = true;
        pDat->EVChargeParameter.DepartureTime = data.departureTime;
    }
    else
        pDat->EVChargeParameter_isUsed = false;

    if (data.maxEntriesSAScheduleTuple)
    {
        pDat->MaxEntriesSAScheduleTuple_isUsed = true;
        pDat->MaxEntriesSAScheduleTuple = data.maxEntriesSAScheduleTuple;
    }
    else
        pDat->MaxEntriesSAScheduleTuple_isUsed = false;

    return write_ExiDocument(exiOut, stream);
}

int cExiIso15118_2013_openV2g::write_V2gChargeParameterDiscoveryRes(stV2gChargeParameterDiscoveryRes& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gChargeParameterDiscoveryRes", &this->dbg);

    iso1EXIDocument exiOut;
    init_iso1EXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_iso1MessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_iso1BodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.ChargeParameterDiscoveryRes_isUsed = 1u;

    init_iso1ChargeParameterDiscoveryResType(&exiOut.V2G_Message.Body.ChargeParameterDiscoveryRes);
    iso1ChargeParameterDiscoveryResType* pDat = &exiOut.V2G_Message.Body.ChargeParameterDiscoveryRes;

    pDat->ResponseCode = (iso1responseCodeType)data.responseCode;

    pDat->EVSEProcessing = data.evseProcessing;
    pDat->AC_EVSEChargeParameter_isUsed = false;
    pDat->DC_EVSEChargeParameter_isUsed = false;
    switch (data.transferMode)
    {
    case iso1EnergyTransferModeType_AC_three_phase_core:
    case iso1EnergyTransferModeType_AC_single_phase_core:
    {
        pDat->AC_EVSEChargeParameter_isUsed = true;
        init_iso1AC_EVSEChargeParameterType(&pDat->AC_EVSEChargeParameter);
        init_iso1AC_EVSEStatusType(&pDat->AC_EVSEChargeParameter.AC_EVSEStatus);
        pDat->AC_EVSEChargeParameter.AC_EVSEStatus.EVSENotification = (iso1EVSENotificationType)data.evseNotification;
        pDat->AC_EVSEChargeParameter.AC_EVSEStatus.NotificationMaxDelay = data.notificationMaxDelay;
        pDat->AC_EVSEChargeParameter.AC_EVSEStatus.RCD = data.rcd;
        init_iso1PhysicalValueType(&pDat->AC_EVSEChargeParameter.EVSEMaxCurrent);
        data.maxCurrent.write(&pDat->AC_EVSEChargeParameter.EVSEMaxCurrent);
        init_iso1PhysicalValueType(&pDat->AC_EVSEChargeParameter.EVSENominalVoltage);
        data.nominalVoltage.write(&pDat->AC_EVSEChargeParameter.EVSENominalVoltage);
    } break;
    case iso1EnergyTransferModeType_DC_core:
    case iso1EnergyTransferModeType_DC_combo_core:
    case iso1EnergyTransferModeType_DC_unique:
    case iso1EnergyTransferModeType_DC_extended:
    {
        pDat->DC_EVSEChargeParameter_isUsed = true;
        init_iso1DC_EVSEChargeParameterType(&pDat->DC_EVSEChargeParameter);
        init_iso1DC_EVSEStatusType(&pDat->DC_EVSEChargeParameter.DC_EVSEStatus);
        pDat->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEStatusCode = (iso1DC_EVSEStatusCodeType)data.evseStatusCode;
        pDat->DC_EVSEChargeParameter.DC_EVSEStatus.EVSENotification = (iso1EVSENotificationType)data.evseNotification;
        pDat->DC_EVSEChargeParameter.DC_EVSEStatus.NotificationMaxDelay = data.notificationMaxDelay;
        if (data.evseIsolationStatus != enIsolationLevel::invalid)
        {
            pDat->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEIsolationStatus_isUsed = true;
            pDat->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEIsolationStatus = (iso1isolationLevelType)data.evseIsolationStatus;
        }
        else
            pDat->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEIsolationStatus_isUsed = false;
        init_iso1PhysicalValueType(&pDat->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit);
        data.maxCurrent.write(&pDat->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit);
        init_iso1PhysicalValueType(&pDat->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit);
        data.maxVoltage.write(&pDat->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit);
        init_iso1PhysicalValueType(&pDat->DC_EVSEChargeParameter.EVSEMaximumPowerLimit);
        data.maxPower.write(&pDat->DC_EVSEChargeParameter.EVSEMaximumPowerLimit);
        init_iso1PhysicalValueType(&pDat->DC_EVSEChargeParameter.EVSEMinimumCurrentLimit);
        data.minCurrent.write(&pDat->DC_EVSEChargeParameter.EVSEMinimumCurrentLimit);
        init_iso1PhysicalValueType(&pDat->DC_EVSEChargeParameter.EVSEMinimumVoltageLimit);
        data.minVoltage.write(&pDat->DC_EVSEChargeParameter.EVSEMinimumVoltageLimit);
        init_iso1PhysicalValueType(&pDat->DC_EVSEChargeParameter.EVSEPeakCurrentRipple);
        data.maxPeakCurrentRipple.write(&pDat->DC_EVSEChargeParameter.EVSEPeakCurrentRipple);
    }
    }

    stPmaxSchedule powerSchedule;
    stPmaxSchedules powerSchedules;
    powerSchedules.list.push_back(powerSchedule);

    stSaScheduleTuple scheduleTuple;
    scheduleTuple.id = 1;
    scheduleTuple.maxPSchedules = powerSchedules;
    stSaSchedules saSchedules;
    saSchedules.list.push_back(scheduleTuple);      // According to the investigated stacks one entry is mandatory although SAScheduleList seems to be a mandatory entry

    int num = (int)saSchedules.list.size();
    pDat->SAScheduleList_isUsed = num > 0;
    init_iso1SAScheduleListType(&pDat->SAScheduleList);
    if (num > 3) num = 3;
    pDat->SAScheduleList.SAScheduleTuple.arrayLen = num;
    for (int i=0; i<num; i++)
    {
        init_iso1SAScheduleTupleType(&pDat->SAScheduleList.SAScheduleTuple.array[i]);
        pDat->SAScheduleList.SAScheduleTuple.array[i].SAScheduleTupleID = (uint8_t)saSchedules.list[i].id;
        pDat->SAScheduleList.SAScheduleTuple.array[i].SalesTariff_isUsed = false;
        int len = (int)saSchedules.list[i].maxPSchedules.list.size();
        init_iso1PMaxScheduleType(&pDat->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule);
        if (len > 5) len = 5;
        pDat->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.arrayLen = len;
        for (int j=0; j<len; j++)
        {
            init_iso1PMaxScheduleEntryType(&pDat->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j]);
            init_iso1PhysicalValueType(&pDat->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j].PMax);
            saSchedules.list[i].maxPSchedules.list[j].maxP.write(&pDat->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j].PMax);
            //if (saSchedules.list[i].maxPSchedules.list[j].relativeStartTime) // First, as zero is a valid number, not using 'relative time interval is not an option. Second, Either relative or absolute time interval is mandatory but absolute time interval is not defined. Thus, relative time interval is practically mandatory so far by ISO 15118.
            {
                pDat->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j].RelativeTimeInterval_isUsed = true;
                init_iso1RelativeTimeIntervalType(&pDat->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j].RelativeTimeInterval);
                pDat->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j].RelativeTimeInterval.start = saSchedules.list[i].maxPSchedules.list[j].relativeStartTime;
                if (saSchedules.list[i].maxPSchedules.list[j].relativeDuration)
                {
                    pDat->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j].RelativeTimeInterval.duration_isUsed = true;
                    pDat->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j].RelativeTimeInterval.duration = saSchedules.list[i].maxPSchedules.list[j].relativeDuration;
                }
                else
                    pDat->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j].RelativeTimeInterval.duration_isUsed = false;
            }
            //else
            //    pDat->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j].RelativeTimeInterval_isUsed = false;
            pDat->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j].TimeInterval_isUsed = false;
        }
    }

    pDat->SASchedules_isUsed = false;

    return write_ExiDocument(exiOut, stream);
}

int cExiIso15118_2013_openV2g::write_V2gPowerDeliveryReq(stV2gPowerDeliveryReq& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gPowerDeliveryReq", &this->dbg);

    iso1EXIDocument exiOut;
    init_iso1EXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_iso1MessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_iso1BodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.PowerDeliveryReq_isUsed = 1u;

    init_iso1PowerDeliveryReqType(&exiOut.V2G_Message.Body.PowerDeliveryReq);
    iso1PowerDeliveryReqType* pDat = &exiOut.V2G_Message.Body.PowerDeliveryReq;

    pDat->ChargeProgress = data.chargeProgress;
    pDat->SAScheduleTupleID = data.saScheduleTupleId;
    pDat->ChargingProfile_isUsed = data.chargingProfile_isUsed;
    pDat->EVPowerDeliveryParameter_isUsed = data.evPowerDeliveryParameter_isUsed;
    init_iso1ChargingProfileType(&pDat->ChargingProfile);
    pDat->ChargingProfile.ProfileEntry.arrayLen = 1;
    for (int i=0; i<1; i++)
    {
        init_iso1ProfileEntryType(&pDat->ChargingProfile.ProfileEntry.array[i]);
        pDat->ChargingProfile.ProfileEntry.array[i].ChargingProfileEntryMaxNumberOfPhasesInUse_isUsed = false;
        pDat->ChargingProfile.ProfileEntry.array[i].ChargingProfileEntryStart = 0;
        stExiPysicalValue value;
        value.fromFloat(0, iso1unitSymbolType_W);
        value.write(&pDat->ChargingProfile.ProfileEntry.array[i].ChargingProfileEntryMaxPower);
    }

    switch (data.transferMode)
    {
    case iso1EnergyTransferModeType_AC_single_phase_core:
    case iso1EnergyTransferModeType_AC_three_phase_core:
    {
    } break;
    case iso1EnergyTransferModeType_DC_core:
    case iso1EnergyTransferModeType_DC_combo_core:
    case iso1EnergyTransferModeType_DC_unique:
    case iso1EnergyTransferModeType_DC_extended:
    {
        pDat->DC_EVPowerDeliveryParameter_isUsed = true;
        pDat->DC_EVPowerDeliveryParameter.ChargingComplete = data.chargingComplete;
        pDat->DC_EVPowerDeliveryParameter.DC_EVStatus = data.evStatus;
        if (data.bulkChargingComplete)
        {
            pDat->DC_EVPowerDeliveryParameter.BulkChargingComplete_isUsed = true;
            pDat->DC_EVPowerDeliveryParameter.BulkChargingComplete = data.bulkChargingComplete;
        }
        else
            pDat->DC_EVPowerDeliveryParameter.BulkChargingComplete_isUsed = false;
    } break;
    }

    return write_ExiDocument(exiOut, stream);
}

int cExiIso15118_2013_openV2g::write_V2gPowerDeliveryRes(stV2gPowerDeliveryRes& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gPowerDeliveryRes", &this->dbg);

    iso1EXIDocument exiOut;
    init_iso1EXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_iso1MessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_iso1BodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.PowerDeliveryRes_isUsed = 1u;

    init_iso1PowerDeliveryResType(&exiOut.V2G_Message.Body.PowerDeliveryRes);
    iso1PowerDeliveryResType* pDat = &exiOut.V2G_Message.Body.PowerDeliveryRes;

    pDat->ResponseCode = (iso1responseCodeType)data.responseCode;

    pDat->EVSEStatus_isUsed = false;
    pDat->AC_EVSEStatus_isUsed = false;
    pDat->AC_EVSEStatus_isUsed = false;
    switch (data.transferMode)
    {
    case iso1EnergyTransferModeType_AC_three_phase_core:
    case iso1EnergyTransferModeType_AC_single_phase_core:
    {
        pDat->AC_EVSEStatus_isUsed = true;
        pDat->AC_EVSEStatus.EVSENotification = (iso1EVSENotificationType)data.evseNotification;
        pDat->AC_EVSEStatus.NotificationMaxDelay = data.notificationMaxDelay;
        pDat->AC_EVSEStatus.RCD = data.rcd;
    } break;
    case iso1EnergyTransferModeType_DC_core:
    case iso1EnergyTransferModeType_DC_combo_core:
    case iso1EnergyTransferModeType_DC_unique:
    case iso1EnergyTransferModeType_DC_extended:
    {
        pDat->DC_EVSEStatus_isUsed = true;
        pDat->DC_EVSEStatus.EVSENotification = (iso1EVSENotificationType)data.evseNotification;
        pDat->DC_EVSEStatus.NotificationMaxDelay = data.notificationMaxDelay;
        pDat->DC_EVSEStatus.EVSEStatusCode = (iso1DC_EVSEStatusCodeType)data.evseStatusCode;
        if (data.evseIsolationStatus != enIsolationLevel::invalid)
        {
            pDat->DC_EVSEStatus.EVSEIsolationStatus_isUsed = true;
            pDat->DC_EVSEStatus.EVSEIsolationStatus = (iso1isolationLevelType)data.evseIsolationStatus;
        }
        else
            pDat->DC_EVSEStatus.EVSEIsolationStatus_isUsed = false;
    }
    }

    return write_ExiDocument(exiOut, stream);
}

int cExiIso15118_2013_openV2g::write_V2gChargingStatusReq(stV2gChargingStatusReq& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gChargingStatusReq", &this->dbg);

    iso1EXIDocument exiOut;
    init_iso1EXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_iso1MessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_iso1BodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.ChargingStatusReq_isUsed = 1u;

    init_iso1ChargingStatusReqType(&exiOut.V2G_Message.Body.ChargingStatusReq);

    //    iso1ChargingStatusReqType* pDat = &exiOut.V2G_Message.Body.ChargingStatusReq;

//    pDat->
//    pDat->ChargeProgress = data.chargeProgress;
//    pDat->SAScheduleTupleID = data.saScheduleTuppleId;
//    pDat->ChargingProfile_isUsed = data.chargingProfile_isUsed;
//    pDat->EVPowerDeliveryParameter_isUsed = data.evPowerDeliveryParameter_isUsed;

//    switch (data.transferMode)
//    {
//    case iso1EnergyTransferModeType_AC_single_phase_core:
//    case iso1EnergyTransferModeType_AC_three_phase_core:
//    {
//    } break;
//    case iso1EnergyTransferModeType_DC_core:
//    case iso1EnergyTransferModeType_DC_combo_core:
//    case iso1EnergyTransferModeType_DC_unique:
//    case iso1EnergyTransferModeType_DC_extended:
//    {
//        pDat->DC_EVPowerDeliveryParameter_isUsed = true;
//        pDat->DC_EVPowerDeliveryParameter.ChargingComplete = data.chargingComplete;
//        pDat->DC_EVPowerDeliveryParameter.DC_EVStatus = data.evStatus;
//        if (data.bulkChargingComplete)
//        {
//            pDat->DC_EVPowerDeliveryParameter.BulkChargingComplete_isUsed = true;
//            pDat->DC_EVPowerDeliveryParameter.BulkChargingComplete = data.bulkChargingComplete;
//        }
//        else
//            pDat->DC_EVPowerDeliveryParameter.BulkChargingComplete_isUsed = false;
//    } break;
//    }

    return write_ExiDocument(exiOut, stream);
}

int cExiIso15118_2013_openV2g::write_V2gChargingStatusRes(stV2gChargingStatusRes& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gChargingStatusRes", &this->dbg);

    iso1EXIDocument exiOut;
    init_iso1EXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_iso1MessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_iso1BodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.ChargingStatusRes_isUsed = 1u;

    init_iso1ChargingStatusResType(&exiOut.V2G_Message.Body.ChargingStatusRes);
    iso1ChargingStatusResType* pDat = &exiOut.V2G_Message.Body.ChargingStatusRes;

    pDat->ResponseCode = (iso1responseCodeType)data.responseCode;

    cExiByteArray id = data.seccId;
    id.asExiCharacters(7, pDat->EVSEID.charactersLen, pDat->EVSEID.characters);
    pDat->SAScheduleTupleID = (uint8_t)data.saScheduleTupleId;
    pDat->EVSEMaxCurrent_isUsed = false;
    if (data.maxCurrent.value)
    {
        pDat->EVSEMaxCurrent_isUsed = true;
        init_iso1PhysicalValueType(&pDat->EVSEMaxCurrent);
        data.maxCurrent.write(&pDat->EVSEMaxCurrent);
    }

    pDat->AC_EVSEStatus.EVSENotification = (iso1EVSENotificationType)data.evseNotification;
    pDat->AC_EVSEStatus.NotificationMaxDelay = data.notificationMaxDelay;
    pDat->AC_EVSEStatus.RCD = data.rcd;

    pDat->MeterInfo_isUsed = false;
    pDat->ReceiptRequired_isUsed = false;

    return write_ExiDocument(exiOut, stream);
}

int cExiIso15118_2013_openV2g::write_V2gSessionStopReq(stV2gSessionStopReq& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gSessionStopReq", &this->dbg);

    iso1EXIDocument exiOut;
    init_iso1EXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_iso1MessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_iso1BodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.SessionStopReq_isUsed = 1u;

    init_iso1SessionStopReqType(&exiOut.V2G_Message.Body.SessionStopReq);
    iso1SessionStopReqType* pDat = &exiOut.V2G_Message.Body.SessionStopReq;

    pDat->ChargingSession = data.stopType;

    return write_ExiDocument(exiOut, stream);
}

int cExiIso15118_2013_openV2g::write_V2gSessionStopRes(stV2gSessionStopRes& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gSessionStopRes", &this->dbg);

    iso1EXIDocument exiOut;
    init_iso1EXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_iso1MessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_iso1BodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.SessionStopRes_isUsed = 1u;

    init_iso1SessionStopResType(&exiOut.V2G_Message.Body.SessionStopRes);
    iso1SessionStopResType* pDat = &exiOut.V2G_Message.Body.SessionStopRes;

    pDat->ResponseCode = (iso1responseCodeType)data.responseCode;

    return write_ExiDocument(exiOut, stream);
}

int cExiIso15118_2013_openV2g::write_V2gCableCheckReq(stV2gCableCheckReq& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gCableCheckReq", &this->dbg);

    iso1EXIDocument exiOut;
    init_iso1EXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_iso1MessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_iso1BodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.CableCheckReq_isUsed = 1u;

    init_iso1CableCheckReqType(&exiOut.V2G_Message.Body.CableCheckReq);
    iso1CableCheckReqType* pDat = &exiOut.V2G_Message.Body.CableCheckReq;

    pDat->DC_EVStatus.EVErrorCode = (iso1DC_EVErrorCodeType)data.evErrorCode;
    pDat->DC_EVStatus.EVRESSSOC = data.evRessSoc;
    pDat->DC_EVStatus.EVReady = data.evReady;

    return write_ExiDocument(exiOut, stream);
}

int cExiIso15118_2013_openV2g::write_V2gCableCheckRes(stV2gCableCheckRes& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gCableCheckRes", &this->dbg);

    iso1EXIDocument exiOut;
    init_iso1EXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_iso1MessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_iso1BodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.CableCheckRes_isUsed = 1u;

    init_iso1CableCheckResType(&exiOut.V2G_Message.Body.CableCheckRes);
    iso1CableCheckResType* pDat = &exiOut.V2G_Message.Body.CableCheckRes;

    pDat->ResponseCode = (iso1responseCodeType)data.responseCode;

    init_iso1DC_EVSEStatusType(&pDat->DC_EVSEStatus);
    pDat->EVSEProcessing = data.evseProcessing;
    pDat->DC_EVSEStatus.EVSEStatusCode = (iso1DC_EVSEStatusCodeType)data.evseStatusCode;
    pDat->DC_EVSEStatus.EVSENotification = (iso1EVSENotificationType)data.evseNotification;
    pDat->DC_EVSEStatus.NotificationMaxDelay = data.notificationMaxDelay;
    pDat->DC_EVSEStatus.EVSEIsolationStatus_isUsed = false;
    if (data.evseIsolationStatus == enIsolationLevel::invalid)
    {
        pDat->DC_EVSEStatus.EVSEIsolationStatus_isUsed = true;
        pDat->DC_EVSEStatus.EVSEIsolationStatus = (iso1isolationLevelType)data.evseIsolationStatus;
    }

    return write_ExiDocument(exiOut, stream);
}

int cExiIso15118_2013_openV2g::write_V2gPreChargeReq(stV2gPreChargeReq& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gPreChargeReq", &this->dbg);

    iso1EXIDocument exiOut;
    init_iso1EXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_iso1MessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_iso1BodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.PreChargeReq_isUsed = 1u;

    init_iso1PreChargeReqType(&exiOut.V2G_Message.Body.PreChargeReq);
    iso1PreChargeReqType* pDat = &exiOut.V2G_Message.Body.PreChargeReq;

    init_iso1DC_EVStatusType(&pDat->DC_EVStatus);
    pDat->DC_EVStatus.EVErrorCode = (iso1DC_EVErrorCodeType)data.evErrorCode;
    pDat->DC_EVStatus.EVRESSSOC = data.evRessSoc;
    pDat->DC_EVStatus.EVReady = data.evReady;
    init_iso1PhysicalValueType(&pDat->EVTargetVoltage);
    data.evTargetVoltage.write(&pDat->EVTargetVoltage);
    init_iso1PhysicalValueType(&pDat->EVTargetCurrent);
    data.evTargetCurrent.write(&pDat->EVTargetCurrent);

    return write_ExiDocument(exiOut, stream);
}

int cExiIso15118_2013_openV2g::write_V2gPreChargeRes(stV2gPreChargeRes& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gPreChargeRes", &this->dbg);

    iso1EXIDocument exiOut;
    init_iso1EXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_iso1MessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_iso1BodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.PreChargeRes_isUsed = 1u;

    init_iso1PreChargeResType(&exiOut.V2G_Message.Body.PreChargeRes);
    iso1PreChargeResType* pDat = &exiOut.V2G_Message.Body.PreChargeRes;

    pDat->ResponseCode = (iso1responseCodeType)data.responseCode;

    init_iso1DC_EVSEStatusType(&pDat->DC_EVSEStatus);
    pDat->DC_EVSEStatus.EVSEStatusCode = (iso1DC_EVSEStatusCodeType)data.evseStatusCode;
    pDat->DC_EVSEStatus.EVSENotification = (iso1EVSENotificationType)data.evseNotification;
    pDat->DC_EVSEStatus.NotificationMaxDelay = data.notificationMaxDelay;
    pDat->DC_EVSEStatus.EVSEIsolationStatus_isUsed = false;
    if (data.evseIsolationStatus != enIsolationLevel::invalid)
    {
        pDat->DC_EVSEStatus.EVSEIsolationStatus_isUsed = true;
        pDat->DC_EVSEStatus.EVSEIsolationStatus = (iso1isolationLevelType)data.evseIsolationStatus;
    }
    init_iso1PhysicalValueType(&pDat->EVSEPresentVoltage);
    data.evsePresentVoltage.write(&pDat->EVSEPresentVoltage);

    return write_ExiDocument(exiOut, stream);
}

int cExiIso15118_2013_openV2g::write_V2gCurrentDemandReq(stV2gCurrentDemandReq& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gCurrentDemandReq", &this->dbg);

    iso1EXIDocument exiOut;
    init_iso1EXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_iso1MessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_iso1BodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.CurrentDemandReq_isUsed = 1u;

    init_iso1CurrentDemandReqType(&exiOut.V2G_Message.Body.CurrentDemandReq);
    iso1CurrentDemandReqType* pDat = &exiOut.V2G_Message.Body.CurrentDemandReq;

    init_iso1DC_EVStatusType(&pDat->DC_EVStatus);
    pDat->DC_EVStatus.EVErrorCode = (iso1DC_EVErrorCodeType)data.evErrorCode;
    pDat->DC_EVStatus.EVRESSSOC = data.evRessSoc;
    pDat->DC_EVStatus.EVReady = data.evReady;

    init_iso1PhysicalValueType(&pDat->EVTargetVoltage);
    data.evTargetVoltage.write(&pDat->EVTargetVoltage);
    init_iso1PhysicalValueType(&pDat->EVTargetCurrent);
    data.evTargetCurrent.write(&pDat->EVTargetCurrent);

    pDat->EVMaximumCurrentLimit_isUsed = false;
    if (data.evCurrentLimit.value)
    {
        pDat->EVMaximumCurrentLimit_isUsed = true;
        init_iso1PhysicalValueType(&pDat->EVMaximumCurrentLimit);
        data.evCurrentLimit.write(&pDat->EVMaximumCurrentLimit);
    }
    pDat->EVMaximumVoltageLimit_isUsed = false;
    if (data.evVoltageLimit.value)
    {
        pDat->EVMaximumVoltageLimit_isUsed = true;
        init_iso1PhysicalValueType(&pDat->EVMaximumVoltageLimit);
        data.evVoltageLimit.write(&pDat->EVMaximumVoltageLimit);
    }
    pDat->EVMaximumPowerLimit_isUsed = false;
    if (data.evCurrentLimit.value)
    {
        pDat->EVMaximumPowerLimit_isUsed = true;
        init_iso1PhysicalValueType(&pDat->EVMaximumPowerLimit);
        data.evPowerLimit.write(&pDat->EVMaximumPowerLimit);
    }

    pDat->BulkChargingComplete_isUsed = false;
    if (data.bulkChargingComplete)
    {
        pDat->BulkChargingComplete_isUsed = true;
        pDat->BulkChargingComplete = data.bulkChargingComplete;
    }
    pDat->RemainingTimeToBulkSoC_isUsed = false;
    if (data.remainingTimeToBulkSoC.value)
    {
        pDat->RemainingTimeToBulkSoC_isUsed = true;
        init_iso1PhysicalValueType(&pDat->RemainingTimeToBulkSoC);
        data.remainingTimeToBulkSoC.write(&pDat->RemainingTimeToBulkSoC);
    }
    pDat->RemainingTimeToFullSoC_isUsed = false;
    if (data.remainingTimeToFullSoC.value)
    {
        pDat->RemainingTimeToFullSoC_isUsed = true;
        init_iso1PhysicalValueType(&pDat->RemainingTimeToFullSoC);
        data.remainingTimeToFullSoC.write(&pDat->RemainingTimeToFullSoC);
    }
    // pDat->ChargingComplete fehlt.

    return write_ExiDocument(exiOut, stream);
}

int cExiIso15118_2013_openV2g::write_V2gCurrentDemandRes(stV2gCurrentDemandRes& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gCurrentDemandRes", &this->dbg);

    iso1EXIDocument exiOut;
    init_iso1EXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_iso1MessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_iso1BodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.CurrentDemandRes_isUsed = 1u;

    init_iso1CurrentDemandResType(&exiOut.V2G_Message.Body.CurrentDemandRes);
    iso1CurrentDemandResType* pDat = &exiOut.V2G_Message.Body.CurrentDemandRes;

    pDat->ResponseCode = (iso1responseCodeType)data.responseCode;

    cExiByteArray id = data.seccId;
    id.asExiCharacters(7, pDat->EVSEID.charactersLen, pDat->EVSEID.characters);

    init_iso1DC_EVSEStatusType(&pDat->DC_EVSEStatus);
    pDat->DC_EVSEStatus.EVSEStatusCode = (iso1DC_EVSEStatusCodeType)data.evseStatusCode;
    pDat->DC_EVSEStatus.EVSENotification = (iso1EVSENotificationType)data.evseNotification;
    pDat->DC_EVSEStatus.NotificationMaxDelay = data.notificationMaxDelay;
    pDat->DC_EVSEStatus.EVSEIsolationStatus_isUsed = false;
    if (data.evseIsolationStatus != enIsolationLevel::invalid)
    {
        pDat->DC_EVSEStatus.EVSEIsolationStatus_isUsed = true;
        pDat->DC_EVSEStatus.EVSEIsolationStatus = (iso1isolationLevelType)data.evseIsolationStatus;
    }

    init_iso1PhysicalValueType(&pDat->EVSEPresentCurrent);
    data.evsePresentCurrent.write(&pDat->EVSEPresentCurrent);
    init_iso1PhysicalValueType(&pDat->EVSEPresentVoltage);
    data.evsePresentVoltage.write(&pDat->EVSEPresentVoltage);

    pDat->EVSEMaximumCurrentLimit_isUsed = false;
    if (data.evseCurrentLimit.value)
    {
        pDat->EVSEMaximumCurrentLimit_isUsed = true;
        init_iso1PhysicalValueType(&pDat->EVSEMaximumCurrentLimit);
        data.evseCurrentLimit.write(&pDat->EVSEMaximumCurrentLimit);
    }
    pDat->EVSEMaximumVoltageLimit_isUsed = false;
    if (data.evseVoltageLimit.value)
    {
        pDat->EVSEMaximumVoltageLimit_isUsed = true;
        init_iso1PhysicalValueType(&pDat->EVSEMaximumVoltageLimit);
        data.evseVoltageLimit.write(&pDat->EVSEMaximumVoltageLimit);
    }
    pDat->EVSEMaximumPowerLimit_isUsed = false;
    if (data.evseCurrentLimit.value)
    {
        pDat->EVSEMaximumPowerLimit_isUsed = true;
        init_iso1PhysicalValueType(&pDat->EVSEMaximumPowerLimit);
        data.evsePowerLimit.write(&pDat->EVSEMaximumPowerLimit);
    }

    pDat->EVSECurrentLimitAchieved = data.evseCurrentLimitAchieved;
    pDat->EVSEVoltageLimitAchieved = data.evseVoltageLimitAchieved;
    pDat->EVSEPowerLimitAchieved = data.evsePowerLimitAchieved;

    pDat->SAScheduleTupleID = data.saScheduleTupleId;
    pDat->MeterInfo_isUsed = false;
    pDat->ReceiptRequired_isUsed = false;

    return write_ExiDocument(exiOut, stream);
}

int cExiIso15118_2013_openV2g::write_V2gWeldingDetectionReq(stV2gWeldingDetectionReq& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gWeldingDetectionReq", &this->dbg);

    iso1EXIDocument exiOut;
    init_iso1EXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_iso1MessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_iso1BodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.WeldingDetectionReq_isUsed = 1u;

    init_iso1WeldingDetectionReqType(&exiOut.V2G_Message.Body.WeldingDetectionReq);
    iso1WeldingDetectionReqType* pDat = &exiOut.V2G_Message.Body.WeldingDetectionReq;

    init_iso1DC_EVStatusType(&pDat->DC_EVStatus);
    pDat->DC_EVStatus.EVErrorCode = (iso1DC_EVErrorCodeType)data.evErrorCode;
    pDat->DC_EVStatus.EVRESSSOC = data.evRessSoc;
    pDat->DC_EVStatus.EVReady = data.evReady;

    return write_ExiDocument(exiOut, stream);
}

int cExiIso15118_2013_openV2g::write_V2gWeldingDetectionRes(stV2gWeldingDetectionRes& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gWeldingDetectionRes", &this->dbg);

    iso1EXIDocument exiOut;
    init_iso1EXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_iso1MessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_iso1BodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.WeldingDetectionRes_isUsed = 1u;

    init_iso1WeldingDetectionResType(&exiOut.V2G_Message.Body.WeldingDetectionRes);
    iso1WeldingDetectionResType* pDat = &exiOut.V2G_Message.Body.WeldingDetectionRes;

    pDat->ResponseCode = (iso1responseCodeType)data.responseCode;

    init_iso1DC_EVSEStatusType(&pDat->DC_EVSEStatus);
    pDat->DC_EVSEStatus.EVSEStatusCode = (iso1DC_EVSEStatusCodeType)data.evseStatusCode;
    pDat->DC_EVSEStatus.EVSENotification = (iso1EVSENotificationType)data.evseNotification;
    pDat->DC_EVSEStatus.NotificationMaxDelay = data.notificationMaxDelay;
    pDat->DC_EVSEStatus.EVSEIsolationStatus_isUsed = false;
    if (data.evseIsolationStatus != enIsolationLevel::invalid)
    {
        pDat->DC_EVSEStatus.EVSEIsolationStatus_isUsed = true;
        pDat->DC_EVSEStatus.EVSEIsolationStatus = (iso1isolationLevelType)data.evseIsolationStatus;
    }
    init_iso1PhysicalValueType(&pDat->EVSEPresentVoltage);
    data.evsePresentVoltage.write(&pDat->EVSEPresentVoltage);

    return write_ExiDocument(exiOut, stream);
}

int cExiIso15118_2013_openV2g::write_V2gServiceDetailReq(stV2gServiceDetailReq& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gServiceDetailReq", &this->dbg);

    iso1EXIDocument exiOut;
    init_iso1EXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_iso1MessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_iso1BodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.ServiceDetailReq_isUsed = 1u;

    init_iso1ServiceDetailReqType(&exiOut.V2G_Message.Body.ServiceDetailReq);
    iso1ServiceDetailReqType* pDat = &exiOut.V2G_Message.Body.ServiceDetailReq;

    pDat->ServiceID = data.serviceId;

    return write_ExiDocument(exiOut, stream);
}

int cExiIso15118_2013_openV2g::write_V2gServiceDetailRes(stV2gServiceDetailRes& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gServiceDetailRes", &this->dbg);

    iso1EXIDocument exiOut;
    init_iso1EXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_iso1MessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_iso1BodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.ServiceDetailRes_isUsed = 1u;

    init_iso1ServiceDetailResType(&exiOut.V2G_Message.Body.ServiceDetailRes);
    iso1ServiceDetailResType* pDat = &exiOut.V2G_Message.Body.ServiceDetailRes;

    pDat->ResponseCode = (iso1responseCodeType)data.responseCode;

    pDat->ServiceID = data.serviceId;

    pDat->ServiceParameterList_isUsed = 0u;
    pDat->ServiceParameterList.ParameterSet.arrayLen = 0;

    return write_ExiDocument(exiOut, stream);
}

