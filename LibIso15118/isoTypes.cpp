#define _CRT_SECURE_NO_WARNINGS

#include <cmath>
//#include <new>

#include "types.h"
#include "isoTypes.h"

#include "../LibCpp/HAL/Tools.h"
#include "../LibCpp/HAL/HW_Tools.h"

#include "openV2G/appHandshake/appHandEXIDatatypes.h"
#include "openV2G/appHandshake/appHandEXIDatatypesDecoder.h"
#include "openV2G/appHandshake/appHandEXIDatatypesEncoder.h"

using namespace std;
using namespace LibCpp;
using namespace Iso15118;

string Iso15118::enStackType_toString(enStackType stackType)
{
    switch (stackType)
    {
    case enStackType::evcc: return "EVCC";
    case enStackType::secc: return "SECC";
    default: return "<undefined>";
    }
}

string Iso15118::enV2gMsg_toString(enV2gMsg messageType)
{
    switch(messageType)
    {
    case enV2gMsg::Empty:                    return "<empty>";
    case enV2gMsg::SessionSetup:             return "SessionSetup";
    case enV2gMsg::ServiceDiscovery:         return "ServiceDiscovery";
    case enV2gMsg::ServiceDetail:            return "ServiceDetail";
    case enV2gMsg::PaymentServiceSelection:  return "PaymentServiceSelection";
    case enV2gMsg::Authorization:            return "Authorisation";
    case enV2gMsg::ChargeParameterDiscovery: return "ChargeParameterDiscovery";
    case enV2gMsg::PowerDelivery:            return "PowerDelivery";
    case enV2gMsg::ChargingStatus:           return "ChargingStatus";
    case enV2gMsg::SessionStop:              return "SessionStop";
    case enV2gMsg::CableCheck:               return "CableCheck";
    case enV2gMsg::PreCharge:                return "PreCharge";
    case enV2gMsg::CurrentDemand:            return "CurrentDemand";
    case enV2gMsg::WeldingDetection:         return "WeldingDetection";
    case enV2gMsg::MeteringReceipt:          return "MeteringReceipt";
    case enV2gMsg::PaymentDetails:           return "PaymentDetails";
    case enV2gMsg::Unimplemented:            return "<unimplemented>";
    case enV2gMsg::ReadFailure:              return "<ReadFailure>";
    default:                                 return "<undefined>";
    }
}

string Iso15118::enV2gResponse_toString(enV2gResponse response)
{
    switch(response)
    {
    case enV2gResponse::OK: return "OK";
    case enV2gResponse::OK_NewSessionEstablished: return "OK_NewSessionEstablished";
    case enV2gResponse::OK_OldSessionJoined: return "OK_OldSessionJoined";
    case enV2gResponse::OK_CertificateExpiresSoon: return "OK_CertificateExpiresSoon";
    case enV2gResponse::FAILED: return "FAILED";
    case enV2gResponse::FAILED_SequenceError: return "FAILED_SequenceError";
    case enV2gResponse::FAILED_ServiceIDInvalid: return "FAILED_ServiceIDInvalid";
    case enV2gResponse::FAILED_UnknownSession: return "FAILED_UnknownSession";
    case enV2gResponse::FAILED_ServiceSelectionInvalid: return "FAILED_ServiceSelectionInvalid";
    case enV2gResponse::FAILED_PaymentSelectionInvalid: return "FAILED_PaymentSelectionInvalid";
    case enV2gResponse::FAILED_CertificateExpired: return "FAILED_CertificateExpired";
    case enV2gResponse::FAILED_SignatureError: return "FAILED_SignatureError";
    case enV2gResponse::FAILED_NoCertificateAvailable: return "FAILED_NoCertificateAvailable";
    case enV2gResponse::FAILED_CertChainError: return "FAILED_CertChainError";
    case enV2gResponse::FAILED_ChallengeInvalid: return "FAILED_ChallengeInvalid";
    case enV2gResponse::FAILED_ContractCanceled: return "FAILED_ContractCanceled";
    case enV2gResponse::FAILED_WrongChargeParameter: return "FAILED_WrongChargeParameter";
    case enV2gResponse::FAILED_PowerDeliveryNotApplied: return "FAILED_PowerDeliveryNotApplied";
    case enV2gResponse::FAILED_TariffSelectionInvalid: return "FAILED_TariffSelectionInvalid";
    case enV2gResponse::FAILED_ChargingProfileInvalid: return "FAILED_ChargingProfileInvalid";
    case enV2gResponse::FAILED_MeteringSignatureNotValid: return "FAILED_MeteringSignatureNotValid";
    case enV2gResponse::FAILED_NoChargeServiceSelected: return "FAILED_NoChargeServiceSelected";
    case enV2gResponse::FAILED_WrongEnergyTransferMode: return "FAILED_WrongEnergyTransferMode";
    case enV2gResponse::FAILED_ContactorError: return "FAILED_ContactorError";
    case enV2gResponse::FAILED_CertificateNotAllowedAtThisEVSE: return "CertificateNotAllowedAtThisEVSE";
    case enV2gResponse::FAILED_CertificateRevoked: return "FAILED_CertificateRevoked";
    default: return "<undefined>";
//    case enV2gResponse::OK_IsolationValid: return "OK_IsolationValid";
//    case enV2gResponse::FAILED_CertificateNotYetValid: return "FAILED_CertificateNotYetValid";
//    case enV2gResponse::OK_IsolationWarning: return "OK_IsolationWarning";
//    case enV2gResponse::WARNING_CertificateExpired: return "WARNING_CertificateExpired";
//    case enV2gResponse::WARNING_NoCertificateAvailable: return "WARNING_NoCertificateAvailable";
//    case enV2gResponse::WARNING_CertValidationError: return "WARNING_CertValidationError";
//    case enV2gResponse::WARNING_CertVerificationError: return "WARNING_CertVerificationError";
//    case enV2gResponse::WARNING_ContractCanceled: return "WARNING_ContractCanceled";
//    case enV2gResponse::FAILED_SignatureError: return "Success";
//    case enV2gResponse::FAILED_IsolationFault: return "FAILED_IsolationFault";
    }
}

string Iso15118::enV2gEvState_toString(enV2gEvState state)
{
    switch(state)
    {
    case enV2gEvState::idle:                           return "Idle";
    case enV2gEvState::awaitSdp:                       return "AwaitSdp";
    case enV2gEvState::awaitSupportedAppProtocol:      return "AwaitSupportedAppProtocol";
    case enV2gEvState::awaitSessionSetup:              return "AwaitSessionSetup";
    case enV2gEvState::awaitServiceDiscovery:          return "AwaitServiceDiscovery";
    case enV2gEvState::awaitPaymentServiceSelection:   return "AwaitPaymentServiceSelection";
    case enV2gEvState::awaitAuthorization:             return "AwaitAuthorisation";
    case enV2gEvState::awaitChargeParameterDiscovery:  return "AwaitChargeParameterDiscovery";
    case enV2gEvState::awaitPowerDelivery:             return "AwaitPowerDelivery";
    case enV2gEvState::awaitChargingStatus:            return "AwaitChargingStatus";
    case enV2gEvState::awaitSessionStop:               return "AwaitSessionStop";
    case enV2gEvState::awaitCableCheck:                return "AwaitCableCheck";
    case enV2gEvState::awaitPreCharge:                 return "AwaitPreCharge";
    case enV2gEvState::awaitCurrentDemand:             return "AwaitCurrentDemand";
    case enV2gEvState::awaitWeldingDetection:          return "AwaitWeldingDetection";
    case enV2gEvState::awaitDisconnection:             return "AwaitDisconnection";
    case enV2gEvState::failure:                        return "Failure";
    case enV2gEvState::finished:                       return "Finished";
    default:                                           return "<unknown>";
    }
};

string Iso15118::enV2gSeState_toString(enV2gSeState state)
{
    switch(state)
    {
    case enV2gSeState::idle:                              return "Idle";
    case enV2gSeState::awaitSdp:                          return "AwaitSdp";
    case enV2gSeState::answeredSdp:                       return "AnsweredSdp";
    case enV2gSeState::answeredSupportedAppProtocol:      return "AnsweredSupportedAppProtocol";
    case enV2gSeState::answeredSessionSetup:              return "AnsweredSessionSetup";
    case enV2gSeState::answeredServiceDiscovery:          return "AnsweredServiceDiscovery";
    case enV2gSeState::answeredServiceDetail:             return "AnsweredServiceDetail";
    case enV2gSeState::answeredPaymentServiceSelection:   return "AnsweredPaymentServiceSelection";
    case enV2gSeState::answeredAuthorization:             return "AnsweredAuthorisation";
    case enV2gSeState::answeredChargeParameterDiscovery:  return "AnsweredChargeParameterDiscovery";
    case enV2gSeState::answeredPowerDelivery:             return "AnsweredPowerDelivery";
    case enV2gSeState::answeredChargingStatus:            return "AnsweredChargingStatus";
    case enV2gSeState::answeredSessionStop:               return "AnsweredSessionStop";
    case enV2gSeState::answeredCableCheck:                return "AnsweredCableCheck";
    case enV2gSeState::answeredPreCharge:                 return "AnsweredPreCharge";
    case enV2gSeState::answeredCurrentDemand:             return "AnsweredCurrentDemand";
    case enV2gSeState::answeredWeldingDetection:          return "AnsweredWeldingDetection";
    case enV2gSeState::failure:                           return "Failure";
    case enV2gSeState::finished:                          return "Finished";
    default:                                              return "<unknown>";
    }
};

string Iso15118::v2gTypeToString(unsigned int messageType)
{
    switch (messageType)
    {
    case V2GTP_TYPE_EXI: return string("EXI");
    case V2GTP_TYPE_SDP_REQ: return string("SdpReq");
    case V2GTP_TYPE_SDP_RES: return string("SdpRes");
    case V2GTP_TYPE_V2S: return string("V2S");
    }
    return string("<invalid>");
}

std::string Iso15118::v2gSecurityToString(unsigned int security)
{
    switch (security)
    {
    case V2G_SECURITY_TLS: return string("TLS");
    case V2G_SECURITY_TCP: return string("TCP");
    }
    return string("<invalid>");
}

