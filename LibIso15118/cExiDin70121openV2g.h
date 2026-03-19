#ifndef ISO15118_H
#define ISO15118_H

#include "cExiCodec.h"
#include "../LibCpp/HAL/cDebug.h"
#include "openV2G/din/dinEXIDatatypes.h"

namespace Iso15118
{
//extern char sendMessage[LibCpp_cIp_TCP_BUFFERSIZE];

class cExiDin70121openV2g : public cExiCodec
{
public:
    cExiDin70121openV2g();

    bool read_V2gHeader(stV2gHeader& result, stStreamControl& stream, bool expectSuppAppProt = false);
    bool read_V2gSessionSetupReq(stV2gSessionSetupReq& result, stV2gHeader& headerCheck);
    bool read_V2gSessionSetupRes(stV2gSessionSetupRes& result, stV2gHeader& headerCheck);
    bool read_V2gServiceDiscoveryReq(stV2gServiceDiscoveryReq& result, stV2gHeader& headerCheck);
    bool read_V2gServiceDiscoveryRes(stV2gServiceDiscoveryRes& result, stV2gHeader& headerCheck);
    bool read_V2gPaymentServiceSelectionReq(stV2gPaymentServiceSelectionReq& result, stV2gHeader& headerCheck);
    bool read_V2gPaymentServiceSelectionRes(stV2gPaymentServiceSelectionRes& result, stV2gHeader& headerCheck);
    bool read_V2gAuthorizationReq(stV2gAuthorizationReq& result, stV2gHeader& headerCheck);
    bool read_V2gAuthorizationRes(stV2gAuthorizationRes& result, stV2gHeader& headerCheck);
    bool read_V2gChargeParameterDiscoveryReq(stV2gChargeParameterDiscoveryReq& result, stV2gHeader& headerCheck);
    bool read_V2gChargeParameterDiscoveryRes(stV2gChargeParameterDiscoveryRes& result, stV2gHeader& headerCheck);
    bool read_V2gPowerDeliveryReq(stV2gPowerDeliveryReq& result, stV2gHeader& headerCheck);
    bool read_V2gPowerDeliveryRes(stV2gPowerDeliveryRes& result, stV2gHeader& headerCheck);
    bool read_V2gChargingStatusReq(stV2gChargingStatusReq& result, stV2gHeader& headerCheck);
    bool read_V2gChargingStatusRes(stV2gChargingStatusRes& result, stV2gHeader& headerCheck);
    bool read_V2gSessionStopReq(stV2gSessionStopReq& result, stV2gHeader& headerCheck);
    bool read_V2gSessionStopRes(stV2gSessionStopRes& result, stV2gHeader& headerCheck);
    bool read_V2gCableCheckReq(stV2gCableCheckReq& result, stV2gHeader& headerCheck);
    bool read_V2gCableCheckRes(stV2gCableCheckRes& result, stV2gHeader& headerCheck);
    bool read_V2gPreChargeReq(stV2gPreChargeReq& result, stV2gHeader& headerCheck);
    bool read_V2gPreChargeRes(stV2gPreChargeRes& result, stV2gHeader& headerCheck);
    bool read_V2gCurrentDemandReq(stV2gCurrentDemandReq& result, stV2gHeader& headerCheck);
    bool read_V2gCurrentDemandRes(stV2gCurrentDemandRes& result, stV2gHeader& headerCheck);
    bool read_V2gWeldingDetectionReq(stV2gWeldingDetectionReq& result, stV2gHeader& headerCheck);
    bool read_V2gWeldingDetectionRes(stV2gWeldingDetectionRes& result, stV2gHeader& headerCheck);
    bool read_V2gServiceDetailReq(stV2gServiceDetailReq& result, stV2gHeader& headerCheck);
    bool read_V2gServiceDetailRes(stV2gServiceDetailRes& result, stV2gHeader& headerCheck);

    int  write_V2gSessionSetupReq(stV2gSessionSetupReq& data, stStreamControl& stream);
    int  write_V2gSessionSetupRes(stV2gSessionSetupRes& data, stStreamControl& stream);
    int  write_V2gServiceDiscoveryReq(stV2gServiceDiscoveryReq& data, stStreamControl& stream);
    int  write_V2gServiceDiscoveryRes(stV2gServiceDiscoveryRes& data, stStreamControl& stream);
    int  write_V2gPaymentServiceSelectionReq(stV2gPaymentServiceSelectionReq& data, stStreamControl& stream);
    int  write_V2gPaymentServiceSelectionRes(stV2gPaymentServiceSelectionRes& data, stStreamControl& stream);
    int  write_V2gAuthorizationReq(stV2gAuthorizationReq& data, stStreamControl& stream);
    int  write_V2gAuthorizationRes(stV2gAuthorizationRes& data, stStreamControl& stream);
    int  write_V2gChargeParameterDiscoveryReq(stV2gChargeParameterDiscoveryReq& data, stStreamControl& stream);
    int  write_V2gChargeParameterDiscoveryRes(stV2gChargeParameterDiscoveryRes& data, stStreamControl& stream);
    int  write_V2gPowerDeliveryReq(stV2gPowerDeliveryReq& data, stStreamControl& stream);
    int  write_V2gPowerDeliveryRes(stV2gPowerDeliveryRes& data, stStreamControl& stream);
    int  write_V2gChargingStatusReq(stV2gChargingStatusReq& data, stStreamControl& stream);
    int  write_V2gChargingStatusRes(stV2gChargingStatusRes& data, stStreamControl& stream);
    int  write_V2gSessionStopReq(stV2gSessionStopReq& data, stStreamControl& stream);
    int  write_V2gSessionStopRes(stV2gSessionStopRes& data, stStreamControl& stream);
    int  write_V2gCableCheckReq(stV2gCableCheckReq& data, stStreamControl& stream);
    int  write_V2gCableCheckRes(stV2gCableCheckRes& data, stStreamControl& stream);
    int  write_V2gPreChargeReq(stV2gPreChargeReq& data, stStreamControl& stream);
    int  write_V2gPreChargeRes(stV2gPreChargeRes& data, stStreamControl& stream);
    int  write_V2gCurrentDemandReq(stV2gCurrentDemandReq& data, stStreamControl& stream);
    int  write_V2gCurrentDemandRes(stV2gCurrentDemandRes& data, stStreamControl& stream);
    int  write_V2gWeldingDetectionReq(stV2gWeldingDetectionReq& data, stStreamControl& stream);
    int  write_V2gWeldingDetectionRes(stV2gWeldingDetectionRes& data, stStreamControl& stream);
    int  write_V2gServiceDetailReq(stV2gServiceDetailReq& data, stStreamControl& stream);
    int  write_V2gServiceDetailRes(stV2gServiceDetailRes& data, stStreamControl& stream);

private:
    int  write_ExiDocument(struct dinEXIDocument& exi, stStreamControl& stream);

protected:
    dinEXIDocument receivedExiDocument;
};
}
#endif
