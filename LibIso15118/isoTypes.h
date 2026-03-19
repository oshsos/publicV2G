#ifndef CISOTYPES_H
#define CISOTYPES_H

#define V2GTP_HEADER_LENGTH 8
#define V2G_SDP_REQ_LENGTH 2
#define V2G_SDP_RES_LENGTH 20

#define V2GTP_VERSION       0x01
#define V2GTP_TYPE_EXI      0x8001
#define V2GTP_TYPE_SDP_REQ  0x9000
#define V2GTP_TYPE_SDP_RES  0x9001
#define V2GTP_TYPE_V2S      0x8100
#define V2G_SECURITY_TLS    0x00
#define V2G_SECURITY_TCP    0x10
#define V2G_TRANSPORT_TCP   0x00
#define V2G_TRANSPORT_UDP   0x10

#define V2G_TIMEOUT 2000
#define V2G_POWER_TIMEOUT 200

#include <cstdint>
#include <string>

#include "openV2G/codec/EXITypes.h"
#include "openV2G/iso1/iso1EXIDatatypes.h"
//#include "openV2G/appHandshake/appHandEXIDatatypes.h"
//#include "openV2G/appHandshake/appHandEXIDatatypesDecoder.h"
//#include "openV2G/appHandshake/appHandEXIDatatypesEncoder.h"
//#include "openV2G/iso1/iso1EXIDatatypesDecoder.h"
//#include "openV2G/iso1/iso1EXIDatatypesEncoder.h"
//#include "openV2G/iso2/iso2EXIDatatypes.h"
//#include "openV2G/iso2/iso2EXIDatatypesDecoder.h"
//#include "openV2G/iso2/iso2EXIDatatypesEncoder.h"

#include "../LibCpp/HAL/Tools.h"
#include "../LibCpp/HAL/cIp.h"
#include "../LibCpp/Time/cTime.h"
#include "../LibCpp/HAL/cPacketSocket.h"

#include "openV2G/din/dinEXIDatatypes.h"

// #include "types.h"

namespace Iso15118
{

std::string v2gTypeToString(unsigned int messageType);
std::string v2gSecurityToString(unsigned int security);
std::string v2gTransportToString(unsigned int transport);

std::string iso1serviceCategoryType_toString(iso1serviceCategoryType value);
std::string iso1paymentOptionType_toString(iso1paymentOptionType value);
std::string iso1EnergyTransferModeType_toString(iso1EnergyTransferModeType value);
iso1EnergyTransferModeType iso1EnergyTransferModeType_fromString(std::string transferMode);
std::string iso1SelectedServiceType_toString(iso1SelectedServiceType service);
std::string iso1EVSEProcessingType_toString(iso1EVSEProcessingType value);
std::string iso1unitSymbolType_toString(iso1unitSymbolType type);
std::string iso1DC_EVErrorCodeType_toString(iso1DC_EVErrorCodeType code);
std::string iso1chargeProgressType_toString(iso1chargeProgressType value);
std::string iso1chargingSessionType_toString(iso1chargingSessionType value);

#pragma pack(push, 1)

typedef struct stV2gTpHeaderMsg
{
    uint8_t  protocolVersion;
    uint8_t  protocolVersionInv;
    uint16_t payloadType;
    uint32_t payloadLength;
} stV2gTpHeaderMsg;

typedef struct _stV2gSdpReqMsg
{
    uint8_t  security;
    uint8_t  transportProtocol;
} stV2gSdpReqMsg;

typedef struct stV2gSdpResMsg
{
public:
    uint16_t ipAddress[8];
    uint16_t port;
    uint8_t  security;
    uint8_t  transportProtocol;
} stV2gSdpResMsg;

#pragma pack(pop)

enum class enV2gMode
{
    Res,
    Req
};

enum class enV2gMsg : int
{
    Empty,
    // Communication setup
    Sdp,
    V2s,
    SupportedAppProtocol,
    // AC-Charging
    SessionSetup,
    ServiceDiscovery,
    PaymentServiceSelection,
    Authorization,
    ChargeParameterDiscovery,
    PowerDelivery,
    ChargingStatus,
    SessionStop,
    // DC-Charging
    CableCheck,
    PreCharge,
    CurrentDemand,
    WeldingDetection,
    // Additional services
    MeteringReceipt,
    ServiceDetail,
    PaymentDetails,
    // Decoding errors
    Unimplemented,
    ReadFailure
};

std::string enV2gMsg_toString(enV2gMsg messageType);

enum class enV2gEvState
{
    idle,
    awaitSdp,
    // AC-charging
    awaitSupportedAppProtocol,
    awaitSessionSetup,
    awaitServiceDiscovery,
    awaitPaymentServiceSelection,
    awaitAuthorization,
    awaitChargeParameterDiscovery,
    awaitPowerDelivery,
    awaitChargingStatus,
    awaitSessionStop,
    // DC-charging
    awaitCableCheck,
    awaitPreCharge,
    awaitCurrentDemand,
    awaitWeldingDetection,
    // State machine operation
    awaitDisconnection,
    failure,
    finished                 // must be placed at the end, as it is used to indicate the enum size.
};

std::string enV2gEvState_toString(enV2gEvState state);

enum class enV2gSeState
{
    idle,
    awaitSdp,
    answeredSdp,
    // AC-charging
    answeredSupportedAppProtocol,
    answeredSessionSetup,
    answeredServiceDiscovery,
    answeredPaymentServiceSelection,
    answeredAuthorization,
    answeredChargeParameterDiscovery,
    answeredPowerDelivery,
    answeredChargingStatus,
    answeredSessionStop,
    // optional
    answeredServiceDetail,
    // DC-charging
    answeredCableCheck,
    answeredPreCharge,
    answeredCurrentDemand,
    answeredWeldingDetection,
    // State machine operation
    failure,
    finished                // must be placed at the end, as it is used to indicate the enum size.
};

std::string enV2gSeState_toString(enV2gSeState state);

/**
 * @brief Input process data of the SE
 * The struct contains data used as input values to control the stack functionality by the user application.
 * The data controls the state flow and contains time varying values controlling the charging process.
 * Some of the values are periodically exchanged with the EV.
 */
struct stSeccProcessDataInput
{
    bool            enable;             ///< Enables the start of a charging session or forces the charging session to stop.
    bool            authorized;         ///< Indicates a successful authorization. Must be 'true' in case no authorization of the vehicle is required.
    float           currentLimit;       ///< The actual current limit of the battery e.g. in almoust full state.
    float           _voltageLimit;      ///< (The sense is unclear - An exeeding voltage must be avoided by the currentLimit) Time varying limit for the voltage.
    float           _powerLimit;        ///< (The sense is unclear - An exeeding power must be avoided by the currentLimit) Time varying limit for the power.

