// It is required to set #define DEPLOY_DIN_CODEC SUPPORT_YES within openV2G/din/dinEXIdatatypes.h

#include <cstdio>
#include <cmath>
//#include <sec_api/string_s.h> nicht für Linux
#include <string>

#include "cExiDin70121openV2g.h"

//#include "../LibCpp/HAL/HW_Tools.h"
#include "../LibCpp/HAL/Tools.h"
#include "../LibCpp/HAL/cDebug.h"

#include "openV2G/codec/EXITypes.h"
//#include "openV2G/appHandshake/appHandEXIDatatypes.h"
//#include "openV2G/appHandshake/appHandEXIDatatypesDecoder.h"
//#include "openV2G/appHandshake/appHandEXIDatatypesEncoder.h"
//#include "openV2G/din/dinEXIDatatypes.h"
#include "openV2G/din/dinEXIDatatypesDecoder.h"
#include "openV2G/din/dinEXIDatatypesEncoder.h"

using namespace std;
using namespace LibCpp;
using namespace Iso15118;

cExiDin70121openV2g::cExiDin70121openV2g()
{
}

bool cExiDin70121openV2g::read_V2gHeader(stV2gHeader& result, stStreamControl& stream, bool expectSuppAppProt)
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

    int error = decode_dinExiDocument(&iStream, &receivedExiDocument);
    if (error)
    {
        if (!expectSuppAppProt)
            dbg.printf(LibCpp::enDebugLevel_Error, "EXI decoding using codec DIN SPEC failed with %i!", error);
        stream.length = 0;
        return false;
    }

    if(!receivedExiDocument.V2G_Message_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "EXI decoded message using codec DIN SPEC contains no V2G message!");
        stream.length = 0;
        return false;
    }

    enV2gMsg messageType = enV2gMsg::Unimplemented;
    enV2gMode messageMode = enV2gMode::Req;

    if (receivedExiDocument.V2G_Message.Body.SessionSetupReq_isUsed)
        {messageType = enV2gMsg::SessionSetup; messageMode = enV2gMode::Req;}
    else if (receivedExiDocument.V2G_Message.Body.ServiceDiscoveryReq_isUsed)
        {messageType = enV2gMsg::ServiceDiscovery; messageMode = enV2gMode::Req;}
    else if (receivedExiDocument.V2G_Message.Body.ServicePaymentSelectionReq_isUsed)
        {messageType = enV2gMsg::PaymentServiceSelection; messageMode = enV2gMode::Req;}
    else if (receivedExiDocument.V2G_Message.Body.ContractAuthenticationReq_isUsed)
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
    else if (receivedExiDocument.V2G_Message.Body.ServicePaymentSelectionRes_isUsed)
        {messageType = enV2gMsg::PaymentServiceSelection; messageMode = enV2gMode::Res;}
    else if (receivedExiDocument.V2G_Message.Body.ContractAuthenticationRes_isUsed)
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

bool cExiDin70121openV2g::read_V2gSessionSetupReq(stV2gSessionSetupReq& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gSessionSetupReq", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::SessionSetup ||
        !receivedExiDocument.V2G_Message.Body.SessionSetupReq_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    dinSessionSetupReqType* pDat = &receivedExiDocument.V2G_Message.Body.SessionSetupReq;

    result.header = headerCheck;

    result.evccId.set((char*)pDat->EVCCID.bytes, pDat->EVCCID.bytesLen);
    return true;
}

bool cExiDin70121openV2g::read_V2gSessionSetupRes(stV2gSessionSetupRes& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gSessionSetupRes", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::SessionSetup ||
        !receivedExiDocument.V2G_Message.Body.SessionSetupRes_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    dinSessionSetupResType* pDat = &receivedExiDocument.V2G_Message.Body.SessionSetupRes;

    result.header = headerCheck;
    result.responseCode = (enV2gResponse)pDat->ResponseCode;

    cExiByteArray id;
    id.set((char*)pDat->EVSEID.bytes, (unsigned int)pDat->EVSEID.bytesLen);
    result.seccId = (cByteArray)id;
    if (pDat->DateTimeNow_isUsed)
        result.seccTimeStamp = pDat->DateTimeNow;

    return true;
}

bool cExiDin70121openV2g::read_V2gServiceDiscoveryReq(stV2gServiceDiscoveryReq& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gServiceDiscoveryReq", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::ServiceDiscovery ||
        !receivedExiDocument.V2G_Message.Body.ServiceDiscoveryReq_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    dinServiceDiscoveryReqType* pDat = &receivedExiDocument.V2G_Message.Body.ServiceDiscoveryReq;

    result.header = headerCheck;

    result.serviceCategory = (iso1serviceCategoryType)pDat->ServiceCategory;
    return true;
}

bool cExiDin70121openV2g::read_V2gServiceDiscoveryRes(stV2gServiceDiscoveryRes& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gServiceDiscoveryRes", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::ServiceDiscovery ||
        !receivedExiDocument.V2G_Message.Body.ServiceDiscoveryRes_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    dinServiceDiscoveryResType* pDat = &receivedExiDocument.V2G_Message.Body.ServiceDiscoveryRes;

    result.header = headerCheck;
    result.responseCode = (enV2gResponse)pDat->ResponseCode;

    result.paymentOptions.clear();    // vector<paymentOptionType
    for (int i=0; i<pDat->PaymentOptions.PaymentOption.arrayLen; i++)
        result.paymentOptions.push_back((iso1paymentOptionType)pDat->PaymentOptions.PaymentOption.array[i]);

    result.chargeServiceFree = pDat->ChargeService.FreeService;
    result.chargeServiceId = pDat->ChargeService.ServiceTag.ServiceID;
    result.chargeServiceCategory = (iso1serviceCategoryType)pDat->ChargeService.ServiceTag.ServiceCategory;

    dinEVSESupportedEnergyTransferType energyType = pDat->ChargeService.EnergyTransferType;
    if (energyType >= dinEVSESupportedEnergyTransferType_DC_dual)
    {
        dbg.printf(enDebugLevel_Error, "Implementation does not support dinEVSESupportedEnergyTransferType = %i!", (int)energyType);
        energyType = dinEVSESupportedEnergyTransferType_DC_dual;
    }
    result.energyTransferModes.clear();
    result.energyTransferModes.push_back((iso1EnergyTransferModeType)energyType);

    //pDat->ChargeService.ServiceName_isUsed = false;
    //pDat->ChargeService.ServiceScope_isUsed = false;

    return true;
}

bool cExiDin70121openV2g::read_V2gPaymentServiceSelectionReq(stV2gPaymentServiceSelectionReq& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gPaymentServiceSelectionReq", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::PaymentServiceSelection ||
        !receivedExiDocument.V2G_Message.Body.ServicePaymentSelectionReq_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    dinServicePaymentSelectionReqType* pDat = &receivedExiDocument.V2G_Message.Body.ServicePaymentSelectionReq;

    result.header = headerCheck;

    result.selectedPaymentOption = (iso1paymentOptionType)pDat->SelectedPaymentOption;
    result.selectedServices.clear();
    iso1SelectedServiceType x;
    result.selectedServices.push_back(x);

    for (int i=0; i<pDat->SelectedServiceList.SelectedService.arrayLen; i++)
    {
        iso1SelectedServiceType selService;
        selService.ServiceID = pDat->SelectedServiceList.SelectedService.array[i].ServiceID;
        selService.ParameterSetID_isUsed = pDat->SelectedServiceList.SelectedService.array[i].ParameterSetID_isUsed;
        selService.ParameterSetID = pDat->SelectedServiceList.SelectedService.array[i].ParameterSetID;
        result.selectedServices.push_back(selService);
    }

    return true;
}

bool cExiDin70121openV2g::read_V2gPaymentServiceSelectionRes(stV2gPaymentServiceSelectionRes& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gPaymentServiceSelectionRes", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::PaymentServiceSelection ||
        !receivedExiDocument.V2G_Message.Body.ServicePaymentSelectionRes_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    dinServicePaymentSelectionResType* pDat = &receivedExiDocument.V2G_Message.Body.ServicePaymentSelectionRes;

    result.header = headerCheck;
    result.responseCode = (enV2gResponse)pDat->ResponseCode;

    return true;
}