std::string Iso15118::v2gTransportToString(unsigned int transport)
{
    switch (transport)
    {
    case V2G_TRANSPORT_TCP: return string("TCP");
    case V2G_TRANSPORT_UDP: return string("UDP");
    }
    return string("<invalid>");
}

std::string Iso15118::iso1serviceCategoryType_toString(iso1serviceCategoryType value)
{
    switch (value)
    {
    case iso1serviceCategoryType_EVCharging: return "EVCharging";
    case iso1serviceCategoryType_Internet: return "Internet";
    case iso1serviceCategoryType_ContractCertificate: return "ContractCertificate";
    case iso1serviceCategoryType_OtherCustom: return "OtherCustom";
    default: return "<unknown>";
    }
}

string Iso15118::iso1paymentOptionType_toString(iso1paymentOptionType value)
{
    switch (value)
    {
    case iso1paymentOptionType_ExternalPayment: return "External payment";
    case iso1paymentOptionType_Contract: return "Contract payment";
    default: return "<unknown>";
    }
}

string Iso15118::iso1EnergyTransferModeType_toString(iso1EnergyTransferModeType value)
{
    switch (value)
    {
    case iso1EnergyTransferModeType_AC_three_phase_core:    return "AC-3phase";
    case iso1EnergyTransferModeType_AC_single_phase_core:   return "AC-1phase";
    case iso1EnergyTransferModeType_DC_core:                return "DC-core";
    case iso1EnergyTransferModeType_DC_combo_core:          return "DC-combo";
    case iso1EnergyTransferModeType_DC_unique:              return "DC-unique";
    case iso1EnergyTransferModeType_DC_extended:            return "DC-extended";
    default: return "<unknown>";
    }
}

iso1EnergyTransferModeType Iso15118::iso1EnergyTransferModeType_fromString(string transferMode)
{
    if (transferMode == "AC-3phase")      return iso1EnergyTransferModeType_AC_three_phase_core;
    if (transferMode == "AC-1phase")      return iso1EnergyTransferModeType_AC_single_phase_core;
    if (transferMode == "DC-core")        return iso1EnergyTransferModeType_DC_core;
    if (transferMode == "DC-combo")       return iso1EnergyTransferModeType_DC_combo_core;
    if (transferMode == "DC-unique")      return iso1EnergyTransferModeType_DC_unique;
    if (transferMode == "DC-extended")    return iso1EnergyTransferModeType_DC_extended;
    return iso1EnergyTransferModeType_AC_three_phase_core;
}

string Iso15118::iso1SelectedServiceType_toString(iso1SelectedServiceType service)
{
    string out = stringFormat("ServiceId = %u", service.ServiceID);
    if (service.ParameterSetID_isUsed)
        out += stringFormat(", ParameterSetId = %i", service.ParameterSetID);
    else
        out += ", ParameterSetId = -";
    return out;
}

string Iso15118::iso1EVSEProcessingType_toString(iso1EVSEProcessingType value)
{
    switch (value)
    {
    case iso1EVSEProcessingType_Finished: return "Finished";
    case iso1EVSEProcessingType_Ongoing: return "Ongoing";
    case iso1EVSEProcessingType_Ongoing_WaitingForCustomerInteraction: return "Await external identification";
    default: return "<unknown>";
    }
}

string Iso15118::iso1unitSymbolType_toString(iso1unitSymbolType type)
{
    switch (type)
    {
    case iso1unitSymbolType_A:  return "A";
    case iso1unitSymbolType_V:  return "V";
    case iso1unitSymbolType_W:  return "W";
    case iso1unitSymbolType_Wh: return "Wh";
    case iso1unitSymbolType_h:  return "h";
    case iso1unitSymbolType_m:  return "m";
    case iso1unitSymbolType_s:  return "s";
    default: return "?";
    }
}

string Iso15118::iso1DC_EVErrorCodeType_toString(iso1DC_EVErrorCodeType code)
{
    switch (code)
    {
    case iso1DC_EVErrorCodeType_NO_ERROR:  return "OK";
    case iso1DC_EVErrorCodeType_FAILED_ChargerConnectorLockFault:  return "Connector lock";
    case iso1DC_EVErrorCodeType_FAILED_ChargingCurrentdifferential:  return "Current differential";
    case iso1DC_EVErrorCodeType_FAILED_EVRESSMalfunction: return "Ress Malfunction";
    case iso1DC_EVErrorCodeType_FAILED_EVShiftPosition:  return "Shift position";
    case iso1DC_EVErrorCodeType_FAILED_ChargingSystemIncompatibility:  return "Incompatibility";
    case iso1DC_EVErrorCodeType_FAILED_RESSTemperatureInhibit:  return "Ress temperature";
    case iso1DC_EVErrorCodeType_FAILED_ChargingVoltageOutOfRange:  return "Voltage out of range";
    case iso1DC_EVErrorCodeType_NoData: return "No data";
    case iso1DC_EVErrorCodeType_Reserved_A:  return "Reserved_A";
    case iso1DC_EVErrorCodeType_Reserved_B:  return "Reserved_B";
    case iso1DC_EVErrorCodeType_Reserved_C:  return "Reserved_C";
    default: return "<unknown>";
    }
}

string Iso15118::iso1chargeProgressType_toString(iso1chargeProgressType value)
{
    switch (value)
    {
    case iso1chargeProgressType_Start: return "Start";
    case iso1chargeProgressType_Stop: return "Stop";
    case iso1chargeProgressType_Renegotiate: return "Renegotiate";
    default: return "<unknown>";
    }
}

string Iso15118::iso1chargingSessionType_toString(iso1chargingSessionType value)
{
    switch (value)
    {
    case iso1chargingSessionType_Terminate: return "Terminate";
    case iso1chargingSessionType_Pause: return "Pause";
    default: return "<unknown>";
    }
}

string Iso15118::enV2gNotification_toString(enV2gNotification type)
{
    switch (type)
    {
    case iso1EVSENotificationType_None: return "None";
    case iso1EVSENotificationType_ReNegotiation: return "Renegotiation";
    case iso1EVSENotificationType_StopCharging: return "Stop charging";
    default: return "<undefined>";
    }
}

string Iso15118::enIsolationLevel_toString(enIsolationLevel level)
{
    switch (level)
    {
    case enIsolationLevel::invalid: return "invalid";
    case enIsolationLevel::valid: return "valid";
    case enIsolationLevel::fault: return "fault";
    case enIsolationLevel::noIMD: return "noIMD";
    default: return "<undefined>";
    }
}

string Iso15118::enDcEvseStatusCode_toString(enDcEvseStatusCode code)
{
    switch (code)
    {
    case enDcEvseStatusCode::NotReady: return "NotReady";
    case enDcEvseStatusCode::Ready: return "Ready";
    case enDcEvseStatusCode::Shutdown: return "Shutdown";
    case enDcEvseStatusCode::UtilityInterruptEvent: return "UtilityInterruptEvent";
    case enDcEvseStatusCode::IsolationMonitoringActive: return "IsolationMonitoringActive";
    case enDcEvseStatusCode::EmergencyShutdown: return "EmergencyShutdown";
    case enDcEvseStatusCode::Malfunction: return "Malfunction";
    case enDcEvseStatusCode::Reserved_8: return "Reserved_8";
    case enDcEvseStatusCode::Reserved_9: return "Reserved_9";
    case enDcEvseStatusCode::Reserved_A: return "Reserved_A";
    case enDcEvseStatusCode::Reserved_B: return "Reserved_B";
    case enDcEvseStatusCode::Reserved_C: return "Reserved_C";
    default: return "<undefined>";
    }
}

cExiByteArray::cExiByteArray(unsigned int size, const char* pSourceData)
{
    (void)size;
    (void)pSourceData;
}

cExiByteArray::cExiByteArray(const int64_t source, bool bigEndian)
{
    set(source, bigEndian);
}

cExiByteArray::cExiByteArray(const string source)
{
    set(source);
}

cExiByteArray::cExiByteArray(const cByteArray& source)
{
    this->set(source);
    // *this = source;
}

cExiByteArray::cExiByteArray(uint16_t charactersLen, exi_string_character_t* characters)
{
    setFromExiCharacters(charactersLen, characters);
}

cExiByteArray::cExiByteArray(cByteArray& byteArray)
{
    set(byteArray.data(), byteArray.size());
}

void cExiByteArray::setFromExiCharacters(uint16_t charactersLen, exi_string_character_t* characters)
{
    set((unsigned int)charactersLen);
    for (int i=0; i<charactersLen; i++)
        data()[i] = characters[i];
}

void cExiByteArray::asExiCharacters(uint16_t maxLen, uint16_t& charactersLen, exi_string_character_t* characters)
{
    int len = size();
    if (maxLen<len) len = maxLen;
    for (int i=0; i<len; i++)
        characters[i] = data()[i];
    charactersLen = len;
}

Iso15118::stAppProtocolType::stAppProtocolType()
{
    protocolNamespace = "urn:iso:15118:2:2013:MsgDef";  // "urn:iso:15118:2:2010:MsgDef", "urn:iso:15118:2:2013:MsgDef", "urn:iso:15118:2:2013:MsgDef" "urn:din:70121:2012:MsgDef"
    versionNumberMajor = 2;
    versionNumberMinor = 0;
    schemaID = 10;
    priority = 1;
}

std::string Iso15118::stAppProtocolType::toString()
{
    return string("ProtocolNamespace = ") + protocolNamespace + ", version = " + to_string(versionNumberMajor) + "." + to_string(versionNumberMinor) + ", SchemaID = " + to_string(schemaID) + ", Priority = " + to_string(priority);
}

stExiPysicalValue::stExiPysicalValue()
{
    value = 0;
    exponent = 0;
    unit = iso1unitSymbolType_s;
}

void stExiPysicalValue::read(iso1PhysicalValueType* pValue)
{
    value = pValue->Value;
    exponent = pValue->Multiplier;
    unit = pValue->Unit;
}

void stExiPysicalValue::write(iso1PhysicalValueType* pValue)
{
    pValue->Value = value;
    pValue->Multiplier = exponent;
    pValue->Unit = unit;
}

void stExiPysicalValue::read(dinPhysicalValueType* pValue)
{
    value = pValue->Value;
    exponent = pValue->Multiplier;
    if (pValue->Unit_isUsed)
        unit = to_iso1unitSymbolType(pValue->Unit);
    else
        unit = (iso1unitSymbolType)0xFF;
}