    stSeccProcessDataInput()
    {
        clear();
    }

    void clear()
    {
        enable = true;
        authorized = true;
        _voltageLimit = 0;
        _powerLimit = 0;
    }
};

/**
 * @brief Output process data of the SE
 * The struct contains time varying data of the stack occurring during the charging process.
 * These are states and physical values.
 * Some of the values are periodically exchanged with the EV.
 */
struct stSeccProcessDataOutput
{
    enV2gSeState    v2gState;           ///< Stack operational state
    float           demandedCurrent;    ///< Output current demanded by the EV
    float           actualCurrent;      ///< Actual output current of the SE
    float           actualVoltage;      ///< Actual output voltage of the SE

    stSeccProcessDataOutput()
    {
        clear();
    }

    void clear()
    {
        v2gState = enV2gSeState::idle;
    }
};

/**
 * @brief Parameter data of the SE
 * The EV uses this data struct to collect all information gained during the communication sequence.
 * These values can be treated as output values from the SE within the EV and
 * as configuration input values within the SE.
 */
struct stSeccInfo
{   // Charging session
    LibCpp::cByteArray                      seccId;                     ///< Supply equipment identifyer
    std::vector<iso1EnergyTransferModeType> energyTransferModes;        ///< List of energy transfer types like AC charging or DC charging the SE physically supports.
    bool                                    freeService;                ///< Indication if the power delivery is free of charge (thus free of contract identification need).
    std::vector<iso1paymentOptionType>      paymentOptions;             ///< List of supported payment methods
    iso1serviceCategoryType                 chargeService;              ///< The type of service(s) the SE provides. The main service is EV charging.
    unsigned int                            chargeServiceId;            ///< Identifier used for the chargeService

    // Physical values
    float                                   maxVoltage;                 ///< Maximum physical voltage provided by the SE
    float                                   nominalVoltage;             ///< Nominal voltage of the SE
    float                                   maxCurrent;                 ///< Maximum physical current provided by the SE
    float                                   minCurrent;                 ///< Minimum current required by the SE
    float                                   maxPower;                   ///< Maximum power the SE is able to deliver

    // Charging process
    LibCpp::cByteArray                      sessionId;                  ///< Identifyer of the charging session chosen by the SE.
    LibCpp::stIpAddress                     ipAddress;                  ///< IP Address used by the SECC
    unsigned char                           macAddress[ETHER_ADDR_LEN]; ///< MAC Address of the SECC
    unsigned int                            schemaId;                   ///< id of the chosen ISO 15118 dialect version.
    bool                                    authorized;                 ///< Authorization process has successfully finished (used on EV side).
    iso1EVSEProcessingType                  processingState;            ///< Indicator, if a certain long lasting process like identification or isolation measurement has finished or not.

    stSeccInfo()
    {
        clear();
    }

    void clear()
    {
        seccId.set("ZZ00001", 7);
        seccId.clear();
        energyTransferModes.clear();
        freeService = true;
        paymentOptions.clear();
        chargeService = (iso1serviceCategoryType)0;
        chargeServiceId = 0;
        maxVoltage = 0;
        nominalVoltage = 0;
        maxCurrent = 0;
        minCurrent = 0;
        maxPower = 0;
        sessionId.set(0x0102030405060708, true);
        ipAddress = LibCpp::stIpAddress();
        for (int i=0; i<ETHER_ADDR_LEN; i++) macAddress[i] = 0;
        schemaId = -1;
        processingState = iso1EVSEProcessingType_Finished;
        authorized = false;
    }