bool cExiDin70121openV2g::read_V2gAuthorizationReq(stV2gAuthorizationReq& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gAuthorizationReq", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::Authorization ||
        !receivedExiDocument.V2G_Message.Body.ContractAuthenticationReq_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    dinContractAuthenticationReqType* pDat = &receivedExiDocument.V2G_Message.Body.ContractAuthenticationReq;

    result.header = headerCheck;

    result.genChallenge.set();
    if (pDat->GenChallenge_isUsed)
    {
        cExiByteArray genChall;
        genChall.setFromExiCharacters(pDat->GenChallenge.charactersLen, pDat->GenChallenge.characters);
        result.genChallenge = (cByteArray)genChall;
    }

    cExiByteArray id;
    if (pDat->Id_isUsed)
    {
        id.setFromExiCharacters(pDat->Id.charactersLen, pDat->Id.characters);
    }
    result.id = id;

    return true;
}

bool cExiDin70121openV2g::read_V2gAuthorizationRes(stV2gAuthorizationRes& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gAuthorizationRes", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::Authorization ||
        !receivedExiDocument.V2G_Message.Body.ContractAuthenticationRes_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    dinContractAuthenticationResType* pDat = &receivedExiDocument.V2G_Message.Body.ContractAuthenticationRes;

    result.header = headerCheck;
    result.responseCode = (enV2gResponse)pDat->ResponseCode;

    result.evseProcessing = (iso1EVSEProcessingType)pDat->EVSEProcessing;

    return true;
}

bool cExiDin70121openV2g::read_V2gChargeParameterDiscoveryReq(stV2gChargeParameterDiscoveryReq& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gChargeParameterDiscoveryReq", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::ChargeParameterDiscovery ||
        !receivedExiDocument.V2G_Message.Body.ChargeParameterDiscoveryReq_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    dinChargeParameterDiscoveryReqType* pDat = &receivedExiDocument.V2G_Message.Body.ChargeParameterDiscoveryReq;

    result.header = headerCheck;

    result.transferMode = (iso1EnergyTransferModeType)pDat->EVRequestedEnergyTransferType;
    if (pDat->AC_EVChargeParameter_isUsed)
    {
        result.energyRequest.read(&pDat->AC_EVChargeParameter.EAmount);
        result.maxVoltage.read(&pDat->AC_EVChargeParameter.EVMaxVoltage);
        result.maxCurrent.read(&pDat->AC_EVChargeParameter.EVMaxCurrent);
        result.minCurrent.read(&pDat->AC_EVChargeParameter.EVMinCurrent);
        result.departureTime = pDat->AC_EVChargeParameter.DepartureTime;
    }
    if (pDat->DC_EVChargeParameter_isUsed)
    {
        result.maxVoltage.read(&pDat->DC_EVChargeParameter.EVMaximumVoltageLimit);
        result.maxCurrent.read(&pDat->DC_EVChargeParameter.EVMaximumCurrentLimit);
        result.dcEvStatus.errorCode = (iso1DC_EVErrorCodeType)pDat->DC_EVChargeParameter.DC_EVStatus.EVErrorCode;
        result.dcEvStatus.ressSOC = pDat->DC_EVChargeParameter.DC_EVStatus.EVRESSSOC;
        result.dcEvStatus.ready = pDat->DC_EVChargeParameter.DC_EVStatus.EVReady;
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
    }

    return true;
}

bool cExiDin70121openV2g::read_V2gChargeParameterDiscoveryRes(stV2gChargeParameterDiscoveryRes& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gChargeParameterDiscoveryRes", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::ChargeParameterDiscovery ||
        !receivedExiDocument.V2G_Message.Body.ChargeParameterDiscoveryRes_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    dinChargeParameterDiscoveryResType* pDat = &receivedExiDocument.V2G_Message.Body.ChargeParameterDiscoveryRes;

    result.header = headerCheck;
    result.responseCode = (enV2gResponse)pDat->ResponseCode;

    result.evseProcessing = (iso1EVSEProcessingType)pDat->EVSEProcessing;
    //    if (pDat->EVSEChargeParameter_isUsed)
    //        result.evseChargeParameter = pDat->EVSEChargeParameter;  // no content is defined
    if (pDat->AC_EVSEChargeParameter_isUsed)
    {
        result.evseNotification = (enV2gNotification)pDat->AC_EVSEChargeParameter.AC_EVSEStatus.EVSENotification;
        result.notificationMaxDelay = pDat->AC_EVSEChargeParameter.AC_EVSEStatus.NotificationMaxDelay;
        result.rcd = pDat->AC_EVSEChargeParameter.AC_EVSEStatus.RCD;
        result.maxCurrent.read(&pDat->AC_EVSEChargeParameter.EVSEMaxCurrent);
        result.maxVoltage.read(&pDat->AC_EVSEChargeParameter.EVSEMaxVoltage);
        result.maxCurrent.read(&pDat->AC_EVSEChargeParameter.EVSEMaxCurrent);
        result.minCurrent.read(&pDat->AC_EVSEChargeParameter.EVSEMinCurrent);
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
                int16_t intMaxP = pDat->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j].PMax;
                powerSchedule.maxP.fromFloat(intMaxP, iso1unitSymbolType_W);
                if (pDat->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j].RelativeTimeInterval_isUsed)
                {
                    powerSchedule.relativeStartTime = pDat->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j].RelativeTimeInterval.start;
                    pDat->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j].RelativeTimeInterval_isUsed = true;
                    if (pDat->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j].RelativeTimeInterval.duration_isUsed)
                       powerSchedule.relativeDuration = pDat->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j].RelativeTimeInterval.duration = saSchedules.list[i].maxPSchedules.list[j].relativeDuration;
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

bool cExiDin70121openV2g::read_V2gPowerDeliveryReq(stV2gPowerDeliveryReq& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gPowerDeliveryReq", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::PowerDelivery ||
        !receivedExiDocument.V2G_Message.Body.PowerDeliveryReq_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    dinPowerDeliveryReqType* pDat = &receivedExiDocument.V2G_Message.Body.PowerDeliveryReq;

    result.header = headerCheck;

    // The relationchip is unclear
    result.chargeProgress = (iso1chargeProgressType)pDat->ReadyToChargeState;

    //result.saScheduleTupleId = pDat->SAScheduleTupleID;

    result.chargingProfile_isUsed = pDat->ChargingProfile_isUsed;
    result.evPowerDeliveryParameter_isUsed = pDat->EVPowerDeliveryParameter_isUsed;

    if (pDat->DC_EVPowerDeliveryParameter_isUsed)
    {
        result.evStatus.EVErrorCode = (iso1DC_EVErrorCodeType)pDat->DC_EVPowerDeliveryParameter.DC_EVStatus.EVErrorCode;
        result.evStatus.EVRESSSOC = pDat->DC_EVPowerDeliveryParameter.DC_EVStatus.EVRESSSOC;
        result.evStatus.EVReady = pDat->DC_EVPowerDeliveryParameter.DC_EVStatus.EVReady;
        result.chargingComplete = pDat->DC_EVPowerDeliveryParameter.ChargingComplete;
        if (pDat->DC_EVPowerDeliveryParameter.BulkChargingComplete_isUsed)
        {
            if (pDat->DC_EVPowerDeliveryParameter.BulkChargingComplete_isUsed)
                result.bulkChargingComplete = pDat->DC_EVPowerDeliveryParameter.BulkChargingComplete;
        }
    }

    return true;
}