void stExiPysicalValue::write(dinPhysicalValueType* pValue)
{
    pValue->Value = value;
    pValue->Multiplier = exponent;
    if (unit != (iso1unitSymbolType)0xFF)
    {
        pValue->Unit_isUsed = true;
        pValue->Unit = to_dinunitSymbolType(unit);
    }
    else
        pValue->Unit_isUsed = false;
}

float stExiPysicalValue::toFloat()
{
    return (float)value * (float)pow((float)10, exponent);
}

void stExiPysicalValue::fromFloat(float value, iso1unitSymbolType unit, int8_t exponent)
{
    this->unit = unit;
    if (value==0)
    {
        this->value = 0;
        this->exponent = 0;
        return;
    }
    if (exponent == 0 && fabs(value)<32768)
    {
        this->exponent = 0;
        this->value = (int16_t)round(value);
        return;
    }
    if (exponent != 0)
    {
        int val = (int)round(value / pow((float)10, exponent));
        this->value = val;
        this->exponent = exponent;
        return;
    }
    int exp = (int)ceil(log10(value)) - 5;
    int val = (int)round(value / pow((float)10, exponent));
    this->value = val;
    this->exponent = exp;
}

string stExiPysicalValue::toString()
{
    return stringFormat("%iE%i%s", (int)value, (int)exponent, iso1unitSymbolType_toString(unit).c_str());
}

iso1unitSymbolType Iso15118::to_iso1unitSymbolType(dinunitSymbolType symbol)
{
    switch (symbol)
    {
    case dinunitSymbolType_h: return iso1unitSymbolType_h;
    case dinunitSymbolType_m: return iso1unitSymbolType_m;
    case dinunitSymbolType_s: return iso1unitSymbolType_s;
    case dinunitSymbolType_A: return iso1unitSymbolType_A;
    case dinunitSymbolType_Ah: return iso1unitSymbolType_A;     // no correct translation possible
    case dinunitSymbolType_V: return iso1unitSymbolType_V;
    case dinunitSymbolType_VA: return iso1unitSymbolType_W;
    case dinunitSymbolType_W: return iso1unitSymbolType_W;
    case dinunitSymbolType_W_s: return iso1unitSymbolType_Wh;   // no correct translation possible
    case dinunitSymbolType_Wh: return iso1unitSymbolType_Wh;
    default: return (iso1unitSymbolType)0xFF;
    }
}

dinunitSymbolType Iso15118::to_dinunitSymbolType(iso1unitSymbolType symbol)
{
    switch (symbol)
    {
    case iso1unitSymbolType_h: return dinunitSymbolType_h;
    case iso1unitSymbolType_m: return dinunitSymbolType_m;
    case iso1unitSymbolType_s: return dinunitSymbolType_s;
    case iso1unitSymbolType_A: return dinunitSymbolType_A;
    case iso1unitSymbolType_V: return dinunitSymbolType_V;
    case iso1unitSymbolType_W: return dinunitSymbolType_W;
    case iso1unitSymbolType_Wh: return dinunitSymbolType_Wh;
    default: return (dinunitSymbolType)0xFF;
    }
}

stPmaxSchedule::stPmaxSchedule()
{
    maxP.fromFloat(3000, iso1unitSymbolType_W, 3);
    relativeStartTime = 0;
    //relativeDuration = 0;
    relativeDuration = 86400; // Minimum of 24h at last schedule entry and if EV departure time is not set [V2G2-303],[V2G2-304].
}

string stPmaxSchedule::toString()
{
    string out;
    out += string("Pmax = ") + maxP.toString();
    out += stringFormat(", Rel. start Time = %i", (int)relativeStartTime);
    out += stringFormat(", Rel. duration = %i", (int)relativeDuration);
    return out;
}

stSaScheduleTuple::stSaScheduleTuple()
{
    id = 1;
    stPmaxSchedule maxPSchedule;
    maxPSchedules.list.push_back(maxPSchedule);
}

string stSaScheduleTuple::toString()
{
    string out;
    out += "Id = " + to_string(id);
    out += ", PmaxSchedules = [";
    for (unsigned int i=0; i<maxPSchedules.list.size(); i++)
    {
        out += maxPSchedules.list[i].toString();
        if (i<maxPSchedules.list.size()-1)
            out += ", ";
    }
    out += "]";
    return out;
}

string stSaSchedules::toString()
{
    string out;
    out += "SaSchedules = [";
    for (unsigned int i=0; i<list.size(); i++)
    {
        if (i) out += ", ";
        out += list[i].toString();
    }
    out += "]";
    return out;
}

stDcEvStatus::stDcEvStatus()
{
    errorCode = iso1DC_EVErrorCodeType_NO_ERROR;
    ressSOC = 0;
    ready = 0;
}

string stDcEvStatus::toString()
{
    string out;
    out += string("ErrorCode = ") + iso1DC_EVErrorCodeType_toString(errorCode);
    out += stringFormat(" RessSOC = %i", (int)ressSOC);
    out += stringFormat(" Ready = %i", ready);
    return out;
}

stV2gSupportedAppProtocolReq::stV2gSupportedAppProtocolReq()
{
    stAppProtocolType element;
    list.push_back(element);
}

string stV2gSupportedAppProtocolReq::toString()
{
    string out;
    out += string("V2G message = SupportedAppProtocolReq, list(") + to_string(list.size()) + ") of Protocols = [";
    unsigned int i = 0;
    for (stAppProtocolType prot : list)
    {
        out += to_string(i++) + ": [" + prot.toString() + "]";
    }
    if (i<list.size()-1)
        out += string(", ");
    out += string("]");
    return out;
}

string Iso15118::enSupportedAppProtocolResponseCode_toString(enSupportedAppProtocolResponseCode code)
{
    switch (code)
    {
    case enSupportedAppProtocolResponseCode_Success: return "Success";
    case enSupportedAppProtocolResponseCode_MinorDeviations: return "Success_MinorDeviations";
    default : return "Failure";
    }
}

stV2gSupportedAppProtocolRes::stV2gSupportedAppProtocolRes()
{
    responseCode = enSupportedAppProtocolResponseCode_Failure;
    schemaID = 0;
}

string stV2gSupportedAppProtocolRes::toString()
{
    string out;
    out += string("V2G message = SupportedAppProtocolRes, response = ") + enSupportedAppProtocolResponseCode_toString(responseCode) + string(", SchemaID = ") + to_string(schemaID);
    return out;
}

stV2gTpHeader::stV2gTpHeader()
{
    set(0, 0, V2GTP_VERSION);
}

stV2gTpHeader::stV2gTpHeader(unsigned int type, unsigned int len, unsigned int version)
{
    set(type, len, version);
}

void stV2gTpHeader::set(unsigned int type, unsigned int len, unsigned int version)
{
    payloadLength = len;
    payloadType = type;
    protocolVersion = version;
}

string stV2gTpHeader::toString()
{
    char txt[256];
    sprintf(txt, "Version = %u, Type = %s, Payload length = %u", protocolVersion, v2gTypeToString(payloadType).c_str(), payloadLength);
    return string(txt);
}

stV2gSdpReq::stV2gSdpReq()
{
    security = V2G_SECURITY_TCP;
    transportProtocol = V2G_TRANSPORT_TCP;
}

string stV2gSdpReq::toString()
{
    char txt[256];
    sprintf(txt, "V2G message = %s, Security = %s, Transport = %s", v2gTypeToString(V2GTP_TYPE_SDP_REQ).c_str(), v2gSecurityToString(security).c_str(), v2gTransportToString(transportProtocol).c_str());
    return string(txt);
}

string stV2gSdpRes::toString()
{
    char txt[256];
    sprintf(txt, "V2G message = %s, IpAddress = %s, Security = %s, Transport = %s", v2gTypeToString(V2GTP_TYPE_SDP_RES).c_str(), serverIpAddress.toString(true, true).c_str(), v2gSecurityToString(security).c_str(), v2gTransportToString(transportProtocol).c_str());
    return string(txt);
}

stV2gHeader::stV2gHeader(enV2gMsg v2gMsgType, enV2gMode v2gMsgMode)
{
    this->tpHeader.payloadType = V2GTP_TYPE_EXI;
    this->messageType = v2gMsgType;
    this->messageMode = v2gMsgMode;
    this->sessionId = cByteArray(1u);
}

string stV2gHeader::toString()
{
    return string("V2G ") + enV2gMsg_toString(messageType) + ((messageMode==enV2gMode::Req) ? " request: " : " response: ") + "SessionId = " + sessionId.toString();
}

Iso15118::stV2gSessionSetupReq::stV2gSessionSetupReq() :
    evccId(6u)
{
    header.messageType = enV2gMsg::SessionSetup;
    header.messageMode = enV2gMode::Req;

    evccId[5] = 1;
}

string stV2gSessionSetupReq::toString()
{
    string out;
    out += header.toString() + " ";
    out += string(", EVCC_ID = ") + evccId.toAllTypesString() + " ";
    return out;
}

stV2gSessionSetupRes::stV2gSessionSetupRes()
{
    header.messageType = enV2gMsg::SessionSetup;
    header.messageMode = enV2gMode::Res;

    responseCode = enV2gResponse::OK;
    seccId.set("ZZ00000", 7);
    seccTimeStamp = 0x80000000;  // Invalid code
}

string stV2gSessionSetupRes::toString()
{
    string out;
    out += header.toString() + " ";
    out += string(", response = ") + enV2gResponse_toString(responseCode) + " ";
    out += string(", SECC_ID = ") + seccId.toAllTypesString();
    return out;
}

stV2gServiceDiscoveryReq::stV2gServiceDiscoveryReq()
{
    header.messageType = enV2gMsg::ServiceDiscovery;
    header.messageMode = enV2gMode::Req;

    serviceCategory = iso1serviceCategoryType_EVCharging;
}

std::string stV2gServiceDiscoveryReq::toString()
{
    string out;
    out += header.toString() + " ";
    out += string(", ServiceCategory = ") + iso1serviceCategoryType_toString(serviceCategory);
    return out;
}