    std::string toString()
    {
        std::string out;
        out += "SECC ID:              " + seccId.asString() +"\n";
        for (iso1EnergyTransferModeType& mode : energyTransferModes)
            out += "Energy Transfer Mode: " + iso1EnergyTransferModeType_toString(mode) +"\n";
        out += "Service:              " + std::string(freeService ? "free" : "payment") + "\n";
        for (iso1paymentOptionType& option : paymentOptions)
            out += "Payment option:       " + iso1paymentOptionType_toString(option) +"\n";
        out += "Charge service:       " + iso1serviceCategoryType_toString(chargeService) +"\n";
        out += "Charge service id:    " + std::to_string(chargeServiceId) +"\n";

        out += "Max voltage:          " + std::to_string(maxVoltage) +"\n";
        out += "Nominal voltage:      " + std::to_string(nominalVoltage) +"\n";
        out += "Max current:          " + std::to_string(maxCurrent) +"\n";
        out += "Min current:          " + std::to_string(minCurrent) +"\n";
        out += "Max power:            " + std::to_string(maxPower) +"\n";


        out += "Session id:           " + sessionId.toString() +"\n";
        out += "Ip address:           " + ipAddress.toString() +"\n";
        out += "MAC address:          " + LibCpp::ByteArrayToString(macAddress, ETHER_ADDR_LEN) +"\n";
        out += "Schema id:            " + std::to_string(schemaId) +"\n";
        out += "Authorized:           " + std::string(authorized ? "authorized" : "unknown") +"\n";
        return out;
    }
};

/**
 * @brief Process input data of the EV
 * The data set is sent by the EV application program to the stack.
 * This data is partly exchanged between the EV and the SE periodically and contains time varying data.
 */
struct stEvccProcessDataInput
{
    bool        enable;             ///< Enables the initiation of a charging process or forces a running charging process to finish
    float       currentDemand;      ///< Actual target value for the SE power electronics
    float       _currentLimit;      ///< (The sense is unclear - The SE shall supply currentDemand or below if not possible) The actual current limit of the battery e.g. in almoust full state
    float       _voltageLimit;      ///< (The sense is unclear - The EV limits the battery voltage itself by currentDemand) Time varying limit for the voltage
    float       _powerLimit;        ///< (The sense is unclear - power is defined by currentDemand) Time varying limit for the power

    stEvccProcessDataInput()
    {
        clear();
    }

    void clear()
    {
        enable = true;
        currentDemand = 10;
        _currentLimit = 0;
        _voltageLimit = 0;
        _powerLimit = 0;
    }
};

/**
 * @brief Process output data of the EV
 * The data set is sent by the EV application program to the stack.
 * This data is partly exchanged between the EV and the SE periodically and contains time varying data.
 */
struct stEvccProcessDataOutput
{
    enV2gEvState    v2gState;       ///< Stack operational state
    float           actualCurrent;  ///< Actual current delivered by the SE

    stEvccProcessDataOutput()       ///< Initializer
    {
        clear();
    }

    void clear()                    ///< Set struct to initial values
    {
        v2gState = enV2gEvState::idle;
        actualCurrent = 0;
    }
};

/**
 * @brief Parameter data of the EV
 * The SE uses this data struct to collect all information gained during the communication sequence.
 * These values can be treated as output values within the SE and
 * as configuration input values within the EV.
 */
struct stEvccInfo
{
    // Charging session
    LibCpp::cByteArray          evccId;                     ///< Vehicle identification string
    iso1EnergyTransferModeType  energyTransferMode;         ///< Energy transfer mode used for this session
    float                       energyCapacity;             ///< Battery charge capacity.
    float                       energyRequest;              ///< Requested energy for the current charging session / Wh.
    uint32_t                    departureTime;              ///< Expected time the charging process may durate.
    iso1paymentOptionType       paymentOption;              ///< Payment option used for this session

    // Technical
    float                       maxVoltage;                 ///< Physical voltage limit of the EV battery or power electronics respectively.
    float                       maxCurrent;                 ///< Physical current limit of the EV battery or power electronics respectively.
    float                       minCurrent;                 ///< Minimum physical current required by EV battery or power electronics respectively.
    float                       maxPower;                   ///< Physical power limit of the EV battery or power electronics respectively.
    float                       targetVoltage;              ///< Voltage to be reached at the end of the charging process (send as process data but it is a non time varyant parameter.

    // Charging process
    LibCpp::stIpAddress         ipAddress;                  ///< IP Address used for the session
    unsigned char               macAddress[ETHER_ADDR_LEN]; ///< IP Address of the EVCC
    unsigned int                schemaId;                   ///< id of the chosen ISO 15118 dialect version.
    float                       memorizedVoltage;           ///< (EVCC operation only) Voltage of the SECC transmitted to the EVCC during PreCharge.

