#ifndef CEXICODEC_H
#define CEXICODEC_H

#include "isoTypes.h"
#include "../LibCpp/HAL/cDebug.h"

namespace Iso15118
{

class cExiCodec
{
public:
    cExiCodec();

    int          encode(char* buffer, unsigned int bufferSize, stV2gMessage& message);
    void         decode(stV2gMessage& message, char* pBuffer, int length, bool expectSuppAppProt = false);

    virtual bool read_V2gHeader(stV2gHeader& result, stStreamControl& stream, bool expectSuppAppProt = false) = 0;
    static  bool read_V2gSdpReq(stV2gSdpReq& result, stV2gTpHeader& headerCheck, stStreamControl& stream);
    static  bool read_V2gSdpRes(stV2gSdpRes& result, stV2gTpHeader& headerCheck, stStreamControl& stream);
    static  bool read_V2gSupportedAppProtocolReq(stV2gSupportedAppProtocolReq& result, stStreamControl& stream);
    static  bool read_V2gSupportedAppProtocolRes(stV2gSupportedAppProtocolRes& result, stStreamControl& stream);
    virtual bool read_V2gSessionSetupReq(stV2gSessionSetupReq& result, stV2gHeader& headerCheck) = 0;
    virtual bool read_V2gSessionSetupRes(stV2gSessionSetupRes& result, stV2gHeader& headerCheck) = 0;
    virtual bool read_V2gServiceDiscoveryReq(stV2gServiceDiscoveryReq& result, stV2gHeader& headerCheck) = 0;
    virtual bool read_V2gServiceDiscoveryRes(stV2gServiceDiscoveryRes& result, stV2gHeader& headerCheck) = 0;
    virtual bool read_V2gPaymentServiceSelectionReq(stV2gPaymentServiceSelectionReq& result, stV2gHeader& headerCheck) = 0;
    virtual bool read_V2gPaymentServiceSelectionRes(stV2gPaymentServiceSelectionRes& result, stV2gHeader& headerCheck) = 0;
    virtual bool read_V2gAuthorizationReq(stV2gAuthorizationReq& result, stV2gHeader& headerCheck) = 0;
    virtual bool read_V2gAuthorizationRes(stV2gAuthorizationRes& result, stV2gHeader& headerCheck) = 0;
    virtual bool read_V2gChargeParameterDiscoveryReq(stV2gChargeParameterDiscoveryReq& result, stV2gHeader& headerCheck) = 0;
    virtual bool read_V2gChargeParameterDiscoveryRes(stV2gChargeParameterDiscoveryRes& result, stV2gHeader& headerCheck) = 0;
    virtual bool read_V2gPowerDeliveryReq(stV2gPowerDeliveryReq& result, stV2gHeader& headerCheck) = 0;
    virtual bool read_V2gPowerDeliveryRes(stV2gPowerDeliveryRes& result, stV2gHeader& headerCheck) = 0;
    virtual bool read_V2gChargingStatusReq(stV2gChargingStatusReq& result, stV2gHeader& headerCheck) = 0;
    virtual bool read_V2gChargingStatusRes(stV2gChargingStatusRes& result, stV2gHeader& headerCheck) = 0;
    virtual bool read_V2gSessionStopReq(stV2gSessionStopReq& result, stV2gHeader& headerCheck) = 0;
    virtual bool read_V2gSessionStopRes(stV2gSessionStopRes& result, stV2gHeader& headerCheck) = 0;
    virtual bool read_V2gCableCheckReq(stV2gCableCheckReq& result, stV2gHeader& headerCheck) = 0;
    virtual bool read_V2gCableCheckRes(stV2gCableCheckRes& result, stV2gHeader& headerCheck) = 0;
    virtual bool read_V2gPreChargeReq(stV2gPreChargeReq& result, stV2gHeader& headerCheck) = 0;
    virtual bool read_V2gPreChargeRes(stV2gPreChargeRes& result, stV2gHeader& headerCheck) = 0;
    virtual bool read_V2gCurrentDemandReq(stV2gCurrentDemandReq& result, stV2gHeader& headerCheck) = 0;
    virtual bool read_V2gCurrentDemandRes(stV2gCurrentDemandRes& result, stV2gHeader& headerCheck) = 0;
    virtual bool read_V2gWeldingDetectionReq(stV2gWeldingDetectionReq& result, stV2gHeader& headerCheck) = 0;
    virtual bool read_V2gWeldingDetectionRes(stV2gWeldingDetectionRes& result, stV2gHeader& headerCheck) = 0;
    virtual bool read_V2gServiceDetailReq(stV2gServiceDetailReq& result, stV2gHeader& headerCheck) = 0;
    virtual bool read_V2gServiceDetailRes(stV2gServiceDetailRes& result, stV2gHeader& headerCheck) = 0;