stV2gServiceDiscoveryRes::stV2gServiceDiscoveryRes()
{
    header.messageType = enV2gMsg::ServiceDiscovery;
    header.messageMode = enV2gMode::Res;

    responseCode = enV2gResponse::OK;
    chargeServiceId = 1;
    chargeServiceFree = true;
    chargeServiceCategory = iso1serviceCategoryType_EVCharging;
    paymentOptions.push_back(iso1paymentOptionType_ExternalPayment);
    energyTransferModes.push_back(iso1EnergyTransferModeType_AC_three_phase_core);
}

string stV2gServiceDiscoveryRes::toString()
{
    unsigned int i;
    string out;
    out += header.toString();
    out += string(", response = ") + enV2gResponse_toString(responseCode);
    out += string(", ChargeServiceCategory = ") + iso1serviceCategoryType_toString(chargeServiceCategory);
    out += string(", ChargeServiceFree = ") + to_string(chargeServiceFree);
    out += string(", ChargeServiceId = ") + to_string(chargeServiceId);
    out += string(", EnergyTransferModes = [");
    for (i=0; i<energyTransferModes.size()-1; i++)
        out += iso1EnergyTransferModeType_toString(energyTransferModes[i]) + ", ";
    out += iso1EnergyTransferModeType_toString(energyTransferModes[i]) + "]";
    out += string(", PaymentOptions = [");
    for (i=0; i<paymentOptions.size()-1; i++)
        out += iso1paymentOptionType_toString(paymentOptions[i]) + ", ";
    out += iso1paymentOptionType_toString(paymentOptions[i]) + "]";
    return out;
}

stV2gPaymentServiceSelectionReq::stV2gPaymentServiceSelectionReq()
{
    header.messageType = enV2gMsg::PaymentServiceSelection;
    header.messageMode = enV2gMode::Req;

    selectedPaymentOption = iso1paymentOptionType_ExternalPayment;
    iso1SelectedServiceType serviceType;
    serviceType.ServiceID = 1;
    serviceType.ParameterSetID = 0;
    serviceType.ParameterSetID_isUsed = false;
    selectedServices.push_back(serviceType);
}

string stV2gPaymentServiceSelectionReq::toString()
{
    unsigned int i;
    string out;
    out += header.toString() + " ";
    out += ", SelectedPaymentOption = " + iso1paymentOptionType_toString(selectedPaymentOption) + ", SelectedServiceOptions = ";
    if (selectedServices.size())
    {
        out += "[";
        for (i=0; i<selectedServices.size()-1; i++)
            out += iso1SelectedServiceType_toString(selectedServices[i]) + ", ";
        out += iso1SelectedServiceType_toString(selectedServices[i]) + "]";
    }
    else
        out += "<empty>";
    return out;
}

stV2gPaymentServiceSelectionRes::stV2gPaymentServiceSelectionRes()
{
    header.messageType = enV2gMsg::PaymentServiceSelection;
    header.messageMode = enV2gMode::Res;

    responseCode = enV2gResponse::OK;
}

string stV2gPaymentServiceSelectionRes::toString()
{
    string out;
    out += header.toString() + " ";
    out += string(", response = ") + enV2gResponse_toString(responseCode);
    return out;
}

stV2gAuthorizationReq::stV2gAuthorizationReq()
{
    header.messageType = enV2gMsg::Authorization;
    header.messageMode = enV2gMode::Req;
}

string stV2gAuthorizationReq::toString()
{
    string out;
    out += header.toString() + " ";
    out += string(", Id = ") + id.asString() + " ";
    out += ", GenChallenge = " + genChallenge.toString();
    return out;
}

stV2gAuthorizationRes::stV2gAuthorizationRes()
{
    header.messageType = enV2gMsg::Authorization;
    header.messageMode = enV2gMode::Res;

    responseCode = enV2gResponse::OK;
    evseProcessing = iso1EVSEProcessingType_Finished;
}

string stV2gAuthorizationRes::toString()
{
    string out;
    out += header.toString() + " ";
    out += string(", response = ") + enV2gResponse_toString(responseCode);
    out += string(", EvseProcessing = ") + iso1EVSEProcessingType_toString(evseProcessing);
    return out;
}

stV2gChargeParameterDiscoveryReq::stV2gChargeParameterDiscoveryReq()
{
    transferMode = iso1EnergyTransferModeType_AC_three_phase_core;

    header.messageType = enV2gMsg::ChargeParameterDiscovery;
    header.messageMode = enV2gMode::Req;


    maxVoltage.fromFloat(400, iso1unitSymbolType_V);
    maxCurrent.fromFloat(32, iso1unitSymbolType_A);
    energyRequest.fromFloat(0, iso1unitSymbolType_Wh);
    minCurrent.fromFloat(0, iso1unitSymbolType_A);
    maxPower.fromFloat(0, iso1unitSymbolType_W);
    energyCapacity.fromFloat(0, iso1unitSymbolType_Wh);
    // energyAmount.fromFloat(1000E3);
    departureTime = 0;
    dcEvStatus.errorCode = iso1DC_EVErrorCodeType_NO_ERROR;
    dcEvStatus.ressSOC = 0;
    dcEvStatus.ready = 0;
    bulkSOC = 0;
    fullSOC = 0;
    maxEntriesSAScheduleTuple = 0;
}

string stV2gChargeParameterDiscoveryReq::toString()
{
    string out;
    out += header.toString();
    out += ", Transfer mode = " + iso1EnergyTransferModeType_toString(transferMode);
    out += ", Max voltage = " + maxVoltage.toString();
    out += ", Max current = " + maxCurrent.toString();
    out += ", Energy request = " + energyRequest.toString();
    out += ", Departure time = " + to_string(departureTime);
    out += ", Min current = " + minCurrent.toString();
    out += ", DC EV status = [" + dcEvStatus.toString() + "]";
    out += ", Max power = " + maxPower.toString();
    out += ", Bulk SOC = " + to_string(bulkSOC);
    out += ", Energy capacity = " + energyCapacity.toString();
    out += ", Full SOC = " + to_string(bulkSOC);
    out += ", Max schedule tuples = " + to_string(maxEntriesSAScheduleTuple);
    return out;
}

stV2gChargeParameterDiscoveryRes::stV2gChargeParameterDiscoveryRes()
{
    transferMode = iso1EnergyTransferModeType_AC_three_phase_core;

    header.messageType = enV2gMsg::ChargeParameterDiscovery;
    header.messageMode = enV2gMode::Res;

    responseCode = enV2gResponse::OK;
    evseProcessing = iso1EVSEProcessingType_Finished; //iso1EVSEProcessingType_Ongoing;
    evseNotification = iso1EVSENotificationType_None;
    notificationMaxDelay = 0;
    rcd = 0;
    maxCurrent.fromFloat(10, iso1unitSymbolType_A);
    nominalVoltage.fromFloat(400, iso1unitSymbolType_V);
    maxVoltage.fromFloat(800, iso1unitSymbolType_V);
    maxPower.fromFloat(8000, iso1unitSymbolType_W);
    minCurrent.fromFloat(0, iso1unitSymbolType_A);
    minVoltage.fromFloat(0, iso1unitSymbolType_V);
    maxPeakCurrentRipple.fromFloat(1, iso1unitSymbolType_A);
    evseIsolationStatus = enIsolationLevel::valid;
    evseStatusCode = enDcEvseStatusCode::Ready;
    saScheduleListUsed = false;
    saSchedulesUsed = false;
}

string stV2gChargeParameterDiscoveryRes::toString()
{
    string out;
    out += header.toString();
    out += ", Response = " + enV2gResponse_toString(responseCode);
    out += ", EvseProcessing = " + iso1EVSEProcessingType_toString(evseProcessing);
    switch (transferMode)
    {
    case iso1EnergyTransferModeType_AC_single_phase_core:
    case iso1EnergyTransferModeType_AC_three_phase_core:
    {
        out += ", MaxCurrent = " + maxCurrent.toString();
        out += ", NominalVoltage = " + nominalVoltage.toString();
        out += ", Rcd = " + to_string(rcd);
    } break;
    case iso1EnergyTransferModeType_DC_core:
    case iso1EnergyTransferModeType_DC_combo_core:
    case iso1EnergyTransferModeType_DC_extended:
    case iso1EnergyTransferModeType_DC_unique:
    {
        out += ", MaxCurrent = " + maxCurrent.toString();
        out += ", MinCurrent = " + minCurrent.toString();
        out += ", MaxVoltage = " + maxVoltage.toString();
        out += ", MinVoltage = " + minVoltage.toString();
        out += ", MaxPower = " + maxPower.toString();
        out += ", PeakCurrentRipple = " + maxPeakCurrentRipple.toString();
        out += ", EvseStatusCode = " + enDcEvseStatusCode_toString(evseStatusCode);
        out += ", EvseIsolationStatus = " + enIsolationLevel_toString(evseIsolationStatus);
    } break;
    }

    out += ", SaScheduleListUsed = " + to_string(saScheduleListUsed);
    out += ", SaSchedulesUsed = " + to_string(saSchedulesUsed);

    out += ", EvseNotification = " + enV2gNotification_toString(evseNotification);
    out += ", NotificationMaxDelay = " + to_string(notificationMaxDelay);
    return out;
}

stV2gPowerDeliveryReq::stV2gPowerDeliveryReq()
{
    transferMode = iso1EnergyTransferModeType_AC_three_phase_core;

    header.messageType = enV2gMsg::PowerDelivery;
    header.messageMode = enV2gMode::Req;

    chargeProgress = iso1chargeProgressType_Start;
    saScheduleTupleId = 0;
    chargingProfile_isUsed = false;
    evPowerDeliveryParameter_isUsed = false;
    chargingComplete = 0;
    evStatus.EVErrorCode = iso1DC_EVErrorCodeType_NO_ERROR;
    evStatus.EVRESSSOC = 0;
    evStatus.EVReady = 0;
    bulkChargingComplete = 0;
}