    stEvccInfo(bool preset = false)
    {
        clear();
        if (preset)
        {
            uint8_t id[6] = {0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
            evccId.set((char*)id, 6);
            energyTransferMode = iso1EnergyTransferModeType_AC_three_phase_core;
            energyCapacity   = 10000;
            energyRequest    = 5000;
            departureTime    = 86400;    // 24h minimum value, if the schedule list contains just one entry
            paymentOption    = iso1paymentOptionType_ExternalPayment;
            maxVoltage       = 400;
            maxCurrent       = 32;
            minCurrent       = 0;
            maxPower         = 20000;
            targetVoltage    = 400;
            memorizedVoltage = 0;
            schemaId         = 10;
            // minCurrent = 5;          // riseV2G
            // energyRequest = 5000;    // riseV2G
            // departureTime = 7200;    // riseV2G
        }
    }

    void clear()
    {
        evccId.clear();
        energyTransferMode = iso1EnergyTransferModeType_AC_three_phase_core;
        energyCapacity = 0;
        energyRequest = 0;
        departureTime = 0;
        paymentOption   = iso1paymentOptionType_ExternalPayment;
        maxVoltage = 0;
        maxCurrent = 0;
        minCurrent = 0;
        maxPower = 0;
        targetVoltage = 0;
        ipAddress = LibCpp::stIpAddress();
        for (int i=0; i<ETHER_ADDR_LEN; i++) macAddress[i] = 0;
        schemaId = 0;
        memorizedVoltage = 0;
    }

    std::string toString()
    {
        std::string out;
        LibCpp::cTime departure = LibCpp::cTime::set(departureTime);

        out += "EVCC ID:              " + evccId.toString() + " (" + std::to_string(evccId.asUInt64(true)) + ")\n";
        out += "Energy Transfer Mode: " + iso1EnergyTransferModeType_toString(energyTransferMode) +"\n";

        out += "Energy Capacity / Wh: " + std::to_string(energyCapacity) +"\n";
        out += "Energy Request  / Wh: " + std::to_string(energyRequest) +"\n";
        out += "Departure time      : " + departure.toString() +"\n";
        out += "Payment option      : " + iso1paymentOptionType_toString(paymentOption) +"\n";
        out += "Max voltage     / V : " + std::to_string(maxVoltage) +"\n";
        out += "Max current:    / A : " + std::to_string(maxCurrent) +"\n";
        out += "Min current:    / A : " + std::to_string(minCurrent) +"\n";
        out += "Max power       / W : " + std::to_string(maxPower) +"\n";
        out += "Target voltage  / V : " + std::to_string(targetVoltage) +"\n";
        out += "Ip address:           " + ipAddress.toString() +"\n";
        out += "MAC address:          " + LibCpp::ByteArrayToString(macAddress, ETHER_ADDR_LEN) +"\n";
        out += "Schema id:            " + std::to_string(schemaId) +"\n";
        out += "Initial voltage / V : " + std::to_string(memorizedVoltage) +"\n";
        return out;
    }
};

enum class enV2gResponse
{
    OK,
    OK_NewSessionEstablished,
    OK_OldSessionJoined,
    OK_CertificateExpiresSoon,
    FAILED,
    FAILED_SequenceError,
    FAILED_ServiceIDInvalid,
    FAILED_UnknownSession,
    FAILED_ServiceSelectionInvalid,
    FAILED_PaymentSelectionInvalid,
    FAILED_CertificateExpired,
    FAILED_SignatureError,
    FAILED_NoCertificateAvailable,
    FAILED_CertChainError,
    FAILED_ChallengeInvalid,
    FAILED_ContractCanceled,
    FAILED_WrongChargeParameter,
    FAILED_PowerDeliveryNotApplied,
    FAILED_TariffSelectionInvalid,
    FAILED_ChargingProfileInvalid,
    FAILED_MeteringSignatureNotValid,
    FAILED_NoChargeServiceSelected,
    FAILED_WrongEnergyTransferMode,
    FAILED_ContactorError,
    FAILED_CertificateNotAllowedAtThisEVSE,
    FAILED_CertificateRevoked
    // From iso2:
    //FAILED_CertificateNotYetValid
    //FAILED_IsolationFault,
    //FAILED_SignatureError,
    //OK_IsolationValid,
    //OK_IsolationWarning,
    //WARNING_CertificateExpired,
    //WARNING_NoCertificateAvailable,
    //WARNING_CertValidationError,
    //WARNING_CertVerificationError,
};

std::string enV2gResponse_toString(enV2gResponse response);

enum enV2gNotification
{
    iso1EVSENotificationType_None,
    iso1EVSENotificationType_ReNegotiation,
    iso1EVSENotificationType_StopCharging
};

std::string enV2gNotification_toString(enV2gNotification type);

enum class enIsolationLevel{
    invalid,
    valid,
    warning,
    fault,
    noIMD = 4
};

std::string enIsolationLevel_toString(enIsolationLevel level);

enum class enDcEvseStatusCode                                                  // eqal to dinDC_EVSEStatusCodeType, iso1DC_EVSEStatusCodeType
{
    NotReady,
    Ready,
    Shutdown,
    UtilityInterruptEvent,
    IsolationMonitoringActive,
    EmergencyShutdown,
    Malfunction,
    Reserved_8,
    Reserved_9,
    Reserved_A,
    Reserved_B,
    Reserved_C
};

std::string enDcEvseStatusCode_toString(enDcEvseStatusCode code);

class cExiByteArray : public LibCpp::cByteArray
{
public:
    cExiByteArray(unsigned int size = 0, const char* sourceData = 0);
    cExiByteArray(const int64_t source, bool bigEndian = true);
    cExiByteArray(const std::string source);
    cExiByteArray(const cByteArray& source);