    static  int  write_V2gSdpReq(stV2gSdpReq& data, stStreamControl& stream);
    static  int  write_V2gSdpRes(stV2gSdpRes& data, stStreamControl& stream);
    static  int  write_V2s(stStreamControl& stream);
    static  int  write_V2gSupportedAppProtocolReq(stV2gSupportedAppProtocolReq& data, stStreamControl& stream);
    static  int  write_V2gSupportedAppProtocolRes(stV2gSupportedAppProtocolRes& data, stStreamControl& stream);
    virtual int  write_V2gSessionSetupReq(stV2gSessionSetupReq& data, stStreamControl& stream) = 0;
    virtual int  write_V2gSessionSetupRes(stV2gSessionSetupRes& data, stStreamControl& stream) = 0;
    virtual int  write_V2gServiceDiscoveryReq(stV2gServiceDiscoveryReq& data, stStreamControl& stream) = 0;
    virtual int  write_V2gServiceDiscoveryRes(stV2gServiceDiscoveryRes& data, stStreamControl& stream) = 0;
    virtual int  write_V2gPaymentServiceSelectionReq(stV2gPaymentServiceSelectionReq& data, stStreamControl& stream) = 0;
    virtual int  write_V2gPaymentServiceSelectionRes(stV2gPaymentServiceSelectionRes& data, stStreamControl& stream) = 0;
    virtual int  write_V2gAuthorizationReq(stV2gAuthorizationReq& data, stStreamControl& stream) = 0;
    virtual int  write_V2gAuthorizationRes(stV2gAuthorizationRes& data, stStreamControl& stream) = 0;
    virtual int  write_V2gChargeParameterDiscoveryReq(stV2gChargeParameterDiscoveryReq& data, stStreamControl& stream) = 0;
    virtual int  write_V2gChargeParameterDiscoveryRes(stV2gChargeParameterDiscoveryRes& data, stStreamControl& stream) = 0;
    virtual int  write_V2gPowerDeliveryReq(stV2gPowerDeliveryReq& data, stStreamControl& stream) = 0;
    virtual int  write_V2gPowerDeliveryRes(stV2gPowerDeliveryRes& data, stStreamControl& stream) = 0;
    virtual int  write_V2gChargingStatusReq(stV2gChargingStatusReq& data, stStreamControl& stream) = 0;
    virtual int  write_V2gChargingStatusRes(stV2gChargingStatusRes& data, stStreamControl& stream) = 0;
    virtual int  write_V2gSessionStopReq(stV2gSessionStopReq& data, stStreamControl& stream) = 0;
    virtual int  write_V2gSessionStopRes(stV2gSessionStopRes& data, stStreamControl& stream) = 0;
    virtual int  write_V2gCableCheckReq(stV2gCableCheckReq& data, stStreamControl& stream) = 0;
    virtual int  write_V2gCableCheckRes(stV2gCableCheckRes& data, stStreamControl& stream) = 0;
    virtual int  write_V2gPreChargeReq(stV2gPreChargeReq& data, stStreamControl& stream) = 0;
    virtual int  write_V2gPreChargeRes(stV2gPreChargeRes& data, stStreamControl& stream) = 0;
    virtual int  write_V2gCurrentDemandReq(stV2gCurrentDemandReq& data, stStreamControl& stream) = 0;
    virtual int  write_V2gCurrentDemandRes(stV2gCurrentDemandRes& data, stStreamControl& stream) = 0;
    virtual int  write_V2gWeldingDetectionReq(stV2gWeldingDetectionReq& data, stStreamControl& stream) = 0;
    virtual int  write_V2gWeldingDetectionRes(stV2gWeldingDetectionRes& data, stStreamControl& stream) = 0;
    virtual int  write_V2gServiceDetailReq(stV2gServiceDetailReq& data, stStreamControl& stream) = 0;
    virtual int  write_V2gServiceDetailRes(stV2gServiceDetailRes& data, stStreamControl& stream) = 0;

private:
    void printUnrepetitive(LibCpp::cDebug* pDbg = nullptr, int* pPrintState = nullptr, std::string output = "");

public:
    LibCpp::cDebug  dbg;

private:
    int printedAuthorization;
    int printedCableCheck;
    int printedPrecharge;
    int printedWeldingDetection;
    int printedCurrentDemand;
    int printedChargingStatus;
};

};

#endif // CEXICODEC_H