string stV2gPowerDeliveryReq::toString()
{
    string out;
    out += header.toString();
    out += ", ChargeProgress = " + iso1chargeProgressType_toString(chargeProgress);
    out += ", SaScheduleTuppleId = " + to_string(saScheduleTupleId);
    out += ", ChargingProfileUsed = " + to_string(chargingProfile_isUsed);
    out += ", EvPowerDeliveryParameter_isUsed = " + to_string(evPowerDeliveryParameter_isUsed);
    out += ", EvStatus.EvReady = " + to_string(evStatus.EVReady);
    out += ", EvStatus.EVErrorCode = " + to_string(evStatus.EVErrorCode);
    out += ", EvStatus.EVRESSSOC = " + to_string(evStatus.EVRESSSOC);
    out += ", ChargingComplete = " + to_string(chargingComplete);
    out += ", BulkChargingComplete = " + to_string(bulkChargingComplete);
    return out;
}


stV2gPowerDeliveryRes::stV2gPowerDeliveryRes()
{
    transferMode = iso1EnergyTransferModeType_AC_three_phase_core;

    header.messageType = enV2gMsg::PowerDelivery;
    header.messageMode = enV2gMode::Res;

    responseCode = enV2gResponse::OK;
    evseNotification = iso1EVSENotificationType_None;
    notificationMaxDelay = 0;
    rcd = 0;
    evseStatusCode = enDcEvseStatusCode::Ready;
    evseIsolationStatus = enIsolationLevel::valid;
}

string stV2gPowerDeliveryRes::toString()
{
    string out;
    out += header.toString();
    out += ", Response = " + enV2gResponse_toString(responseCode);
    out += ", EvseNotification = " + enV2gNotification_toString(evseNotification);
    out += ", NotificationMaxDelay = " + to_string(notificationMaxDelay);
    out += ", Rcd = " + to_string(rcd);
    out += ", EvseStatusCode = " + enDcEvseStatusCode_toString(evseStatusCode);
    out += ", EvseIsolationStatus = " + enIsolationLevel_toString(evseIsolationStatus);
    return out;
}

stV2gChargingStatusReq::stV2gChargingStatusReq()
{
    header.messageType = enV2gMsg::ChargingStatus;
    header.messageMode = enV2gMode::Req;

    noContent = 0;
}

string stV2gChargingStatusReq::toString()
{
    string out;
    out += header.toString();
    return out;
}


stV2gChargingStatusRes::stV2gChargingStatusRes()
{
    header.messageType = enV2gMsg::ChargingStatus;
    header.messageMode = enV2gMode::Res;

    responseCode = enV2gResponse::OK;

    saScheduleTupleId = 1;

    evseNotification = iso1EVSENotificationType_None;
    notificationMaxDelay = 0;
    rcd = 0;

    meterInfo_isUsed = false;
    receiptRequired_isUsed = false;
}

string stV2gChargingStatusRes::toString()
{
    string out;
    out += header.toString();
    out += ", Response = " + enV2gResponse_toString(responseCode);
    out += ", Secc id = " + seccId.toAllTypesString();
    out += ", Schedule tuple = " + to_string(saScheduleTupleId);
    out += ", Max current = " + maxCurrent.toString();
    out += ", Notification = " + enV2gNotification_toString(evseNotification);
    out += ", Notification delay = " + to_string(notificationMaxDelay);
    out += ", Rcd = " + to_string(rcd);
    out += ", Meter used = " + to_string(meterInfo_isUsed);
    out += ", Receipt used = " + to_string(receiptRequired_isUsed);
    return out;
}

stV2gSessionStopReq::stV2gSessionStopReq()
{
    header.messageType = enV2gMsg::SessionStop;
    header.messageMode = enV2gMode::Req;

    stopType = iso1chargingSessionType_Terminate;
}

string stV2gSessionStopReq::toString()
{
    string out;
    out += header.toString();
    out += ", Stop type = " + iso1chargingSessionType_toString(stopType);
    return out;
}


stV2gSessionStopRes::stV2gSessionStopRes()
{
    header.messageType = enV2gMsg::SessionStop;
    header.messageMode = enV2gMode::Res;

    responseCode = enV2gResponse::OK;
}

string stV2gSessionStopRes::toString()
{
    string out;
    out += header.toString();
    out += ", Response = " + enV2gResponse_toString(responseCode);
    return out;
}

stV2gCableCheckReq::stV2gCableCheckReq()
{
    header.messageType = enV2gMsg::CableCheck;
    header.messageMode = enV2gMode::Req;

    evErrorCode = iso1DC_EVErrorCodeType_NO_ERROR;
    evRessSoc = 0;
    evReady = true;
}

string stV2gCableCheckReq::toString()
{
    string out;
    out += header.toString();
    out += ", evErrorCode = " + iso1DC_EVErrorCodeType_toString(evErrorCode);
    out += ", evRessSoc = " + to_string(evRessSoc);
    out += ", evReady = " + to_string(evReady);
    return out;
}

stV2gCableCheckRes::stV2gCableCheckRes()
{
    header.messageType = enV2gMsg::CableCheck;
    header.messageMode = enV2gMode::Res;

    responseCode = enV2gResponse::OK;

    evseProcessing = iso1EVSEProcessingType_Finished;
    evseNotification = iso1EVSENotificationType_None;
    notificationMaxDelay = 0;
    evseIsolationStatus = enIsolationLevel::valid;
    evseStatusCode = enDcEvseStatusCode::Ready;
}

string stV2gCableCheckRes::toString()
{
    string out;
    out += header.toString();
    out += string(", response = ") + enV2gResponse_toString(responseCode);
    out += ", evseProcessing = " + iso1EVSEProcessingType_toString(evseProcessing);
    out += ", evseStatusCode = " + enDcEvseStatusCode_toString(evseStatusCode);
    out += ", evseIsolationStatus = " + enIsolationLevel_toString(evseIsolationStatus);
    out += ", evseNotification = " + enV2gNotification_toString(evseNotification);
    out += ", notificationMaxDelay = " + to_string(notificationMaxDelay);
    return out;
}

stV2gPreChargeReq::stV2gPreChargeReq()
{
    header.messageType = enV2gMsg::PreCharge;
    header.messageMode = enV2gMode::Req;

    evErrorCode = iso1DC_EVErrorCodeType_NO_ERROR;
    evRessSoc = 0;
    evReady = true;
}

string stV2gPreChargeReq::toString()
{
    string out;
    out += header.toString();
    out += ", evTargetVoltage = " + evTargetVoltage.toString();
    out += ", evTargetCurrent = " + evTargetCurrent.toString();
    out += ", evErrorCode = " + iso1DC_EVErrorCodeType_toString(evErrorCode);
    out += ", evRessSoc = " + to_string(evRessSoc);
    out += ", evReady = " + to_string(evReady);
    return out;
}

stV2gPreChargeRes::stV2gPreChargeRes()
{
    header.messageType = enV2gMsg::PreCharge;
    header.messageMode = enV2gMode::Res;

    responseCode = enV2gResponse::OK;

    evseNotification = iso1EVSENotificationType_None;
    notificationMaxDelay = 0;
    evseIsolationStatus = enIsolationLevel::valid;
    evseStatusCode = enDcEvseStatusCode::Ready;
}

string stV2gPreChargeRes::toString()
{
    string out;
    out += header.toString();
    out += string(", response = ") + enV2gResponse_toString(responseCode);
    out += ", evsePresentVoltage = " + evsePresentVoltage.toString();
    out += ", evseStatusCode = " + enDcEvseStatusCode_toString(evseStatusCode);
    out += ", evseIsolationStatus = " + enIsolationLevel_toString(evseIsolationStatus);
    out += ", evseNotification = " + enV2gNotification_toString(evseNotification);
    out += ", notificationMaxDelay = " + to_string(notificationMaxDelay);
    return out;
}

/**
 * @brief Data content of the CurrentDemandReq message
 * This message contains the process data periodically exchanged during the charging being active.
 */
stV2gCurrentDemandReq::stV2gCurrentDemandReq()
{
    header.messageType = enV2gMsg::CurrentDemand;
    header.messageMode = enV2gMode::Req;

    evErrorCode = iso1DC_EVErrorCodeType_NO_ERROR;
    evRessSoc = 0;
    evReady = true;
    bulkChargingComplete = false;
    chargingComplete = false;

    evTargetCurrent.fromFloat(5, iso1unitSymbolType_A);
    evTargetVoltage.fromFloat(600, iso1unitSymbolType_V);
    evCurrentLimit.fromFloat(10, iso1unitSymbolType_A);
    evVoltageLimit.fromFloat(600, iso1unitSymbolType_V);
    evPowerLimit.fromFloat(10000, iso1unitSymbolType_W);
    remainingTimeToBulkSoC.fromFloat(0, iso1unitSymbolType_s);
    remainingTimeToFullSoC.fromFloat(0, iso1unitSymbolType_s);
}

string stV2gCurrentDemandReq::toString()
{
    string out;
    out += header.toString();
    out += ", evTargetCurrent = " + evTargetCurrent.toString();
    out += ", evTargetVoltage = " + evTargetVoltage.toString();
    out += ", evCurrentLimit = " + evCurrentLimit.toString();
    out += ", evVoltageLimit = " + evVoltageLimit.toString();
    out += ", evPowerLimit = " + evPowerLimit.toString();
    out += ", evRessSoc = " + to_string(evRessSoc);
    out += ", chargingComplete = " + to_string(bulkChargingComplete);
    out += ", bulkChargingComplete = " + to_string(bulkChargingComplete);
    out += ", remainingTimeToFullSoC = " + remainingTimeToFullSoC.toString();
    out += ", remainingTimeToBulkSoC = " + remainingTimeToBulkSoC.toString();
    out += ", evErrorCode = " + iso1DC_EVErrorCodeType_toString(evErrorCode);
    out += ", evReady = " + to_string(evReady);
    return out;
}

stV2gCurrentDemandRes::stV2gCurrentDemandRes()
{
    header.messageType = enV2gMsg::CurrentDemand;
    header.messageMode = enV2gMode::Res;

    responseCode = enV2gResponse::OK;

    seccId.set("ZZ00001", 7);

    evseNotification = iso1EVSENotificationType_None;
    notificationMaxDelay = 0;
    evseIsolationStatus = enIsolationLevel::valid;
    evseStatusCode = enDcEvseStatusCode::Ready;

    evseCurrentLimitAchieved = false;
    evseVoltageLimitAchieved = false;
    evsePowerLimitAchieved = false;

    saScheduleTupleId = 1;
    meterInfo_isUsed = false;
    receiptRequired_isUsed = false;

    evsePresentCurrent.fromFloat(5, iso1unitSymbolType_A);
    evsePresentVoltage.fromFloat(400, iso1unitSymbolType_V);
    evseCurrentLimit.fromFloat(10, iso1unitSymbolType_A);
    evseVoltageLimit.fromFloat(600, iso1unitSymbolType_V);
    evsePowerLimit.fromFloat(6000, iso1unitSymbolType_W);
}