    cExiByteArray(uint16_t charactersLen, exi_string_character_t* characters);
    cExiByteArray(LibCpp::cByteArray& byteArray);
    void setFromExiCharacters(uint16_t charactersLen, exi_string_character_t* characters);
    void asExiCharacters(uint16_t maxLen, uint16_t& charactersLen, exi_string_character_t* characters);
};

struct stAppProtocolType
{
    std::string     protocolNamespace;
    unsigned int    versionNumberMajor;
    unsigned int    versionNumberMinor;
    unsigned int    schemaID;
    unsigned int    priority;

    stAppProtocolType();
    std::string toString();
};

struct stDcEvStatus
{
    iso1DC_EVErrorCodeType errorCode;   // = dinDC_EVErrorCodeType
    int8_t ressSOC;
    int    ready;

    stDcEvStatus();
    std::string toString();
};

struct stExiPysicalValue
{
    int16_t value;
    int8_t  exponent;
    iso1unitSymbolType unit;

    stExiPysicalValue();
    void read(iso1PhysicalValueType* pValue);
    void write(iso1PhysicalValueType* pValue);
    void read(dinPhysicalValueType* pValue);
    void write(dinPhysicalValueType* pValue);
    float toFloat();
    void fromFloat(float value, iso1unitSymbolType unit = iso1unitSymbolType_s, int8_t exponent = 0);
    std::string toString();
};

iso1unitSymbolType to_iso1unitSymbolType(dinunitSymbolType symbol);

dinunitSymbolType to_dinunitSymbolType(iso1unitSymbolType symbol);

/**
 * @brief Abstraction of struct iso1PMaxScheduleEntryType
 */
struct stPmaxSchedule
{
    stExiPysicalValue maxP;
    uint32_t relativeStartTime;
    uint32_t relativeDuration;

    stPmaxSchedule();
    std::string toString();
};

struct  stPmaxSchedules
{
    std::vector<stPmaxSchedule> list;
};

struct stSaScheduleTuple
{
    uint16_t id;
    stPmaxSchedules maxPSchedules;

    stSaScheduleTuple();
    std::string toString();
};

struct  stSaSchedules
{
    std::vector<stSaScheduleTuple> list;

    std::string toString();
};

struct stV2gTpHeader
{
public:
    unsigned int payloadLength;
    unsigned int payloadType;
    unsigned int protocolVersion;

    stV2gTpHeader();
    stV2gTpHeader(unsigned int type, unsigned int len = 0, unsigned int version = V2GTP_VERSION);
    void set(unsigned int type, unsigned int len = 0, unsigned int version = V2GTP_VERSION);

    std::string toString();
};

struct stV2gSdpReq
{
public:
    unsigned int security;
    unsigned int transportProtocol;

    stV2gSdpReq();
    std::string toString();
};

struct stV2gSdpRes
{
public:
    LibCpp::stIpAddress serverIpAddress;
    unsigned int security;
    unsigned int transportProtocol;

    std::string toString();
};

struct stV2gSupportedAppProtocolReq
{
    std::vector<stAppProtocolType> list;

    stV2gSupportedAppProtocolReq();
    std::string toString();
};

enum enSupportedAppProtocolResponseCode
{
    enSupportedAppProtocolResponseCode_Success,
    enSupportedAppProtocolResponseCode_MinorDeviations,
    enSupportedAppProtocolResponseCode_Failure
};

std::string enSupportedAppProtocolResponseCode_toString(enSupportedAppProtocolResponseCode code);

struct stV2gSupportedAppProtocolRes
{
    enSupportedAppProtocolResponseCode responseCode;
    unsigned int schemaID;

    stV2gSupportedAppProtocolRes();
    std::string toString();
};

struct stV2gHeader
{
    stV2gTpHeader tpHeader;
    enV2gMsg messageType;
    enV2gMode messageMode;
    LibCpp::cByteArray sessionId;

    stV2gHeader(enV2gMsg v2gMsgType = enV2gMsg::ReadFailure, enV2gMode v2gMsgMode = enV2gMode::Req);
    std::string toString();
};

struct stV2gSessionSetupReq
{
    stV2gHeader header;

    LibCpp::cByteArray evccId;    // maximum of 6 bytes

    stV2gSessionSetupReq();
    std::string toString();
};

struct stV2gSessionSetupRes
{
    stV2gHeader         header;
    enV2gResponse       responseCode;

    LibCpp::cByteArray  seccId;           ///< The slightly misleading naming evseID as used in the standard has been changed here to be consistend to the session setup request.
    int64_t             seccTimeStamp;

    stV2gSessionSetupRes();
    std::string toString();
};

struct stV2gServiceDiscoveryReq
{
    stV2gHeader         header;

    iso1serviceCategoryType  serviceCategory;   // same as dinserviceCategoryType

    stV2gServiceDiscoveryReq();
    std::string toString();
};

struct stV2gServiceDiscoveryRes
{
    stV2gHeader         header;
    enV2gResponse       responseCode;

    std::vector<iso1paymentOptionType>  paymentOptions;     // = dinPaymentOptionType
    unsigned int        chargeServiceId;
    bool                chargeServiceFree;
    iso1serviceCategoryType chargeServiceCategory;          // = dinserviceCategoryType
    std::vector<iso1EnergyTransferModeType> energyTransferModes;        // = dinEVSESupportedEnergyTransferType von 0-4, 5->übertragbar, >5 nur in din vmtl. ungenutzt

