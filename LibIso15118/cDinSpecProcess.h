/**
 * State diagrams:
 * ISO/DIS 15118-2 :2018 (ED2)
 * Figure 189 — EVCC communication states for AC V2G messaging
 * Figure 190 - EVCC communication states for DC V2G messaging
 **/

#ifndef CISO15118PROCESS_H
#define CISO15118PROCESS_H

#include "../LibCpp/cTimer.h"
#include "../LibCpp/HAL/cDebug.h"
#include "../LibCpp/HAL/Tools.h"
#include "iso15118.h"
#include "cControlPilot.h"
#include"../LibCpp/Lab/cPowerSource.h"

namespace Iso15118
{
class cIsoProcessSECC
{
public:
    cIsoProcessSECC(cControlPilot* pControlPilot = nullptr, LibCpp::cPowerSource* pPowerSource = nullptr);

    void reset();
    void setServerIpAddress(LibCpp::stIpAddress serverIpAddress);
    int evaluate(char* receiveMessage, int receiveMessageLen);
    bool operate(bool run = true);

public:
    LibCpp::cDebug dbg;
    Iso15118::enV2gState v2gState;

    LibCpp::cByteArray  seccId;
    uint64_t            sessionId;
    LibCpp::cByteArray  evccId;

    iso1EnergyTransferModeType energyTransferMode = iso1EnergyTransferModeType_DC_core;

    int chargingStatusPrinted;

    char sendMessage[LibCpp_cIp_TCP_BUFFERSIZE];
    struct iso1EXIDocument receivedExiDocument;

    cTimer responseTimer;
    uint64_t responseTimes[enV2gState_finished+1];

    cControlPilot* pControlPilot;
    LibCpp::cPowerSource* pPowerSource;

private:
    LibCpp::stIpAddress serverIpAddress;
    enV2gState          nextV2gState;
};
}
#endif