bool cExiDin70121openV2g::read_V2gPowerDeliveryRes(stV2gPowerDeliveryRes& result, stV2gHeader& headerCheck)
{
    cDebug dbg("stV2gPowerDeliveryRes", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::PowerDelivery ||
        !receivedExiDocument.V2G_Message.Body.PowerDeliveryRes_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    dinPowerDeliveryResType* pDat = &receivedExiDocument.V2G_Message.Body.PowerDeliveryRes;

    result.header = headerCheck;
    result.responseCode = (enV2gResponse)pDat->ResponseCode;

    result.transferMode = iso1EnergyTransferModeType_AC_three_phase_core;
    if (pDat->EVSEStatus_isUsed)
    {
        //        result.evseNotification = (enV2gNotification)pDat->EVSEStatus.EVSENotification;   // no content
        //        result.notificationMaxDelay = pDat->EVSEStatus.NotificationMaxDelay;              // no content
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

bool cExiDin70121openV2g::read_V2gChargingStatusReq(stV2gChargingStatusReq& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gChargingStatusReq", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::ChargingStatus ||
        !receivedExiDocument.V2G_Message.Body.ChargingStatusReq_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    dinChargingStatusReqType* pDat = &receivedExiDocument.V2G_Message.Body.ChargingStatusReq;

    result.header = headerCheck;

    result.noContent = pDat->noContent;

    return true;
}

bool cExiDin70121openV2g::read_V2gChargingStatusRes(stV2gChargingStatusRes& result, stV2gHeader& headerCheck)
{
    cDebug dbg("stV2gChargingStatusRes", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::ChargingStatus ||
        !receivedExiDocument.V2G_Message.Body.ChargingStatusRes_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    dinChargingStatusResType* pDat = &receivedExiDocument.V2G_Message.Body.ChargingStatusRes;

    result.header = headerCheck;
    result.responseCode = (enV2gResponse)pDat->ResponseCode;

    result.seccId.set((char*)pDat->EVSEID.bytes, (unsigned int)pDat->EVSEID.bytesLen);
    result.saScheduleTupleId = pDat->SAScheduleTupleID;
    if (pDat->EVSEMaxCurrent_isUsed)
        result.maxCurrent.read(&pDat->EVSEMaxCurrent);
    result.meterInfo_isUsed = pDat->MeterInfo_isUsed;
    result.receiptRequired_isUsed = true;

    result.evseNotification = (enV2gNotification)pDat->AC_EVSEStatus.EVSENotification;
    result.notificationMaxDelay = pDat->AC_EVSEStatus.NotificationMaxDelay;
    result.rcd = pDat->AC_EVSEStatus.RCD;

    return true;
}

bool cExiDin70121openV2g::read_V2gSessionStopReq(stV2gSessionStopReq& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gSessionStopReq", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::SessionStop ||
        !receivedExiDocument.V2G_Message.Body.SessionStopReq_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    // dinSessionStopType* pDat = &receivedExiDocument.V2G_Message.Body.SessionStopReq;

    result.header = headerCheck;

    result.stopType = iso1chargingSessionType_Terminate;

    return true;
}

bool cExiDin70121openV2g::read_V2gSessionStopRes(stV2gSessionStopRes& result, stV2gHeader& headerCheck)
{
    cDebug dbg("stV2gSessionStopRes", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::SessionStop ||
        !receivedExiDocument.V2G_Message.Body.SessionStopRes_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    dinSessionStopResType* pDat = &receivedExiDocument.V2G_Message.Body.SessionStopRes;

    result.header = headerCheck;
    result.responseCode = (enV2gResponse)pDat->ResponseCode;

    return true;
}

bool cExiDin70121openV2g::read_V2gCableCheckReq(stV2gCableCheckReq& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gCableCheckReq", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::CableCheck ||
        !receivedExiDocument.V2G_Message.Body.CableCheckReq_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    dinCableCheckReqType* pDat = &receivedExiDocument.V2G_Message.Body.CableCheckReq;

    result.header = headerCheck;

    result.evErrorCode = (iso1DC_EVErrorCodeType)pDat->DC_EVStatus.EVErrorCode;
    result.evRessSoc = pDat->DC_EVStatus.EVRESSSOC;
    result.evReady = pDat->DC_EVStatus.EVReady;

    return true;
}

bool cExiDin70121openV2g::read_V2gCableCheckRes(stV2gCableCheckRes& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gCableCheckRes", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::CableCheck ||
        !receivedExiDocument.V2G_Message.Body.CableCheckRes_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    dinCableCheckResType* pDat = &receivedExiDocument.V2G_Message.Body.CableCheckRes;

    result.header = headerCheck;
    result.responseCode = (enV2gResponse)pDat->ResponseCode;

    result.evseStatusCode = (enDcEvseStatusCode)pDat->DC_EVSEStatus.EVSEStatusCode;
    result.evseNotification = (enV2gNotification)pDat->DC_EVSEStatus.EVSENotification;
    result.notificationMaxDelay = pDat->DC_EVSEStatus.NotificationMaxDelay;
    result.evseIsolationStatus = enIsolationLevel::invalid;
    if (pDat->DC_EVSEStatus.EVSEIsolationStatus_isUsed)
        result.evseIsolationStatus = (enIsolationLevel)pDat->DC_EVSEStatus.EVSEIsolationStatus;
    result.evseProcessing = (iso1EVSEProcessingType)pDat->EVSEProcessing;

    return true;
}

bool cExiDin70121openV2g::read_V2gPreChargeReq(stV2gPreChargeReq& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gPreChargeReq", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::PreCharge ||
        !receivedExiDocument.V2G_Message.Body.PreChargeReq_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    dinPreChargeReqType* pDat = &receivedExiDocument.V2G_Message.Body.PreChargeReq;

    result.header = headerCheck;

    result.evErrorCode = (iso1DC_EVErrorCodeType)pDat->DC_EVStatus.EVErrorCode;
    result.evRessSoc = pDat->DC_EVStatus.EVRESSSOC;
    result.evReady = pDat->DC_EVStatus.EVReady;
    result.evTargetVoltage.read(&pDat->EVTargetVoltage);
    result.evTargetCurrent.read(&pDat->EVTargetCurrent);

    return true;
}

bool cExiDin70121openV2g::read_V2gPreChargeRes(stV2gPreChargeRes& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gPreChargeRes", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::PreCharge ||
        !receivedExiDocument.V2G_Message.Body.PreChargeRes_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    dinPreChargeResType* pDat = &receivedExiDocument.V2G_Message.Body.PreChargeRes;

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

bool cExiDin70121openV2g::read_V2gCurrentDemandReq(stV2gCurrentDemandReq& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gCurrentDemandReq", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::CurrentDemand ||
        !receivedExiDocument.V2G_Message.Body.CurrentDemandReq_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    dinCurrentDemandReqType* pDat = &receivedExiDocument.V2G_Message.Body.CurrentDemandReq;

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

    result.evErrorCode = (iso1DC_EVErrorCodeType)pDat->DC_EVStatus.EVErrorCode;
    result.evRessSoc = pDat->DC_EVStatus.EVRESSSOC;
    result.evReady = pDat->DC_EVStatus.EVReady;

    result.evTargetCurrent.read(&pDat->EVTargetCurrent);
    result.evTargetVoltage.read(&pDat->EVTargetVoltage);

    return true;
}

bool cExiDin70121openV2g::read_V2gCurrentDemandRes(stV2gCurrentDemandRes& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gCurrentDemandRes", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::CurrentDemand ||
        !receivedExiDocument.V2G_Message.Body.CurrentDemandRes_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    dinCurrentDemandResType* pDat = &receivedExiDocument.V2G_Message.Body.CurrentDemandRes;

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

    result.saScheduleTupleId = 0;

    result.meterInfo_isUsed = false;
    result.receiptRequired_isUsed = false;

    return true;
}

bool cExiDin70121openV2g::read_V2gWeldingDetectionReq(stV2gWeldingDetectionReq& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gWeldingDetectionReq", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::WeldingDetection ||
        !receivedExiDocument.V2G_Message.Body.WeldingDetectionReq_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    dinWeldingDetectionReqType* pDat = &receivedExiDocument.V2G_Message.Body.WeldingDetectionReq;

    result.header = headerCheck;

    result.evErrorCode = (iso1DC_EVErrorCodeType)pDat->DC_EVStatus.EVErrorCode;
    result.evRessSoc = pDat->DC_EVStatus.EVRESSSOC;
    result.evReady = pDat->DC_EVStatus.EVReady;

    return true;
}

bool cExiDin70121openV2g::read_V2gWeldingDetectionRes(stV2gWeldingDetectionRes& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gWeldingDetectionRes", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::WeldingDetection ||
        !receivedExiDocument.V2G_Message.Body.WeldingDetectionRes_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    dinWeldingDetectionResType* pDat = &receivedExiDocument.V2G_Message.Body.WeldingDetectionRes;

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

bool cExiDin70121openV2g::read_V2gServiceDetailReq(stV2gServiceDetailReq& result, stV2gHeader& headerCheck)
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

bool cExiDin70121openV2g::read_V2gServiceDetailRes(stV2gServiceDetailRes& result, stV2gHeader& headerCheck)
{
    cDebug dbg("read_V2gServiceDetailRes", &this->dbg);

    if (headerCheck.messageType != enV2gMsg::ServiceDiscovery ||
        !receivedExiDocument.V2G_Message.Body.ServiceDiscoveryRes_isUsed)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Received EXI document %s does not fit to message evaluation.", enV2gMsg_toString(headerCheck.messageType).c_str());
        return false;
    }

    dinServiceDiscoveryResType* pDat = &receivedExiDocument.V2G_Message.Body.ServiceDiscoveryRes;

    result.header = headerCheck;
    result.responseCode = (enV2gResponse)pDat->ResponseCode;

    return true;
}

int cExiDin70121openV2g::write_ExiDocument(struct dinEXIDocument& exi, stStreamControl& stream)
{
    cDebug dbg("write_ExiDocument(dinEXIDocument)", &this->dbg);

    size_t pos = V2GTP_HEADER_LENGTH;
    bitstream_t oStream = outStream(stream, pos);

    int errn = 0;
    if( (errn = encode_dinExiDocument(&oStream, &exi)) == 0)
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

int cExiDin70121openV2g::write_V2gSessionSetupReq(stV2gSessionSetupReq& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gSessionSetupReq", &this->dbg);

    dinEXIDocument exiOut;
    init_dinEXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_dinMessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_dinBodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.SessionSetupReq_isUsed = 1u;

    init_dinSessionSetupReqType(&exiOut.V2G_Message.Body.SessionSetupReq);

    cExiByteArray evcc_Id(data.evccId);
    evcc_Id.asChar((char*)exiOut.V2G_Message.Body.SessionSetupReq.EVCCID.bytes, 6);
    exiOut.V2G_Message.Body.SessionSetupReq.EVCCID.bytesLen = evcc_Id.size();

    return write_ExiDocument(exiOut, stream);
}

int cExiDin70121openV2g::write_V2gSessionSetupRes(stV2gSessionSetupRes& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gSessionSetupRes", &this->dbg);

    dinEXIDocument exiOut;
    init_dinEXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_dinMessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_dinBodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.SessionSetupRes_isUsed = 1u;

    init_dinSessionSetupResType(&exiOut.V2G_Message.Body.SessionSetupRes);
    dinSessionSetupResType* pDat = &exiOut.V2G_Message.Body.SessionSetupRes;

    pDat->ResponseCode = (dinresponseCodeType)data.responseCode;

    cExiByteArray secc_Id(data.seccId);
    secc_Id.asChar((char*)pDat->EVSEID.bytes, 8);
    pDat->EVSEID.bytesLen = secc_Id.size();
    //if (data.seccTimeStamp != 0x80000000)
    //{
    pDat->DateTimeNow_isUsed = 1u;
    pDat->DateTimeNow = 123456789;
    //}
    //else
    //{
    //    exiOut.V2G_Message.Body.SessionSetupRes.EVSETimeStamp_isUsed = 0u;
    //    //exiOut.V2G_Message.Body.SessionSetupRes.EVSETimeStamp = 123456789;
    //}

    return write_ExiDocument(exiOut, stream);
}

int cExiDin70121openV2g::write_V2gServiceDiscoveryReq(stV2gServiceDiscoveryReq& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gServiceDiscoveryReq", &this->dbg);

    dinEXIDocument exiOut;
    init_dinEXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_dinMessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_dinBodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.ServiceDiscoveryReq_isUsed = 1u;

    init_dinServiceDiscoveryReqType(&exiOut.V2G_Message.Body.ServiceDiscoveryReq);
    dinServiceDiscoveryReqType* pDat = &exiOut.V2G_Message.Body.ServiceDiscoveryReq;

    pDat->ServiceCategory_isUsed = true;
    pDat->ServiceCategory = (dinserviceCategoryType)data.serviceCategory;
    pDat->ServiceScope_isUsed = false;

    return write_ExiDocument(exiOut, stream);
}

int cExiDin70121openV2g::write_V2gServiceDiscoveryRes(stV2gServiceDiscoveryRes& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gServiceDiscoveryRes", &this->dbg);

    unsigned int i;

    dinEXIDocument exiOut;
    init_dinEXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_dinMessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_dinBodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.ServiceDiscoveryRes_isUsed = 1u;

    init_dinServiceDiscoveryResType(&exiOut.V2G_Message.Body.ServiceDiscoveryRes);
    dinServiceDiscoveryResType* pDat = &exiOut.V2G_Message.Body.ServiceDiscoveryRes;

    init_dinPaymentOptionsType(&pDat->PaymentOptions);
    pDat->PaymentOptions.PaymentOption.arrayLen = (uint16_t)data.paymentOptions.size();
    for (i=0; i<data.paymentOptions.size(); i++)
        pDat->PaymentOptions.PaymentOption.array[i] = (dinpaymentOptionType)data.paymentOptions[i];

    init_dinServiceChargeType(&pDat->ChargeService);
    pDat->ChargeService.FreeService = data.chargeServiceFree;
    pDat->ChargeService.ServiceTag.ServiceID = data.chargeServiceId;
    pDat->ChargeService.ServiceTag.ServiceCategory = (dinserviceCategoryType)data.chargeServiceCategory;
    if (data.energyTransferModes.size())
        pDat->ChargeService.EnergyTransferType = (dinEVSESupportedEnergyTransferType)data.energyTransferModes[0];

    pDat->ResponseCode = dinresponseCodeType_OK;

    return write_ExiDocument(exiOut, stream);
}

int cExiDin70121openV2g::write_V2gPaymentServiceSelectionReq(stV2gPaymentServiceSelectionReq& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gPaymentServiceSelectionReq", &this->dbg);

    dinEXIDocument exiOut;
    init_dinEXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_dinMessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_dinBodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.ServicePaymentSelectionReq_isUsed = 1u;

    init_dinServicePaymentSelectionReqType(&exiOut.V2G_Message.Body.ServicePaymentSelectionReq);
    dinServicePaymentSelectionReqType* pDat = &exiOut.V2G_Message.Body.ServicePaymentSelectionReq;

    pDat->SelectedPaymentOption = (dinpaymentOptionType)data.selectedPaymentOption;

    init_dinSelectedServiceListType(&pDat->SelectedServiceList);
    pDat->SelectedServiceList.SelectedService.arrayLen = (uint16_t)data.selectedServices.size();
    for (unsigned int i=0; i<data.selectedServices.size(); i++)
    {
        init_dinSelectedServiceType(&pDat->SelectedServiceList.SelectedService.array[i]);
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

int cExiDin70121openV2g::write_V2gPaymentServiceSelectionRes(stV2gPaymentServiceSelectionRes& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gPaymentServiceSelectionRes", &this->dbg);

    dinEXIDocument exiOut;
    init_dinEXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_dinMessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_dinBodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.ServicePaymentSelectionRes_isUsed = 1u;

    init_dinServicePaymentSelectionResType(&exiOut.V2G_Message.Body.ServicePaymentSelectionRes);
    dinServicePaymentSelectionResType* pDat = &exiOut.V2G_Message.Body.ServicePaymentSelectionRes;

    pDat->ResponseCode = (dinresponseCodeType)data.responseCode;

    return write_ExiDocument(exiOut, stream);
}

int cExiDin70121openV2g::write_V2gAuthorizationReq(stV2gAuthorizationReq& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gAuthorizationReq", &this->dbg);

    dinEXIDocument exiOut;
    init_dinEXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_dinMessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_dinBodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.ContractAuthenticationReq_isUsed = 1u;

    init_dinContractAuthenticationReqType(&exiOut.V2G_Message.Body.ContractAuthenticationReq);
    dinContractAuthenticationReqType* pDat = &exiOut.V2G_Message.Body.ContractAuthenticationReq;

    if (data.genChallenge.size())
    {
        cExiByteArray gen = data.genChallenge;
        pDat->GenChallenge_isUsed = true;
        pDat->GenChallenge.charactersLen = gen.size();
        gen.asExiCharacters(16, pDat->GenChallenge.charactersLen, pDat->GenChallenge.characters);
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

int cExiDin70121openV2g::write_V2gAuthorizationRes(stV2gAuthorizationRes& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gAuthorizationRes", &this->dbg);

    dinEXIDocument exiOut;
    init_dinEXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_dinMessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_dinBodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.ContractAuthenticationRes_isUsed = 1u;

    init_dinContractAuthenticationResType(&exiOut.V2G_Message.Body.ContractAuthenticationRes);
    dinContractAuthenticationResType* pDat = &exiOut.V2G_Message.Body.ContractAuthenticationRes;

    pDat->ResponseCode = (dinresponseCodeType)data.responseCode;

    pDat->EVSEProcessing = (dinEVSEProcessingType)data.evseProcessing;

    return write_ExiDocument(exiOut, stream);
}

int cExiDin70121openV2g::write_V2gChargeParameterDiscoveryReq(stV2gChargeParameterDiscoveryReq& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gChargeParameterDiscoveryReq", &this->dbg);

    dinEXIDocument exiOut;
    init_dinEXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_dinMessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_dinBodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.ChargeParameterDiscoveryReq_isUsed = 1u;

    init_dinChargeParameterDiscoveryReqType(&exiOut.V2G_Message.Body.ChargeParameterDiscoveryReq);
    dinChargeParameterDiscoveryReqType* pDat = &exiOut.V2G_Message.Body.ChargeParameterDiscoveryReq;

    pDat->AC_EVChargeParameter_isUsed = false;
    pDat->DC_EVChargeParameter_isUsed = false;
    pDat->EVRequestedEnergyTransferType = (dinEVRequestedEnergyTransferType)data.transferMode;
    switch (data.transferMode)
    {
    case iso1EnergyTransferModeType_AC_single_phase_core:
    case iso1EnergyTransferModeType_AC_three_phase_core:
    {
        pDat->AC_EVChargeParameter_isUsed = true;
        init_dinAC_EVChargeParameterType(&pDat->AC_EVChargeParameter);
        data.maxVoltage.write(&pDat->AC_EVChargeParameter.EVMaxVoltage);
        data.maxCurrent.write(&pDat->AC_EVChargeParameter.EVMaxCurrent);
        data.minCurrent.write(&pDat->AC_EVChargeParameter.EVMinCurrent);
        data.energyRequest.write(&pDat->AC_EVChargeParameter.EAmount);
        if (data.departureTime)
        {
            //pDat->AC_EVChargeParameter.DepartureTime_isUsed = true;
            pDat->AC_EVChargeParameter.DepartureTime = data.departureTime;
        }
//        else
//            pDat->AC_EVChargeParameter.DepartureTime_isUsed = false;
        } break;
    case iso1EnergyTransferModeType_DC_core:
    case iso1EnergyTransferModeType_DC_combo_core:
    case iso1EnergyTransferModeType_DC_unique:
    case iso1EnergyTransferModeType_DC_extended:
    {
        pDat->DC_EVChargeParameter_isUsed = true;
        init_dinDC_EVChargeParameterType(&pDat->DC_EVChargeParameter);
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
        init_dinDC_EVStatusType(&pDat->DC_EVChargeParameter.DC_EVStatus);
        pDat->DC_EVChargeParameter.DC_EVStatus.EVErrorCode = (dinDC_EVErrorCodeType)data.dcEvStatus.errorCode;
        pDat->DC_EVChargeParameter.DC_EVStatus.EVRESSSOC = data.dcEvStatus.ressSOC;
        pDat->DC_EVChargeParameter.DC_EVStatus.EVReady = data.dcEvStatus.ready;
        // if (data.departureTime)
        // {
        //     pDat->DC_EVChargeParameter.DepartureTime_isUsed = true;
        //     pDat->DC_EVChargeParameter.DepartureTime = data.departureTime;
        // }
        // else
        //     pDat->DC_EVChargeParameter.DepartureTime_isUsed = false;
    } break;
    }

    if (data.departureTime)
    {
        pDat->EVChargeParameter_isUsed = true;
        init_dinEVChargeParameterType(&pDat->EVChargeParameter);
        // pDat->EVChargeParameter.DepartureTime_isUsed = true;
        // pDat->EVChargeParameter.DepartureTime = data.departureTime;
    }
    else
        pDat->EVChargeParameter_isUsed = false;

    //    if (data.maxEntriesSAScheduleTuple)
    //    {
    //        pDat->MaxEntriesSAScheduleTuple_isUsed = true;
    //        pDat->MaxEntriesSAScheduleTuple = data.maxEntriesSAScheduleTuple;
    //    }
    //    else
    //        pDat->MaxEntriesSAScheduleTuple_isUsed = false;

    return write_ExiDocument(exiOut, stream);
}

int cExiDin70121openV2g::write_V2gChargeParameterDiscoveryRes(stV2gChargeParameterDiscoveryRes& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gChargeParameterDiscoveryRes", &this->dbg);

    dinEXIDocument exiOut;
    init_dinEXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_dinMessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_dinBodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.ChargeParameterDiscoveryRes_isUsed = 1u;

    init_dinChargeParameterDiscoveryResType(&exiOut.V2G_Message.Body.ChargeParameterDiscoveryRes);
    dinChargeParameterDiscoveryResType* pDat = &exiOut.V2G_Message.Body.ChargeParameterDiscoveryRes;

    pDat->ResponseCode = (dinresponseCodeType)data.responseCode;

    pDat->EVSEProcessing = (dinEVSEProcessingType)data.evseProcessing;
    pDat->AC_EVSEChargeParameter_isUsed = false;
    pDat->DC_EVSEChargeParameter_isUsed = false;
    switch (data.transferMode)
    {
    case iso1EnergyTransferModeType_AC_three_phase_core:
    case iso1EnergyTransferModeType_AC_single_phase_core:
    {
        pDat->AC_EVSEChargeParameter_isUsed = true;
        init_dinAC_EVSEChargeParameterType(&pDat->AC_EVSEChargeParameter);
        init_dinAC_EVSEStatusType(&pDat->AC_EVSEChargeParameter.AC_EVSEStatus);
        pDat->AC_EVSEChargeParameter.AC_EVSEStatus.EVSENotification = (dinEVSENotificationType)data.evseNotification;
        pDat->AC_EVSEChargeParameter.AC_EVSEStatus.NotificationMaxDelay = data.notificationMaxDelay;
        pDat->AC_EVSEChargeParameter.AC_EVSEStatus.RCD = data.rcd;
        init_dinPhysicalValueType(&pDat->AC_EVSEChargeParameter.EVSEMaxCurrent);
        data.maxCurrent.write(&pDat->AC_EVSEChargeParameter.EVSEMaxCurrent);
        init_dinPhysicalValueType(&pDat->AC_EVSEChargeParameter.EVSEMaxVoltage);
        data.nominalVoltage.write(&pDat->AC_EVSEChargeParameter.EVSEMaxVoltage);
        init_dinPhysicalValueType(&pDat->AC_EVSEChargeParameter.EVSEMinCurrent);
        data.minCurrent.write(&pDat->AC_EVSEChargeParameter.EVSEMinCurrent);
    } break;
    case iso1EnergyTransferModeType_DC_core:
    case iso1EnergyTransferModeType_DC_combo_core:
    case iso1EnergyTransferModeType_DC_unique:
    case iso1EnergyTransferModeType_DC_extended:
    {
        pDat->DC_EVSEChargeParameter_isUsed = true;
        init_dinDC_EVSEChargeParameterType(&pDat->DC_EVSEChargeParameter);
        init_dinDC_EVSEStatusType(&pDat->DC_EVSEChargeParameter.DC_EVSEStatus);
        pDat->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEStatusCode = (dinDC_EVSEStatusCodeType)data.evseStatusCode;
        pDat->DC_EVSEChargeParameter.DC_EVSEStatus.EVSENotification = (dinEVSENotificationType)data.evseNotification;
        pDat->DC_EVSEChargeParameter.DC_EVSEStatus.NotificationMaxDelay = data.notificationMaxDelay;
        if (data.evseIsolationStatus != enIsolationLevel::invalid)
        {
            pDat->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEIsolationStatus_isUsed = true;
            pDat->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEIsolationStatus = (dinisolationLevelType)data.evseIsolationStatus;
        }
        else
            pDat->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEIsolationStatus_isUsed = false;
        init_dinPhysicalValueType(&pDat->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit);
        data.maxCurrent.write(&pDat->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit);
        init_dinPhysicalValueType(&pDat->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit);
        data.maxVoltage.write(&pDat->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit);
        init_dinPhysicalValueType(&pDat->DC_EVSEChargeParameter.EVSEMinimumCurrentLimit);
        data.minCurrent.write(&pDat->DC_EVSEChargeParameter.EVSEMinimumCurrentLimit);
        init_dinPhysicalValueType(&pDat->DC_EVSEChargeParameter.EVSEMinimumVoltageLimit);
        data.minVoltage.write(&pDat->DC_EVSEChargeParameter.EVSEMinimumVoltageLimit);
        init_dinPhysicalValueType(&pDat->DC_EVSEChargeParameter.EVSEPeakCurrentRipple);
        data.maxPeakCurrentRipple.write(&pDat->DC_EVSEChargeParameter.EVSEPeakCurrentRipple);
        pDat->DC_EVSEChargeParameter.EVSEMaximumPowerLimit_isUsed = true;
        init_dinPhysicalValueType(&pDat->DC_EVSEChargeParameter.EVSEMaximumPowerLimit);
        data.maxPower.write(&pDat->DC_EVSEChargeParameter.EVSEMaximumPowerLimit);
    }
    }

    stPmaxSchedule powerSchedule;
    stPmaxSchedules powerSchedules;
    powerSchedules.list.push_back(powerSchedule);

    stSaScheduleTuple scheduleTuple;
    scheduleTuple.id = 1;
    scheduleTuple.maxPSchedules = powerSchedules;
    stSaSchedules saSchedules;
    saSchedules.list.push_back(scheduleTuple);

    int num = (int)saSchedules.list.size();
    pDat->SAScheduleList_isUsed = true;
    init_dinSAScheduleListType(&pDat->SAScheduleList);
    if (num > 3) num = 3;
    pDat->SAScheduleList.SAScheduleTuple.arrayLen = num;
    for (int i=0; i<num; i++)
    {
        init_dinSAScheduleTupleType(&pDat->SAScheduleList.SAScheduleTuple.array[i]);
        pDat->SAScheduleList.SAScheduleTuple.array[i].SAScheduleTupleID = saSchedules.list[i].id;
        pDat->SAScheduleList.SAScheduleTuple.array[i].SalesTariff_isUsed = false;
        int len = (int)saSchedules.list[i].maxPSchedules.list.size();
        init_dinPMaxScheduleType(&pDat->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule);
        if (len > 5) len = 5;
        pDat->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.arrayLen = len;
        for (int j=0; j<len; j++)
        {
            init_dinPMaxScheduleEntryType(&pDat->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j]);
            //init_dinPhysicalValueType(&pDat->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j].PMax);
            //saSchedules.list[i].maxPSchedules.list[j].maxP.write(&pDat->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j].PMax);
            pDat->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j].PMax = (int16_t)saSchedules.list[i].maxPSchedules.list[j].maxP.toFloat();
            //if (saSchedules.list[i].maxPSchedules.list[j].relativeStartTime) // First, as zero is a valid number, not using 'relative time interval is not an option. Second, Either relative or absolute time interval is mandatory but absolute time interval is not defined. Thus, relative time interval is practically mandatory so far by ISO 15118.
            {
                pDat->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j].RelativeTimeInterval_isUsed = true;
                init_dinRelativeTimeIntervalType(&pDat->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j].RelativeTimeInterval);
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

int cExiDin70121openV2g::write_V2gPowerDeliveryReq(stV2gPowerDeliveryReq& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gPowerDeliveryReq", &this->dbg);

    dinEXIDocument exiOut;
    init_dinEXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_dinMessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_dinBodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.PowerDeliveryReq_isUsed = 1u;

    init_dinPowerDeliveryReqType(&exiOut.V2G_Message.Body.PowerDeliveryReq);
    dinPowerDeliveryReqType* pDat = &exiOut.V2G_Message.Body.PowerDeliveryReq;

    pDat->ReadyToChargeState = data.chargeProgress;
    //pDat->SAScheduleTupleID = data.saScheduleTupleId;
    pDat->ChargingProfile_isUsed = data.chargingProfile_isUsed;
    pDat->EVPowerDeliveryParameter_isUsed = data.evPowerDeliveryParameter_isUsed;

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
        pDat->DC_EVPowerDeliveryParameter.DC_EVStatus.EVErrorCode = (dinDC_EVErrorCodeType)data.evStatus.EVErrorCode;
        pDat->DC_EVPowerDeliveryParameter.DC_EVStatus.EVRESSSOC = data.evStatus.EVRESSSOC;
        pDat->DC_EVPowerDeliveryParameter.DC_EVStatus.EVReady = data.evStatus.EVReady;
        pDat->DC_EVPowerDeliveryParameter.DC_EVStatus.EVCabinConditioning_isUsed = 0;
        pDat->DC_EVPowerDeliveryParameter.DC_EVStatus.EVRESSConditioning_isUsed = 0;
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

int cExiDin70121openV2g::write_V2gPowerDeliveryRes(stV2gPowerDeliveryRes& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gPowerDeliveryRes", &this->dbg);

    dinEXIDocument exiOut;
    init_dinEXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_dinMessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_dinBodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.PowerDeliveryRes_isUsed = 1u;

    init_dinPowerDeliveryResType(&exiOut.V2G_Message.Body.PowerDeliveryRes);
    dinPowerDeliveryResType* pDat = &exiOut.V2G_Message.Body.PowerDeliveryRes;

    pDat->ResponseCode = (dinresponseCodeType)data.responseCode;

    pDat->EVSEStatus_isUsed = false;
    pDat->AC_EVSEStatus_isUsed = false;
    pDat->AC_EVSEStatus_isUsed = false;
    switch (data.transferMode)
    {
    case iso1EnergyTransferModeType_AC_three_phase_core:
    case iso1EnergyTransferModeType_AC_single_phase_core:
    {
        pDat->AC_EVSEStatus_isUsed = true;
        pDat->AC_EVSEStatus.EVSENotification = (dinEVSENotificationType)data.evseNotification;
        pDat->AC_EVSEStatus.NotificationMaxDelay = data.notificationMaxDelay;
        pDat->AC_EVSEStatus.RCD = data.rcd;
    } break;
    case iso1EnergyTransferModeType_DC_core:
    case iso1EnergyTransferModeType_DC_combo_core:
    case iso1EnergyTransferModeType_DC_unique:
    case iso1EnergyTransferModeType_DC_extended:
    {
        pDat->DC_EVSEStatus_isUsed = true;
        pDat->DC_EVSEStatus.EVSENotification = (dinEVSENotificationType)data.evseNotification;
        pDat->DC_EVSEStatus.NotificationMaxDelay = data.notificationMaxDelay;
        pDat->DC_EVSEStatus.EVSEStatusCode = (dinDC_EVSEStatusCodeType)data.evseStatusCode;
        if (data.evseIsolationStatus != enIsolationLevel::invalid)
        {
            pDat->DC_EVSEStatus.EVSEIsolationStatus_isUsed = true;
            pDat->DC_EVSEStatus.EVSEIsolationStatus = (dinisolationLevelType)data.evseIsolationStatus;
        }
        else
            pDat->DC_EVSEStatus.EVSEIsolationStatus_isUsed = false;
    }
    }

    return write_ExiDocument(exiOut, stream);
}

int cExiDin70121openV2g::write_V2gChargingStatusReq(stV2gChargingStatusReq& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gChargingStatusReq", &this->dbg);

    dinEXIDocument exiOut;
    init_dinEXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_dinMessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_dinBodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.ChargingStatusReq_isUsed = 1u;

    init_dinChargingStatusReqType(&exiOut.V2G_Message.Body.ChargingStatusReq);

    return write_ExiDocument(exiOut, stream);
}

int cExiDin70121openV2g::write_V2gChargingStatusRes(stV2gChargingStatusRes& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gChargingStatusRes", &this->dbg);

    dinEXIDocument exiOut;
    init_dinEXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_dinMessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_dinBodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.ChargingStatusRes_isUsed = 1u;

    init_dinChargingStatusResType(&exiOut.V2G_Message.Body.ChargingStatusRes);
    dinChargingStatusResType* pDat = &exiOut.V2G_Message.Body.ChargingStatusRes;

    pDat->ResponseCode = (dinresponseCodeType)data.responseCode;

    data.seccId.asChar((char*)pDat->EVSEID.bytes, pDat->EVSEID.bytesLen);
    pDat->SAScheduleTupleID = data.saScheduleTupleId;
    pDat->EVSEMaxCurrent_isUsed = false;
    if (data.maxCurrent.value)
    {
        pDat->EVSEMaxCurrent_isUsed = true;
        init_dinPhysicalValueType(&pDat->EVSEMaxCurrent);
        data.maxCurrent.write(&pDat->EVSEMaxCurrent);
    }

    pDat->AC_EVSEStatus.EVSENotification = (dinEVSENotificationType)data.evseNotification;
    pDat->AC_EVSEStatus.NotificationMaxDelay = data.notificationMaxDelay;
    pDat->AC_EVSEStatus.RCD = data.rcd;

    pDat->MeterInfo_isUsed = false;
    //pDat->ReceiptRequired_isUsed = false;

    return write_ExiDocument(exiOut, stream);
}

int cExiDin70121openV2g::write_V2gSessionStopReq(stV2gSessionStopReq& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gSessionStopReq", &this->dbg);

    dinEXIDocument exiOut;
    init_dinEXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_dinMessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_dinBodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.SessionStopReq_isUsed = 1u;

    init_dinSessionStopType(&exiOut.V2G_Message.Body.SessionStopReq);
    //dinSessionStopType* pDat = &exiOut.V2G_Message.Body.SessionStopReq;

    //pDat->ChargingSession = data.stopType;

    return write_ExiDocument(exiOut, stream);
}

int cExiDin70121openV2g::write_V2gSessionStopRes(stV2gSessionStopRes& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gSessionStopRes", &this->dbg);

    dinEXIDocument exiOut;
    init_dinEXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_dinMessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_dinBodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.SessionStopRes_isUsed = 1u;

    init_dinSessionStopResType(&exiOut.V2G_Message.Body.SessionStopRes);
    dinSessionStopResType* pDat = &exiOut.V2G_Message.Body.SessionStopRes;

    pDat->ResponseCode = (dinresponseCodeType)data.responseCode;

    return write_ExiDocument(exiOut, stream);
}

int cExiDin70121openV2g::write_V2gCableCheckReq(stV2gCableCheckReq& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gCableCheckReq", &this->dbg);

    dinEXIDocument exiOut;
    init_dinEXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_dinMessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_dinBodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.CableCheckReq_isUsed = 1u;

    init_dinCableCheckReqType(&exiOut.V2G_Message.Body.CableCheckReq);
    dinCableCheckReqType* pDat = &exiOut.V2G_Message.Body.CableCheckReq;

    pDat->DC_EVStatus.EVErrorCode = (dinDC_EVErrorCodeType)data.evErrorCode;
    pDat->DC_EVStatus.EVRESSSOC = data.evRessSoc;
    pDat->DC_EVStatus.EVReady = data.evReady;

    return write_ExiDocument(exiOut, stream);
}

int cExiDin70121openV2g::write_V2gCableCheckRes(stV2gCableCheckRes& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gCableCheckRes", &this->dbg);

    dinEXIDocument exiOut;
    init_dinEXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_dinMessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_dinBodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.CableCheckRes_isUsed = 1u;

    init_dinCableCheckResType(&exiOut.V2G_Message.Body.CableCheckRes);
    dinCableCheckResType* pDat = &exiOut.V2G_Message.Body.CableCheckRes;

    pDat->ResponseCode = (dinresponseCodeType)data.responseCode;

    pDat->EVSEProcessing = (dinEVSEProcessingType)data.evseProcessing;
    init_dinDC_EVSEStatusType(&pDat->DC_EVSEStatus);
    pDat->DC_EVSEStatus.EVSEStatusCode = (dinDC_EVSEStatusCodeType)data.evseStatusCode;
    pDat->DC_EVSEStatus.EVSENotification = (dinEVSENotificationType)data.evseNotification;
    pDat->DC_EVSEStatus.NotificationMaxDelay = data.notificationMaxDelay;
    pDat->DC_EVSEStatus.EVSEIsolationStatus_isUsed = false;
    if (data.evseIsolationStatus!=enIsolationLevel::invalid)
    {
        pDat->DC_EVSEStatus.EVSEIsolationStatus_isUsed = true;
        pDat->DC_EVSEStatus.EVSEIsolationStatus = (dinisolationLevelType)data.evseIsolationStatus;
    }

    return write_ExiDocument(exiOut, stream);
}

int cExiDin70121openV2g::write_V2gPreChargeReq(stV2gPreChargeReq& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gPreChargeReq", &this->dbg);

    dinEXIDocument exiOut;
    init_dinEXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_dinMessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_dinBodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.PreChargeReq_isUsed = 1u;

    init_dinPreChargeReqType(&exiOut.V2G_Message.Body.PreChargeReq);
    dinPreChargeReqType* pDat = &exiOut.V2G_Message.Body.PreChargeReq;

    init_dinDC_EVStatusType(&pDat->DC_EVStatus);
    pDat->DC_EVStatus.EVErrorCode = (dinDC_EVErrorCodeType)data.evErrorCode;
    pDat->DC_EVStatus.EVRESSSOC = data.evRessSoc;
    pDat->DC_EVStatus.EVReady = data.evReady;
    init_dinPhysicalValueType(&pDat->EVTargetVoltage);
    data.evTargetVoltage.write(&pDat->EVTargetVoltage);
    init_dinPhysicalValueType(&pDat->EVTargetCurrent);
    data.evTargetCurrent.write(&pDat->EVTargetCurrent);

    return write_ExiDocument(exiOut, stream);
}

int cExiDin70121openV2g::write_V2gPreChargeRes(stV2gPreChargeRes& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gPreChargeRes", &this->dbg);

    dinEXIDocument exiOut;
    init_dinEXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_dinMessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_dinBodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.PreChargeRes_isUsed = 1u;

    init_dinPreChargeResType(&exiOut.V2G_Message.Body.PreChargeRes);
    dinPreChargeResType* pDat = &exiOut.V2G_Message.Body.PreChargeRes;

    pDat->ResponseCode = (dinresponseCodeType)data.responseCode;

    init_dinDC_EVSEStatusType(&pDat->DC_EVSEStatus);
    pDat->DC_EVSEStatus.EVSEStatusCode = (dinDC_EVSEStatusCodeType)data.evseStatusCode;
    pDat->DC_EVSEStatus.EVSENotification = (dinEVSENotificationType)data.evseNotification;
    pDat->DC_EVSEStatus.NotificationMaxDelay = data.notificationMaxDelay;
    pDat->DC_EVSEStatus.EVSEIsolationStatus_isUsed = false;
    if (data.evseIsolationStatus != enIsolationLevel::invalid)
    {
        pDat->DC_EVSEStatus.EVSEIsolationStatus_isUsed = true;
        pDat->DC_EVSEStatus.EVSEIsolationStatus = (dinisolationLevelType)data.evseIsolationStatus;
    }
    init_dinPhysicalValueType(&pDat->EVSEPresentVoltage);
    data.evsePresentVoltage.write(&pDat->EVSEPresentVoltage);

    return write_ExiDocument(exiOut, stream);
}

int cExiDin70121openV2g::write_V2gCurrentDemandReq(stV2gCurrentDemandReq& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gCurrentDemandReq", &this->dbg);

    dinEXIDocument exiOut;
    init_dinEXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_dinMessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_dinBodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.CurrentDemandReq_isUsed = 1u;

    init_dinCurrentDemandReqType(&exiOut.V2G_Message.Body.CurrentDemandReq);
    dinCurrentDemandReqType* pDat = &exiOut.V2G_Message.Body.CurrentDemandReq;

    init_dinDC_EVStatusType(&pDat->DC_EVStatus);
    pDat->DC_EVStatus.EVErrorCode = (dinDC_EVErrorCodeType)data.evErrorCode;
    pDat->DC_EVStatus.EVRESSSOC = data.evRessSoc;
    pDat->DC_EVStatus.EVReady = data.evReady;

    init_dinPhysicalValueType(&pDat->EVTargetVoltage);
    data.evTargetVoltage.write(&pDat->EVTargetVoltage);
    init_dinPhysicalValueType(&pDat->EVTargetCurrent);
    data.evTargetCurrent.write(&pDat->EVTargetCurrent);

    pDat->EVMaximumCurrentLimit_isUsed = false;
    if (data.evCurrentLimit.value)
    {
        pDat->EVMaximumCurrentLimit_isUsed = true;
        init_dinPhysicalValueType(&pDat->EVMaximumCurrentLimit);
        data.evCurrentLimit.write(&pDat->EVMaximumCurrentLimit);
    }
    pDat->EVMaximumVoltageLimit_isUsed = false;
    if (data.evVoltageLimit.value)
    {
        pDat->EVMaximumVoltageLimit_isUsed = true;
        init_dinPhysicalValueType(&pDat->EVMaximumVoltageLimit);
        data.evVoltageLimit.write(&pDat->EVMaximumVoltageLimit);
    }
    pDat->EVMaximumPowerLimit_isUsed = false;
    if (data.evCurrentLimit.value)
    {
        pDat->EVMaximumPowerLimit_isUsed = true;
        init_dinPhysicalValueType(&pDat->EVMaximumPowerLimit);
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
        init_dinPhysicalValueType(&pDat->RemainingTimeToBulkSoC);
        data.remainingTimeToBulkSoC.write(&pDat->RemainingTimeToBulkSoC);
    }
    pDat->RemainingTimeToFullSoC_isUsed = false;
    if (data.remainingTimeToFullSoC.value)
    {
        pDat->RemainingTimeToFullSoC_isUsed = true;
        init_dinPhysicalValueType(&pDat->RemainingTimeToFullSoC);
        data.remainingTimeToFullSoC.write(&pDat->RemainingTimeToFullSoC);
    }

    return write_ExiDocument(exiOut, stream);
}

int cExiDin70121openV2g::write_V2gCurrentDemandRes(stV2gCurrentDemandRes& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gCurrentDemandRes", &this->dbg);

    dinEXIDocument exiOut;
    init_dinEXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_dinMessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_dinBodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.CurrentDemandRes_isUsed = 1u;

    init_dinCurrentDemandResType(&exiOut.V2G_Message.Body.CurrentDemandRes);
    dinCurrentDemandResType* pDat = &exiOut.V2G_Message.Body.CurrentDemandRes;

    pDat->ResponseCode = (dinresponseCodeType)data.responseCode;

    init_dinDC_EVSEStatusType(&pDat->DC_EVSEStatus);
    pDat->DC_EVSEStatus.EVSEStatusCode = (dinDC_EVSEStatusCodeType)data.evseStatusCode;
    pDat->DC_EVSEStatus.EVSENotification = (dinEVSENotificationType)data.evseNotification;
    pDat->DC_EVSEStatus.NotificationMaxDelay = data.notificationMaxDelay;
    pDat->DC_EVSEStatus.EVSEIsolationStatus_isUsed = false;
    if (data.evseIsolationStatus != enIsolationLevel::invalid)
    {
        pDat->DC_EVSEStatus.EVSEIsolationStatus_isUsed = true;
        pDat->DC_EVSEStatus.EVSEIsolationStatus = (dinisolationLevelType)data.evseIsolationStatus;
    }

    init_dinPhysicalValueType(&pDat->EVSEPresentCurrent);
    data.evsePresentCurrent.write(&pDat->EVSEPresentCurrent);
    init_dinPhysicalValueType(&pDat->EVSEPresentVoltage);
    data.evsePresentVoltage.write(&pDat->EVSEPresentVoltage);

    pDat->EVSEMaximumCurrentLimit_isUsed = false;
    if (data.evseCurrentLimit.value)
    {
        pDat->EVSEMaximumCurrentLimit_isUsed = true;
        init_dinPhysicalValueType(&pDat->EVSEMaximumCurrentLimit);
        data.evseCurrentLimit.write(&pDat->EVSEMaximumCurrentLimit);
    }
    pDat->EVSEMaximumVoltageLimit_isUsed = false;
    if (data.evseVoltageLimit.value)
    {
        pDat->EVSEMaximumVoltageLimit_isUsed = true;
        init_dinPhysicalValueType(&pDat->EVSEMaximumVoltageLimit);
        data.evseVoltageLimit.write(&pDat->EVSEMaximumVoltageLimit);
    }
    pDat->EVSEMaximumPowerLimit_isUsed = false;
    if (data.evseCurrentLimit.value)
    {
        pDat->EVSEMaximumPowerLimit_isUsed = true;
        init_dinPhysicalValueType(&pDat->EVSEMaximumPowerLimit);
        data.evsePowerLimit.write(&pDat->EVSEMaximumPowerLimit);
    }

    pDat->EVSECurrentLimitAchieved = data.evseCurrentLimitAchieved;
    pDat->EVSEVoltageLimitAchieved = data.evseVoltageLimitAchieved;
    pDat->EVSEPowerLimitAchieved = data.evsePowerLimitAchieved;

    // pDat->SAScheduleTupleID = data.saScheduleTupleId;
    // pDat->MeterInfo_isUsed = false;
    // pDat->ReceiptRequired_isUsed = false;

    return write_ExiDocument(exiOut, stream);
}

int cExiDin70121openV2g::write_V2gWeldingDetectionReq(stV2gWeldingDetectionReq& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gWeldingDetectionReq", &this->dbg);

    dinEXIDocument exiOut;
    init_dinEXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_dinMessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_dinBodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.WeldingDetectionReq_isUsed = 1u;

    init_dinWeldingDetectionReqType(&exiOut.V2G_Message.Body.WeldingDetectionReq);
    dinWeldingDetectionReqType* pDat = &exiOut.V2G_Message.Body.WeldingDetectionReq;

    init_dinDC_EVStatusType(&pDat->DC_EVStatus);
    pDat->DC_EVStatus.EVErrorCode = (dinDC_EVErrorCodeType)data.evErrorCode;
    pDat->DC_EVStatus.EVRESSSOC = data.evRessSoc;
    pDat->DC_EVStatus.EVReady = data.evReady;

    return write_ExiDocument(exiOut, stream);
}

int cExiDin70121openV2g::write_V2gWeldingDetectionRes(stV2gWeldingDetectionRes& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gWeldingDetectionRes", &this->dbg);

    dinEXIDocument exiOut;
    init_dinEXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_dinMessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_dinBodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.WeldingDetectionRes_isUsed = 1u;

    init_dinWeldingDetectionResType(&exiOut.V2G_Message.Body.WeldingDetectionRes);
    dinWeldingDetectionResType* pDat = &exiOut.V2G_Message.Body.WeldingDetectionRes;

    pDat->ResponseCode = (dinresponseCodeType)data.responseCode;

    init_dinDC_EVSEStatusType(&pDat->DC_EVSEStatus);
    pDat->DC_EVSEStatus.EVSEStatusCode = (dinDC_EVSEStatusCodeType)data.evseStatusCode;
    pDat->DC_EVSEStatus.EVSENotification = (dinEVSENotificationType)data.evseNotification;
    pDat->DC_EVSEStatus.NotificationMaxDelay = data.notificationMaxDelay;
    pDat->DC_EVSEStatus.EVSEIsolationStatus_isUsed = false;
    if (data.evseIsolationStatus != enIsolationLevel::invalid)
    {
        pDat->DC_EVSEStatus.EVSEIsolationStatus_isUsed = true;
        pDat->DC_EVSEStatus.EVSEIsolationStatus = (dinisolationLevelType)data.evseIsolationStatus;
    }
    init_dinPhysicalValueType(&pDat->EVSEPresentVoltage);
    data.evsePresentVoltage.write(&pDat->EVSEPresentVoltage);

    return write_ExiDocument(exiOut, stream);
}

int cExiDin70121openV2g::write_V2gServiceDetailReq(stV2gServiceDetailReq& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gServiceDetailReq", &this->dbg);

    dinEXIDocument exiOut;
    init_dinEXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_dinMessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_dinBodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.ServiceDetailReq_isUsed = 1u;

    init_dinServiceDetailReqType(&exiOut.V2G_Message.Body.ServiceDetailReq);
    dinServiceDetailReqType* pDat = &exiOut.V2G_Message.Body.ServiceDetailReq;

    pDat->ServiceID = data.serviceId;

    return write_ExiDocument(exiOut, stream);
}

int cExiDin70121openV2g::write_V2gServiceDetailRes(stV2gServiceDetailRes& data, stStreamControl& stream)
{
    cDebug dbg("write_V2gServiceDetailRes", &this->dbg);

    dinEXIDocument exiOut;
    init_dinEXIDocument(&exiOut);
    exiOut.V2G_Message_isUsed = 1u;

    init_dinMessageHeaderType(&exiOut.V2G_Message.Header);
    data.header.sessionId.asChar((char*)exiOut.V2G_Message.Header.SessionID.bytes, 8);
    exiOut.V2G_Message.Header.SessionID.bytesLen = data.header.sessionId.size();

    init_dinBodyType(&exiOut.V2G_Message.Body);
    exiOut.V2G_Message.Body.ServiceDetailRes_isUsed = 1u;

    init_dinServiceDetailResType(&exiOut.V2G_Message.Body.ServiceDetailRes);
    dinServiceDetailResType* pDat = &exiOut.V2G_Message.Body.ServiceDetailRes;

    pDat->ResponseCode = (dinresponseCodeType)data.responseCode;

    pDat->ServiceID = data.serviceId;

    pDat->ServiceParameterList_isUsed = 0u;
    pDat->ServiceParameterList.ParameterSet.arrayLen = 0;

    return write_ExiDocument(exiOut, stream);
}