    stV2gServiceDiscoveryRes();
    std::string toString();
};

struct stV2gPaymentServiceSelectionReq
{
    stV2gHeader         header;

    iso1paymentOptionType selectedPaymentOption;            // = dinPaymentOptionType
    std::vector<iso1SelectedServiceType> selectedServices;  // = sinSelectedServiceType (struct)

    stV2gPaymentServiceSelectionReq();
    std::string toString();
};

struct stV2gPaymentServiceSelectionRes
{
    stV2gHeader         header;
    enV2gResponse       responseCode;

    stV2gPaymentServiceSelectionRes();
    std::string toString();
};

struct stV2gAuthorizationReq
{
    stV2gHeader         header;

    LibCpp::cByteArray genChallenge;
    LibCpp::cByteArray id;

    stV2gAuthorizationReq();
    std::string toString();
};

struct stV2gAuthorizationRes
{
    stV2gHeader         header;
    enV2gResponse       responseCode;

    iso1EVSEProcessingType evseProcessing;  // more values but compatible with dinEVSEProcessingType

    stV2gAuthorizationRes();
    std::string toString();
};

struct stV2gChargeParameterDiscoveryReq
{
    iso1EnergyTransferModeType transferMode;    // = dinEVRequestedEnergyTransferType ~= dinEVSESupportedEnergyTransferType

    stV2gHeader         header;

    stExiPysicalValue   maxVoltage;
    stExiPysicalValue   maxCurrent;
    stExiPysicalValue   energyRequest;   ///< (opt@dc)
    uint32_t            departureTime;   ///< (opt)
    stExiPysicalValue   minCurrent;      ///< ac-Parameter
    stDcEvStatus        dcEvStatus;      ///< dc-Parameter
    stExiPysicalValue   maxPower;        ///< (opt) dc-Parameter
    stExiPysicalValue   energyCapacity;  ///< (opt) dc-Parameter
    int8_t              bulkSOC;         ///< (opt) dc-Parameter
    int8_t              fullSOC;         ///< (opt) dc-Parameter
    uint16_t            maxEntriesSAScheduleTuple;

    stV2gChargeParameterDiscoveryReq();
    std::string toString();
};

struct stV2gChargeParameterDiscoveryRes
{
    iso1EnergyTransferModeType transferMode; // used to decide whether AC-structs or DC-structs need to be filled

    stV2gHeader         header;
    enV2gResponse       responseCode;

    iso1EVSEProcessingType evseProcessing;
    // iso1EVSEChargeParameterType evseChargeParameter; // no content

    stExiPysicalValue maxCurrent;       //- AC/DC: AC_EVSEChargeParameter
    stExiPysicalValue nominalVoltage;   //- AC: AC_EVSEChargeParameter
    stExiPysicalValue minCurrent;       //- DC_EVSEChargeParameter
    stExiPysicalValue maxVoltage;       //- DC_EVSEChargeParameter
    stExiPysicalValue maxPower;         //- DC_EVSEChargeParameter
    stExiPysicalValue minVoltage;       //- DC_EVSEChargeParameter
    stExiPysicalValue maxPeakCurrentRipple; //- DC_EVSEChargeParameter
    int rcd;                            //- AC_EVSEChargeParameter.AC_EVSEStatus

    enDcEvseStatusCode evseStatusCode;                 //- DC_EVSEChargeParameter.DC_EVSEStatus
    enIsolationLevel  evseIsolationStatus;            //- (opt) DC_EVSEChargeParameter.DC_EVSEStatus

    bool saScheduleListUsed;
    bool saSchedulesUsed;

    enV2gNotification evseNotification; //- AC/DC: AC_EVSEChargeParameter.AC_EVSEStatus iso1EVSENotificationType
    uint16_t notificationMaxDelay;      //- AC/DC: AC_EVSEChargeParameter.AC_EVSEStatus
    //    iso1SAScheduleListType saScheduleList;
    //    iso1SASchedulesType saSchedules;



    stV2gChargeParameterDiscoveryRes();
    std::string toString();
};

struct stV2gPowerDeliveryReq
{
    iso1EnergyTransferModeType transferMode; // used to decide whether AC-structs or DC-structs need to be filled

    stV2gHeader             header;

    iso1chargeProgressType  chargeProgress;
    uint8_t                 saScheduleTupleId;
    bool                    chargingProfile_isUsed;             // (opt)
    bool                    evPowerDeliveryParameter_isUsed;    // (no content)
    iso1DC_EVStatusType     evStatus;                           // (DC)
    int                     chargingComplete;                   // (DC)
    int                     bulkChargingComplete;               // (opt DC)

    stV2gPowerDeliveryReq();
    std::string toString();
};

struct stV2gPowerDeliveryRes
{
    iso1EnergyTransferModeType transferMode;    // used to decide whether AC-structs or DC-structs need to be filled

    stV2gHeader         header;
    enV2gResponse       responseCode;

    enV2gNotification evseNotification;         // EVSEStatus, AC_EVSEStatus, DC_EVSEStatus
    uint16_t notificationMaxDelay;              // EVSEStatus, AC_EVSEStatus, DC_EVSEStatus
    int rcd;                                    // AC AC_EVSEStatus
    enDcEvseStatusCode evseStatusCode;                         // DC DC_EVSEStatus
    enIsolationLevel evseIsolationStatus;                    // (opt) DC DC_EVSEStatus