string stV2gCurrentDemandRes::toString()
{
    string out;
    out += header.toString();
    out += string(", response = ") + enV2gResponse_toString(responseCode);
    out += ", evsePresentCurrent = " + evsePresentCurrent.toString();
    out += ", evsePresentVoltage = " + evsePresentVoltage.toString();
    out += ", evseCurrentLimitAchieved = " + to_string(evseCurrentLimitAchieved);
    out += ", evseVoltageLimitAchieved = " + to_string(evseVoltageLimitAchieved);
    out += ", evsePowerLimitAchieved = " + to_string(evsePowerLimitAchieved);
    out += ", evseCurrentLimit = " + evseCurrentLimit.toString();
    out += ", evseVoltageLimit = " + evseVoltageLimit.toString();
    out += ", evsePowerLimit = " + evsePowerLimit.toString();
    out += ", evseStatusCode = " + enDcEvseStatusCode_toString(evseStatusCode);
    out += ", evseIsolationStatus = " + enIsolationLevel_toString(evseIsolationStatus);
    out += ", evseNotification = " + enV2gNotification_toString(evseNotification);
    out += ", notificationMaxDelay = " + to_string(notificationMaxDelay);
    out += ", saScheduleTupleId = " + to_string(saScheduleTupleId);
    out += ", meterInfo_isUsed = " + to_string(meterInfo_isUsed);
    out += ", receiptRequired_isUsed = " + to_string(receiptRequired_isUsed);
    return out;
}

stV2gWeldingDetectionReq::stV2gWeldingDetectionReq()
{
    header.messageType = enV2gMsg::WeldingDetection;
    header.messageMode = enV2gMode::Req;

    evErrorCode = iso1DC_EVErrorCodeType_NO_ERROR;
    evRessSoc = 0;
    evReady = true;
}

string stV2gWeldingDetectionReq::toString()
{
    string out;
    out += header.toString();
    out += ", evErrorCode = " + iso1DC_EVErrorCodeType_toString(evErrorCode);
    out += ", evRessSoc = " + to_string(evRessSoc);
    out += ", evReady = " + to_string(evReady);
    return out;
}

stV2gWeldingDetectionRes::stV2gWeldingDetectionRes()
{
    header.messageType = enV2gMsg::WeldingDetection;
    header.messageMode = enV2gMode::Res;

    responseCode = enV2gResponse::OK;

    evsePresentVoltage.fromFloat(400, iso1unitSymbolType_V);
    evseNotification = iso1EVSENotificationType_None;
    notificationMaxDelay = 0;
    evseIsolationStatus = enIsolationLevel::valid;
    evseStatusCode = enDcEvseStatusCode::Ready;
}

string stV2gWeldingDetectionRes::toString()
{
    string out;
    out += header.toString();
    out += ", response = " + enV2gResponse_toString(responseCode);
    out += ", evsePresentVoltage = " + evsePresentVoltage.toString();
    out += ", evseStatusCode = " + enDcEvseStatusCode_toString(evseStatusCode);
    out += ", evseIsolationStatus = " + enIsolationLevel_toString(evseIsolationStatus);
    out += ", evseNotification = " + enV2gNotification_toString(evseNotification);
    out += ", notificationMaxDelay = " + to_string(notificationMaxDelay);
    return out;
}

stV2gServiceDetailReq::stV2gServiceDetailReq()
{
    header.messageType = enV2gMsg::ServiceDetail;
    header.messageMode = enV2gMode::Req;

    serviceId = 0;
}

string stV2gServiceDetailReq::toString()
{
    string out;
    out += header.toString();
    out += ", serviceId = " + to_string(serviceId);
    return out;
}

stV2gServiceDetailRes::stV2gServiceDetailRes()
{
    header.messageType = enV2gMsg::ServiceDetail;
    header.messageMode = enV2gMode::Res;

    responseCode = enV2gResponse::OK;

    serviceId = 0;
}

string stV2gServiceDetailRes::toString()
{
    string out;
    out += header.toString();
    out += string(", response = ") + enV2gResponse_toString(responseCode);
    out += ", serviceId = " + to_string(serviceId);
    return out;
}

stV2gMessage::stV2gMessage() : content()
{
    mode = enV2gMode::Req;
    type = enV2gMsg::Empty;
}

stV2gMessage::stV2gMessage(stV2gMessage& source)
{
    this->mode = enV2gMode::Res;
    this->type = enV2gMsg::Empty;
    set(source.type, source.mode, &source);
}

stV2gMessage::stV2gMessage(enV2gMsg type, enV2gMode mode)
{
    this->mode = enV2gMode::Res;
    this->type = enV2gMsg::Empty;
    set(type, mode);
}

void stV2gMessage::set(enV2gMsg type, enV2gMode mode, stV2gMessage* pSource)
{
    clear();
    this->mode = mode;
    this->type = type;

    if (mode == enV2gMode::Req)
    {
        switch (type)
        {
        case enV2gMsg::Sdp:
            new(&content.req.sdp) stV2gSdpReq();
            if (pSource) content.req.sdp = pSource->content.req.sdp;
            break;
        case enV2gMsg::SupportedAppProtocol:
            new(&content.req.supportedAppProtocol) stV2gSupportedAppProtocolReq();
            if (pSource) content.req.supportedAppProtocol = pSource->content.req.supportedAppProtocol;
            break;
        case enV2gMsg::SessionSetup:
            new(&content.req.sessionSetup) stV2gSessionSetupReq();
            if (pSource) content.req.sessionSetup = pSource->content.req.sessionSetup;
            break;
        case enV2gMsg::ServiceDiscovery:
            new(&content.req.serviceDiscovery) stV2gServiceDiscoveryReq();
            if (pSource) content.req.serviceDiscovery = pSource->content.req.serviceDiscovery;
            break;
        case enV2gMsg::PaymentServiceSelection:
            new(&content.req.paymentServiceSelection) stV2gPaymentServiceSelectionReq();
            if (pSource) content.req.paymentServiceSelection = pSource->content.req.paymentServiceSelection;
            break;
        case enV2gMsg::Authorization:
            new(&content.req.authorization) stV2gAuthorizationReq();
            if (pSource) content.req.authorization = pSource->content.req.authorization;
            break;
        case enV2gMsg::ChargeParameterDiscovery:
            new(&content.req.chargeParameterDiscovery) stV2gChargeParameterDiscoveryReq();
            if (pSource) content.req.chargeParameterDiscovery = pSource->content.req.chargeParameterDiscovery;
            break;
        case enV2gMsg::PowerDelivery:
            new(&content.req.powerDelivery) stV2gPowerDeliveryReq();
            if (pSource) content.req.powerDelivery = pSource->content.req.powerDelivery;
            break;
        case enV2gMsg::ChargingStatus:
            new(&content.req.chargingStatus) stV2gChargingStatusReq();
            if (pSource) content.req.chargingStatus = pSource->content.req.chargingStatus;
            break;
        case enV2gMsg::SessionStop:
            new(&content.req.sessionStop) stV2gSessionStopReq();
            if (pSource) content.req.sessionStop = pSource->content.req.sessionStop;
            break;
        case enV2gMsg::CableCheck:
            new(&content.req.cableCheck) stV2gCableCheckReq();
            if (pSource) content.req.cableCheck = pSource->content.req.cableCheck;
            break;
        case enV2gMsg::PreCharge:
            new(&content.req.preCharge) stV2gPreChargeReq();
            if (pSource) content.req.preCharge = pSource->content.req.preCharge;
            break;
        case enV2gMsg::CurrentDemand:
            new(&content.req.currentDemand) stV2gCurrentDemandReq();
            if (pSource) content.req.currentDemand = pSource->content.req.currentDemand;
            break;
        case enV2gMsg::WeldingDetection:
            new(&content.req.weldingDetection) stV2gWeldingDetectionReq();
            if (pSource) content.req.weldingDetection = pSource->content.req.weldingDetection;
            break;
        case enV2gMsg::ServiceDetail:
            new(&content.req.serviceDetail) stV2gServiceDetailReq();
            if (pSource) content.req.serviceDetail = pSource->content.req.serviceDetail;
            break;
        case enV2gMsg::MeteringReceipt:
            break;
        case enV2gMsg::PaymentDetails:
            break;
        default:;
        }
    }
    else if (mode == enV2gMode::Res)
    {
        switch (type)
        {
        case enV2gMsg::Sdp:
            new(&content.res.sdp) stV2gSdpRes();
            if (pSource) content.res.sdp = pSource->content.res.sdp;
            break;
        case enV2gMsg::SupportedAppProtocol:
            new(&content.res.serviceDetail) stV2gSupportedAppProtocolRes();
            if (pSource) content.res.serviceDetail = pSource->content.res.serviceDetail;
            break;
        case enV2gMsg::SessionSetup:
            new(&content.res.sessionSetup) stV2gSessionSetupRes();
            if (pSource) content.res.sessionSetup = pSource->content.res.sessionSetup;
            break;
        case enV2gMsg::ServiceDiscovery:
            new(&content.res.serviceDiscovery) stV2gServiceDiscoveryRes();
            if (pSource) content.res.serviceDiscovery = pSource->content.res.serviceDiscovery;
            break;
        case enV2gMsg::PaymentServiceSelection:
            new(&content.res.paymentServiceSelection) stV2gPaymentServiceSelectionRes();
            if (pSource) content.res.paymentServiceSelection = pSource->content.res.paymentServiceSelection;
            break;
        case enV2gMsg::Authorization:
            new(&content.res.authorization) stV2gAuthorizationRes();
            if (pSource) content.res.authorization = pSource->content.res.authorization;
            break;
        case enV2gMsg::ChargeParameterDiscovery:
            new(&content.res.chargeParameterDiscovery) stV2gChargeParameterDiscoveryRes();
            if (pSource) content.res.chargeParameterDiscovery = pSource->content.res.chargeParameterDiscovery;
            break;
        case enV2gMsg::PowerDelivery:
            new(&content.res.powerDelivery) stV2gPowerDeliveryRes();
            if (pSource) content.res.powerDelivery = pSource->content.res.powerDelivery;
            break;
        case enV2gMsg::ChargingStatus:
            new(&content.res.chargingStatus) stV2gChargingStatusRes();
            if (pSource) content.res.chargingStatus = pSource->content.res.chargingStatus;
            break;
        case enV2gMsg::SessionStop:
            new(&content.res.sessionStop) stV2gSessionStopRes();
            if (pSource) content.res.sessionStop = pSource->content.res.sessionStop;
            break;
        case enV2gMsg::CableCheck:
            new(&content.res.cableCheck) stV2gCableCheckRes();
            if (pSource) content.res.cableCheck = pSource->content.res.cableCheck;
            break;
        case enV2gMsg::PreCharge:
            new(&content.res.preCharge) stV2gPreChargeRes();
            if (pSource) content.res.preCharge = pSource->content.res.preCharge;
            break;
        case enV2gMsg::CurrentDemand:
            new(&content.res.currentDemand) stV2gCurrentDemandRes();
            if (pSource) content.res.currentDemand = pSource->content.res.currentDemand;
            break;
        case enV2gMsg::WeldingDetection:
            new(&content.res.weldingDetection) stV2gWeldingDetectionRes();
            if (pSource) content.res.weldingDetection = pSource->content.res.weldingDetection;
            break;
        case enV2gMsg::ServiceDetail:
            new(&content.res.serviceDetail) stV2gServiceDetailRes();
            if (pSource) content.res.serviceDetail = pSource->content.res.serviceDetail;
            break;
        case enV2gMsg::MeteringReceipt:
            break;
        case enV2gMsg::PaymentDetails:
            break;
        default:;
        }
    }
}

