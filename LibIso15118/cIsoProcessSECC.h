/**
 * State diagrams:
 * ISO/DIS 15118-2 :2018 (ED2)
 * Figure 189 — EVCC communication states for AC V2G messaging
 * Figure 190 - EVCC communication states for DC V2G messaging
 **/

#ifndef CISOPROCESSSECC_H
#define CISOPROCESSSECC_H

#include "../LibCpp/cTimer.h"
#include "../LibCpp/HAL/cDebug.h"
#include "../LibCpp/HAL/cUdpSocket.h"
#include "../LibCpp/HAL/cTcpServer.h"
#include "../LibCpp/HAL/Tls/cTls.h"
#include "cSeHardware.h"
#include "cControlPilot.h"
#include "isoTypes.h"
#include "cExiCodec.h"

namespace Iso15118
{
class cIsoProcessSECC : LibCpp::iFramePort
{
public:
    cIsoProcessSECC(cExiCodec* pCodec, LibCpp::cUdpSocket* pUdp, LibCpp::cTcpServer* pTcp, int schemaId, cControlPilot* pControlPilot = nullptr, cSeHardware* pSe = nullptr);

    void setupCommunication();
    void clear();
    void restart();
    void setServerIpAddress(LibCpp::stIpAddress serverIpAddress);
    void evaluate();
    bool operate(bool run = true);
    bool isInitialized();                           ///< Checks for control pilot and SE hardware to be initialized.
    bool isOperational();                           ///< The 'operate' method requires to be called with 'false' argument in order to restart the state machine.
    enV2gSeState state();                           ///< @copybrief cIsoProcessSECC::state

protected:
    void send(stV2gMessage& sendMsg);
    void onFrameReceive(LibCpp::cFramePort* pFramePort);
    void prepareCurrentDemandRes(stV2gMessage& msg);
    void preparePreChargeRes(stV2gMessage& msg);
    void prepareWeldingDetectionRes(stV2gMessage& msg);

public:
    LibCpp::cDebug dbg;

protected:
    cExiCodec*                  pCodec;
    LibCpp::cUdpSocket*         pUdp;
    LibCpp::cTcpServer*         pTcp;
    LibCpp::cConnectedClient*   pClient;
    cControlPilot*              pControlPilot;
    cSeHardware                 stdSe;
    cSeHardware*                pSe;

public:
    stSeccProcessDataInput      seccInput;
    stSeccProcessDataOutput     seccOutput;
    stSeccInfo                  seccInfo;
    stEvccInfo                  evccInfo;

    LibCpp::cTimer responseTimer;
    uint64_t responseTimes[(int)enV2gSeState::finished+1];

    //enV2gSeState v2gState;
    bool enteredV2gState;

    LibCpp::cTls::stCertFiles tlsConfig;
    bool provideTls;        // A configuration variable to accept TLS communication from an EV
    bool useTls;            // Memorizes, if TLS communication has been negotiated
//    LibCpp::cByteArray  seccId;
//    uint64_t            sessionId;
//    LibCpp::cByteArray  evccId;

//    iso1EnergyTransferModeType energyTransferMode = iso1EnergyTransferModeType_DC_core;

    //int schemaId;
protected:
    char sendMessage[LibCpp_cIp_TCP_BUFFERSIZE];

    //LibCpp::stIpAddress serverIpAddress;
    stV2gMessage        receivedMessage;
    //float evMemorizedVoltage;
    LibCpp::cTimer      heardBeatTimer;
};
}
#endif