    stV2gPowerDeliveryRes();
    std::string toString();
};

struct stV2gChargingStatusReq
{
    stV2gHeader         header;

    int                 noContent;

    stV2gChargingStatusReq();
    std::string toString();
};

struct stV2gChargingStatusRes
{
    iso1EnergyTransferModeType transferMode;    // used to decide whether AC-structs or DC-structs need to be filled

    stV2gHeader         header;
    enV2gResponse       responseCode;

    LibCpp::cByteArray  seccId;
    uint16_t            saScheduleTupleId;
    stExiPysicalValue   maxCurrent;

    enV2gNotification evseNotification;         // EVSEStatus, AC_EVSEStatus, DC_EVSEStatus
    uint16_t notificationMaxDelay;              // EVSEStatus, AC_EVSEStatus, DC_EVSEStatus
    int rcd;                                    // AC AC_EVSEStatus

    bool meterInfo_isUsed;
    bool receiptRequired_isUsed;

    stV2gChargingStatusRes();
    std::string toString();
};

struct stV2gSessionStopReq
{
    stV2gHeader         header;

    iso1chargingSessionType stopType;

    stV2gSessionStopReq();
    std::string toString();
};

struct stV2gSessionStopRes
{
    stV2gHeader         header;
    enV2gResponse       responseCode;

    stV2gSessionStopRes();
    std::string toString();
};

struct stV2gCableCheckReq
{
    stV2gHeader             header;

    iso1DC_EVErrorCodeType  evErrorCode;    // = dinDC_EVErrorCodeType
    uint8_t                 evRessSoc;
    bool                    evReady;

    stV2gCableCheckReq();
    std::string toString();
};

struct stV2gCableCheckRes
{
    stV2gHeader             header;
    enV2gResponse           responseCode;

    enDcEvseStatusCode      evseStatusCode;
    enV2gNotification       evseNotification;
    uint16_t                notificationMaxDelay;
    enIsolationLevel        evseIsolationStatus;
    iso1EVSEProcessingType  evseProcessing;

    stV2gCableCheckRes();
    std::string toString();
};

struct stV2gPreChargeReq
{
    stV2gHeader             header;

    stExiPysicalValue       evTargetVoltage;
    stExiPysicalValue       evTargetCurrent;
    iso1DC_EVErrorCodeType  evErrorCode;
    uint8_t                 evRessSoc;
    bool                    evReady;

    stV2gPreChargeReq();
    std::string toString();
};

struct stV2gPreChargeRes
{
    stV2gHeader             header;
    enV2gResponse           responseCode;

    stExiPysicalValue       evsePresentVoltage;
    enDcEvseStatusCode      evseStatusCode;
    enV2gNotification       evseNotification;
    uint16_t                notificationMaxDelay;
    enIsolationLevel        evseIsolationStatus;

    stV2gPreChargeRes();
    std::string toString();
};

struct stV2gCurrentDemandReq
{
    stV2gHeader             header;

    stExiPysicalValue       evTargetCurrent;
    stExiPysicalValue       evTargetVoltage;
    stExiPysicalValue       evCurrentLimit;
    stExiPysicalValue       evVoltageLimit;
    stExiPysicalValue       evPowerLimit;
    uint8_t                 evRessSoc;

    bool                    chargingComplete;
    bool                    bulkChargingComplete;

    stExiPysicalValue       remainingTimeToBulkSoC;
    stExiPysicalValue       remainingTimeToFullSoC;

    bool                    evReady;
    iso1DC_EVErrorCodeType  evErrorCode;


    stV2gCurrentDemandReq();
    std::string toString();
};

struct stV2gCurrentDemandRes
{
    stV2gHeader             header;
    enV2gResponse           responseCode;

    LibCpp::cByteArray      seccId;
    stExiPysicalValue       evsePresentCurrent;
    stExiPysicalValue       evsePresentVoltage;
    stExiPysicalValue       evseCurrentLimit;
    stExiPysicalValue       evseVoltageLimit;
    stExiPysicalValue       evsePowerLimit;

    enDcEvseStatusCode      evseStatusCode;
    enV2gNotification       evseNotification;
    uint16_t                notificationMaxDelay;
    enIsolationLevel        evseIsolationStatus;

    bool                    evseCurrentLimitAchieved;
    bool                    evseVoltageLimitAchieved;
    bool                    evsePowerLimitAchieved;

    uint8_t                 saScheduleTupleId;
    bool                    meterInfo_isUsed;
    bool                    receiptRequired_isUsed;

    stV2gCurrentDemandRes();
    std::string toString();
};

struct stV2gWeldingDetectionReq
{
    stV2gHeader             header;

    iso1DC_EVErrorCodeType  evErrorCode;
    uint8_t                 evRessSoc;
    bool                    evReady;

    stV2gWeldingDetectionReq();
    std::string toString();
};

struct stV2gWeldingDetectionRes
{
    stV2gHeader             header;
    enV2gResponse           responseCode;