stV2gMessage::~stV2gMessage()
{
    clear();
}

void stV2gMessage::clear()
{
    if (type == enV2gMsg::Empty)
        return;

    if (mode == enV2gMode::Req)
    {
        switch (type)
        {
        case enV2gMsg::Sdp:
            content.req.sdp.~stV2gSdpReq();
            break;
        case enV2gMsg::SupportedAppProtocol:
            content.req.supportedAppProtocol.~stV2gSupportedAppProtocolReq();
            break;
        case enV2gMsg::SessionSetup:
            content.req.sessionSetup.~stV2gSessionSetupReq();
            break;
        case enV2gMsg::ServiceDiscovery:
            content.req.serviceDiscovery.~stV2gServiceDiscoveryReq();
            break;
        case enV2gMsg::PaymentServiceSelection:
            content.req.paymentServiceSelection.~stV2gPaymentServiceSelectionReq();
            break;
        case enV2gMsg::Authorization:
            content.req.authorization.~stV2gAuthorizationReq();
            break;
        case enV2gMsg::ChargeParameterDiscovery:
            content.req.chargeParameterDiscovery.~stV2gChargeParameterDiscoveryReq();
            break;
        case enV2gMsg::PowerDelivery:
            content.req.powerDelivery.~stV2gPowerDeliveryReq();
            break;
        case enV2gMsg::ChargingStatus:
            content.req.chargingStatus.~stV2gChargingStatusReq();
            break;
        case enV2gMsg::SessionStop:
            content.req.sessionStop.~stV2gSessionStopReq();
            break;
        case enV2gMsg::CableCheck:
            content.req.cableCheck.~stV2gCableCheckReq();
            break;
        case enV2gMsg::PreCharge:
            content.req.preCharge.~stV2gPreChargeReq();
            break;
        case enV2gMsg::CurrentDemand:
            content.req.currentDemand.~stV2gCurrentDemandReq();
            break;
        case enV2gMsg::WeldingDetection:
            content.req.weldingDetection.~stV2gWeldingDetectionReq();
            break;
        case enV2gMsg::ServiceDetail:
            content.req.serviceDetail.~stV2gServiceDetailReq();
            break;
        case enV2gMsg::MeteringReceipt:
            break;
        case enV2gMsg::PaymentDetails:
            break;
        default:;
        }
    }
    else if (mode == enV2gMode::Res)
    {
        switch (type)
        {
        case enV2gMsg::Sdp:
            content.res.sdp.~stV2gSdpRes();
            break;
        case enV2gMsg::SupportedAppProtocol:
            content.res.supportedAppProtocol.~stV2gSupportedAppProtocolRes();
            break;
        case enV2gMsg::SessionSetup:
            content.res.sessionSetup.~stV2gSessionSetupRes();
            break;
        case enV2gMsg::ServiceDiscovery:
            content.res.serviceDiscovery.~stV2gServiceDiscoveryRes();
            break;
        case enV2gMsg::PaymentServiceSelection:
            content.res.paymentServiceSelection.~stV2gPaymentServiceSelectionRes();
            break;
        case enV2gMsg::Authorization:
            content.res.authorization.~stV2gAuthorizationRes();
            break;
        case enV2gMsg::ChargeParameterDiscovery:
            content.res.chargeParameterDiscovery.~stV2gChargeParameterDiscoveryRes();
            break;
        case enV2gMsg::PowerDelivery:
            content.res.powerDelivery.~stV2gPowerDeliveryRes();
            break;
        case enV2gMsg::ChargingStatus:
            content.res.chargingStatus.~stV2gChargingStatusRes();
            break;
        case enV2gMsg::SessionStop:
            content.res.sessionStop.~stV2gSessionStopRes();
            break;
        case enV2gMsg::CableCheck:
            content.res.cableCheck.~stV2gCableCheckRes();
            break;
        case enV2gMsg::PreCharge:
            content.res.preCharge.~stV2gPreChargeRes();
            break;
        case enV2gMsg::CurrentDemand:
            content.res.currentDemand.~stV2gCurrentDemandRes();
            break;
        case enV2gMsg::WeldingDetection:
            content.res.weldingDetection.~stV2gWeldingDetectionRes();
            break;
        case enV2gMsg::ServiceDetail:
            content.res.serviceDetail.~stV2gServiceDetailRes();
            break;
        case enV2gMsg::MeteringReceipt:
            break;
        case enV2gMsg::PaymentDetails:
            break;
        default:;
        }
    }
    this->mode = enV2gMode::Res;
    this->type = enV2gMsg::Empty;
}

Iso15118::stStreamControl::stStreamControl(char* stream, unsigned int size)
{
    set(stream, size);
}

void Iso15118::stStreamControl::set(char* stream, unsigned int size)
{
    data = stream;
    length = size;
}

void Iso15118::stStreamControl::move(unsigned int bytes)
{
    if (length>=bytes)
    {
        data += bytes;
        length -= bytes;
    }
    else
    {
        data += length;
        length = 0;
    }
}

bitstream_t Iso15118::inStream(stStreamControl stream, size_t& pos)
{
    bitstream_t iStream;
    iStream.size = (size_t)stream.length;
    iStream.data = (uint8_t*)stream.data;
    iStream.pos = &pos;
    iStream.buffer = 0;
    iStream.capacity = 0;
    return iStream;
}

bitstream_t Iso15118::outStream(stStreamControl stream, size_t& pos)
{
    bitstream_t oStream;
    oStream.size = (size_t)stream.length;
    oStream.data = (uint8_t*)stream.data;
    oStream.pos = &pos;
    oStream.buffer = 0;
    oStream.capacity = 8;
    return oStream;
}

std::string Iso15118::stStreamControl::toString()
{
    return cIp::instance().messageString(data, length);
}

/**
 * @brief Parses a byte buffer stream for the V2GTP header
 * @param result Payload datastruct
 * @param stream Pointer to the first byte to be streamed
 * @param streamLength Length of the input stream counted from the stream pointer
 * @return Pointer to the first byte not being consumed. Zero in case of a parsing error.
 */
bool Iso15118::read_V2gTpHeader(stV2gTpHeader& result, stStreamControl& stream)
{
    if (!stream.data)
    {stream.length = 0; return false;}
    if (stream.length < V2GTP_HEADER_LENGTH)
    {stream.length = 0; return false;}
    unsigned char help = ~((unsigned char)(stream.data)[0]); // To awoid Raspberry Pi compiler error indicated by the warning: comparison of promoted bitwise complement of an unsigned value with unsigned.
    if ((unsigned char)(stream.data)[1] != help)
    {stream.length = 0; return false;}

    stV2gTpHeaderMsg* pMsg = (stV2gTpHeaderMsg*)stream.data;

    BigEndian(&pMsg->payloadType);
    BigEndian(&pMsg->payloadLength);

    result.protocolVersion = pMsg->protocolVersion;
    result.payloadType = pMsg->payloadType;
    result.payloadLength = pMsg->payloadLength;

    stream.move(V2GTP_HEADER_LENGTH);
    if (!(result.payloadType == V2GTP_TYPE_SDP_RES || result.payloadType == V2GTP_TYPE_SDP_REQ || result.payloadType == V2GTP_TYPE_EXI || result.payloadType == V2GTP_TYPE_V2S))
        return false;
    return true;
}

int Iso15118::write_V2gTpHeader(stV2gTpHeader& data, stStreamControl& stream)
{
    if (!stream.data) {stream.length = 0; return 0;}
    if (stream.length < V2GTP_HEADER_LENGTH) {stream.length = 0; return 0;}

    stV2gTpHeaderMsg* pMsg = (stV2gTpHeaderMsg*)stream.data;

    pMsg->protocolVersion = data.protocolVersion;
    pMsg->protocolVersionInv = ~((uint8_t)data.protocolVersion);
    pMsg->payloadType = data.payloadType;
    pMsg->payloadLength = data.payloadLength;

    BigEndian(&pMsg->payloadType);
    BigEndian(&pMsg->payloadLength);

    stream.move(V2GTP_HEADER_LENGTH);
    return V2GTP_HEADER_LENGTH;
}

bool Iso15118::read_V2gSdpReq(stV2gSdpReq& result, stV2gTpHeader& headerCheck, stStreamControl& stream)
{
    if (!stream.data) return false;
    if (stream.length < V2G_SDP_REQ_LENGTH) return false;
    if (headerCheck.payloadLength != V2G_SDP_REQ_LENGTH) return false;

    stV2gSdpReqMsg* pMsg = (stV2gSdpReqMsg*)stream.data;

    result.security = pMsg->security;
    result.transportProtocol = pMsg->transportProtocol;

    stream.move(2);
    return true;
}