    stExiPysicalValue       evsePresentVoltage;
    enDcEvseStatusCode      evseStatusCode;
    enV2gNotification       evseNotification;
    uint16_t                notificationMaxDelay;
    enIsolationLevel        evseIsolationStatus;

    stV2gWeldingDetectionRes();
    std::string toString();
};

struct stV2gServiceDetailReq
{
    stV2gHeader         header;

    unsigned int  serviceId;

    stV2gServiceDetailReq();
    std::string toString();
};

struct stV2gServiceDetailRes
{
    stV2gHeader         header;
    enV2gResponse       responseCode;

    unsigned int  serviceId;

    stV2gServiceDetailRes();
    std::string toString();
};

typedef struct stV2gMessage
{
    enV2gMode   mode;
    enV2gMsg    type;
    union       content
    {
        union   req
        {
            // Communication setup
            stV2gSdpReq                          sdp;
            stV2gSupportedAppProtocolReq        supportedAppProtocol;
            // Iso15118 V2G messages
            // AC charging
            stV2gSessionSetupReq                sessionSetup;
            stV2gServiceDiscoveryReq            serviceDiscovery;
            stV2gPaymentServiceSelectionReq     paymentServiceSelection;
            stV2gAuthorizationReq               authorization;
            stV2gChargeParameterDiscoveryReq    chargeParameterDiscovery;
            stV2gPowerDeliveryReq               powerDelivery;
            stV2gChargingStatusReq              chargingStatus;
            stV2gSessionStopReq                 sessionStop;
            // DC-Charging
            stV2gCableCheckReq                  cableCheck;
            stV2gPreChargeReq                   preCharge;
            stV2gCurrentDemandReq               currentDemand;
            stV2gWeldingDetectionReq            weldingDetection;
            // Additional services
            stV2gServiceDetailReq               serviceDetail;
            // stV2gMeteringReceiptReq          meteringReceipt;
            // stV2gPaymentDetailReq            paymentDetails;

            req() : sdp{} {};
            req(req& source){(void)source;};
            ~req() {};
        } req;
        union res
        {
            // Communication setup
            stV2gSdpRes                          sdp;
            stV2gSessionSetupRes                sessionSetup;
            // Iso15118 V2G messages
            // AC charging
            stV2gSupportedAppProtocolRes        supportedAppProtocol;
            stV2gServiceDiscoveryRes            serviceDiscovery;
            stV2gPaymentServiceSelectionRes     paymentServiceSelection;
            stV2gAuthorizationRes               authorization;
            stV2gChargeParameterDiscoveryRes    chargeParameterDiscovery;
            stV2gPowerDeliveryRes               powerDelivery;
            stV2gChargingStatusRes              chargingStatus;
            stV2gSessionStopRes                 sessionStop;
            // DC-Charging
            stV2gCableCheckRes                  cableCheck;
            stV2gPreChargeRes                   preCharge;
            stV2gCurrentDemandRes               currentDemand;
            stV2gWeldingDetectionRes            weldingDetection;
            // Additional services
            stV2gServiceDetailRes               serviceDetail;
            // stV2gMeteringReceiptRes          meteringReceipt;
            // stV2gPaymentDetailRes            paymentDetails;

            res() {};
            res(res& source) {(void)source;};
            ~res() {};
        } res;

        content() : req{} {};
        content(content& source) {(void)source;};
        ~content() {};
    } content;

//    stV2gMessage() : content{} {};
//    ~stV2gMessage();
    stV2gMessage();
    stV2gMessage(stV2gMessage& source);
    stV2gMessage(enV2gMsg type, enV2gMode mode = enV2gMode::Req);
    ~stV2gMessage();
    void set(enV2gMsg type, enV2gMode mode = enV2gMode::Req, stV2gMessage* pSource = nullptr);
    void clear();
} stV2gMessage;

struct stStreamControl
{
    char* data;
    unsigned int length;

    stStreamControl(char* stream, unsigned int size);
    void set(char* stream, unsigned int size);
    void move(unsigned int bytes);
    std::string toString();
};

bitstream_t inStream(stStreamControl stream, size_t& pos);
bitstream_t outStream(stStreamControl stream, size_t& pos);

bool read_V2gTpHeader(stV2gTpHeader& pResult, stStreamControl& stream);
bool read_V2gSdpReq(stV2gSdpReq& result, stV2gTpHeader& headerCheck, stStreamControl& stream);
bool read_V2gSdpRes(stV2gSdpRes& result, stV2gTpHeader& headerCheck, stStreamControl& stream);
bool read_V2gSupportedAppProtocolReq(stV2gSupportedAppProtocolReq& result, stStreamControl& stream);
bool read_V2gSupportedAppProtocolRes(stV2gSupportedAppProtocolRes& result, stStreamControl& stream);

int  write_V2gTpHeader(stV2gTpHeader& data, stStreamControl& stream);
int  write_V2gSdpReq(stV2gSdpReq& data, stStreamControl& stream);
int  write_V2gSdpRes(stV2gSdpRes& data, stStreamControl& stream);
int  write_V2s(stStreamControl& stream);
int  write_V2gSupportedAppProtocolReq(stV2gSupportedAppProtocolReq& data, stStreamControl& stream);
int  write_V2gSupportedAppProtocolRes(stV2gSupportedAppProtocolRes& data, stStreamControl& stream);

}

#endif