bool Iso15118::read_V2gSdpRes(stV2gSdpRes& result, stV2gTpHeader& headerCheck, stStreamControl& stream)
{
    if (!stream.data) return false;
    if (stream.length < V2G_SDP_RES_LENGTH) return false;
    if (headerCheck.payloadLength != V2G_SDP_RES_LENGTH) return false;

    stV2gSdpResMsg* pMsg = (stV2gSdpResMsg*)stream.data;

    result.security = pMsg->security;
    result.transportProtocol = pMsg->transportProtocol;

    result.serverIpAddress.family = LibCpp::enAddressFamily_IPv6;
    for (int i = 0; i < 8; i++)
    {
        BigEndian(&pMsg->ipAddress[i]);
        result.serverIpAddress.address[i] = pMsg->ipAddress[i];
    }
    BigEndian(&pMsg->port);
    result.serverIpAddress.port = pMsg->port;

    stream.move(V2G_SDP_RES_LENGTH);
    return true;
}

int Iso15118::write_V2gSdpReq(stV2gSdpReq& data, stStreamControl& stream)
{
    cDebug dbg("Iso15118::write_V2gSdpReq");

    if (!stream.data)
    {
        dbg.printf(enDebugLevel_Fatal, "Send buffer destination missing!");
        stream.length = 0;
        return 0;
    }
    if (stream.length < (V2GTP_HEADER_LENGTH + V2G_SDP_REQ_LENGTH))
    {
        dbg.printf(enDebugLevel_Fatal, "Send buffer size is too low!");
        stream.length = 0;
        return 0;
    }

    stV2gTpHeader header(V2GTP_TYPE_SDP_REQ, V2G_SDP_REQ_LENGTH);
    write_V2gTpHeader(header, stream);

    stV2gSdpReqMsg* pMsg = (stV2gSdpReqMsg*)stream.data;

    pMsg->security = data.security;
    pMsg->transportProtocol = data.transportProtocol;

    stream.move(V2GTP_HEADER_LENGTH + V2G_SDP_REQ_LENGTH);
    return (V2GTP_HEADER_LENGTH + V2G_SDP_REQ_LENGTH);
}

int Iso15118::write_V2gSdpRes(stV2gSdpRes& data, stStreamControl& stream)
{
    cDebug dbg("Iso15118::write_V2gSdpRes");

    if (!stream.data)
    {
        dbg.printf(enDebugLevel_Fatal, "Send buffer destination missing!");
        stream.length = 0;
        return 0;
    }
    if (stream.length < (V2GTP_HEADER_LENGTH + V2G_SDP_RES_LENGTH))
    {
        dbg.printf(enDebugLevel_Fatal, "Send buffer size is too low!");
        stream.length = 0;
        return 0;
    }

    stV2gTpHeader header(V2GTP_TYPE_SDP_RES, V2G_SDP_RES_LENGTH);
    write_V2gTpHeader(header, stream);

    stV2gSdpResMsg* pMsg = (stV2gSdpResMsg*)stream.data;

    pMsg->security = data.security;
    pMsg->transportProtocol = data.transportProtocol;
    pMsg->port = data.serverIpAddress.port;
    BigEndian(&pMsg->port);
    for (int i = 0; i < 8; i++)
    {
        pMsg->ipAddress[i] = data.serverIpAddress.address[i];
        BigEndian(&pMsg->ipAddress[i]);
    }

    stream.move(V2GTP_HEADER_LENGTH + V2G_SDP_RES_LENGTH);
    return (V2GTP_HEADER_LENGTH + V2G_SDP_RES_LENGTH);
}

int Iso15118::write_V2s(stStreamControl& stream)
{
    cDebug dbg("Iso15118::write_V2gSdpRes");

    if (!stream.data)
    {
        dbg.printf(enDebugLevel_Fatal, "Send buffer destination missing!");
        stream.length = 0;
        return 0;
    }
    if (stream.length < (V2GTP_HEADER_LENGTH))
    {
        dbg.printf(enDebugLevel_Fatal, "Send buffer size is too low!");
        stream.length = 0;
        return 0;
    }

    stV2gTpHeader header(V2GTP_TYPE_V2S, 0);
    write_V2gTpHeader(header, stream);

    stream.move(V2GTP_HEADER_LENGTH);
    return (V2GTP_HEADER_LENGTH);
}

bool Iso15118::read_V2gSupportedAppProtocolReq(stV2gSupportedAppProtocolReq& result, stStreamControl& stream)
{
    cDebug dbg("Iso15118::read_V2gSupportedAppProtocolReq");

    struct appHandEXIDocument exiDoc;
    size_t pos = 0;
    bitstream_t iStream;
    iStream.size = (size_t)stream.length;
    iStream.data = (uint8_t*)stream.data;
    iStream.pos = &pos;
    iStream.buffer = 0;
    iStream.capacity = 0;

    int err = decode_appHandExiDocument(&iStream, &exiDoc);

    result.list.clear();
    if (err == 0)
    {
        if (!exiDoc.supportedAppProtocolReq_isUsed)
            return false;
        for(int i=0;i<exiDoc.supportedAppProtocolReq.AppProtocol.arrayLen;i++)
        {
            stAppProtocolType protType;
            char txt[64];
            exi_string_character_t* string = exiDoc.supportedAppProtocolReq.AppProtocol.array[i].ProtocolNamespace.characters;
            int len = exiDoc.supportedAppProtocolReq.AppProtocol.array[i].ProtocolNamespace.charactersLen;
            if (len>63)
            {
                len = 63;
                dbg.printf(LibCpp::enDebugLevel_Fatal, "Text conversion buffer too small!");
            }
            for (int i=0; i<len; i++)
                txt[i] = (char)string[i];
            txt[len] = 0;
            protType.protocolNamespace = txt;
            protType.versionNumberMajor = exiDoc.supportedAppProtocolReq.AppProtocol.array[i].VersionNumberMajor;
            protType.versionNumberMinor = exiDoc.supportedAppProtocolReq.AppProtocol.array[i].VersionNumberMinor;
            protType.schemaID = exiDoc.supportedAppProtocolReq.AppProtocol.array[i].SchemaID;
            protType.priority = exiDoc.supportedAppProtocolReq.AppProtocol.array[i].Priority;
            result.list.push_back(protType);
        }
    }
    return err == 0;
}

bool Iso15118::read_V2gSupportedAppProtocolRes(stV2gSupportedAppProtocolRes& result, stStreamControl& stream)
{
    cDebug dbg("Iso15118::read_V2gSupportedAppProtocolRes");

    struct appHandEXIDocument exiDoc;
    size_t pos = 0;
    bitstream_t iStream;
    iStream.size = (size_t)stream.length;
    iStream.data = (uint8_t*)stream.data;
    iStream.pos = &pos;
    iStream.buffer = 0;
    iStream.capacity = 0;

    int err = decode_appHandExiDocument(&iStream, &exiDoc);

    if (err == 0)
    {
        if (!exiDoc.supportedAppProtocolRes_isUsed)
            return false;
        result.responseCode = (enSupportedAppProtocolResponseCode)exiDoc.supportedAppProtocolRes.ResponseCode;
        if (exiDoc.supportedAppProtocolRes.SchemaID_isUsed)
            result.schemaID = exiDoc.supportedAppProtocolRes.SchemaID;
    }
    return err == 0;
}

int Iso15118::write_V2gSupportedAppProtocolReq(stV2gSupportedAppProtocolReq& data, stStreamControl& stream)
{
    cDebug dbg("Iso15118::write_V2gSupportedAppProtocolReq");

    struct appHandEXIDocument appHandReq;
    init_appHandEXIDocument(&appHandReq);
    appHandReq.supportedAppProtocolReq_isUsed = 1u;

    appHandReq.supportedAppProtocolReq.AppProtocol.arrayLen = 1;

    cExiByteArray nameSpace(data.list[0].protocolNamespace);
    nameSpace.asExiCharacters(100, appHandReq.supportedAppProtocolReq.AppProtocol.array[0].ProtocolNamespace.charactersLen, appHandReq.supportedAppProtocolReq.AppProtocol.array[0].ProtocolNamespace.characters);

    appHandReq.supportedAppProtocolReq.AppProtocol.array[0].SchemaID = data.list[0].schemaID;
    appHandReq.supportedAppProtocolReq.AppProtocol.array[0].VersionNumberMajor = data.list[0].versionNumberMajor;
    appHandReq.supportedAppProtocolReq.AppProtocol.array[0].VersionNumberMinor = data.list[0].versionNumberMinor;
    appHandReq.supportedAppProtocolReq.AppProtocol.array[0].Priority = data.list[0].priority;

    size_t pos = V2GTP_HEADER_LENGTH;
    bitstream_t oStream = outStream(stream, pos);

    if( encode_appHandExiDocument(&oStream, &appHandReq) == 0)
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
    dbg.printf(LibCpp::enDebugLevel_Fatal, "Exi encoding failed! Check send buffer size.");
    stream.length = 0;
    return 0;
}

int Iso15118::write_V2gSupportedAppProtocolRes(stV2gSupportedAppProtocolRes& data, stStreamControl& stream)
{
    cDebug dbg("Iso15118::read_V2gSupportedAppProtocolReq");

    struct appHandEXIDocument appHandResp;
    init_appHandEXIDocument(&appHandResp);

    appHandResp.supportedAppProtocolRes_isUsed = 1u;
    appHandResp.supportedAppProtocolRes.ResponseCode = appHandresponseCodeType_OK_SuccessfulNegotiation;
    appHandResp.supportedAppProtocolRes.SchemaID = data.schemaID;
    appHandResp.supportedAppProtocolRes.SchemaID_isUsed = 1u;

    size_t pos = V2GTP_HEADER_LENGTH;
    bitstream_t oStream = outStream(stream, pos);

    if( encode_appHandExiDocument(&oStream, &appHandResp) == 0)
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
    dbg.printf(LibCpp::enDebugLevel_Fatal, "Exi encoding failed! Check send buffer size.");
    stream.length = 0;
    return 0;
}

