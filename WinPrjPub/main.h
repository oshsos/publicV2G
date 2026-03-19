#ifndef MAIN_H
#define MAIN_H


#include <conio.h>

#include "../LibCpp/HAL/cDebug.h"

#include "../LibCpp/HAL/Windows/cWinPacketSocket.h"    // Put 'cWinIp.h' (or any windows socket) as first windows hardware recource, to ensure 'winsock2.h' is included before 'windows.h'.
#include "../LibCpp/HAL/Windows/cWinUdpSocket.h"
#include "../LibCpp/HAL/Windows/cWinTcpClient.h"
#include "../LibCpp/HAL/Windows/cWinConsoleInput.h"

//#include "../LibIso15118/types.h"
#include "../LibIso15118/PlcStack/cPlcStackPub.h"
#include "../LibIso15118/Codico/cCodicoPilot.h"
#include "../LibIso15118/cExiIso15118_2013_openV2g.h"
#include "../LibIso15118/cExiDin70121openV2g.h"
#include "../LibIso15118/Support/Configuration.h"
#include "../LibCpp/Lab/cEAsource.h"

using namespace std;
using namespace LibCpp;
using namespace Iso15118;

enum class enOperationState
{
    initialization,
    plug,
    stateBtimer,
    slac,
    communication,
    processStopped,
    outOfOperation
};

std::string enOperationState_toString(enOperationState state);

//extern enStackType stackType;
extern stConfiguration config;

extern cWinConsoleInput console;
extern cDebug dbg;
extern cWinPacketSocket packetSocket;
extern cWinUdpSocket udp;
extern cWinTcpClient clientPower;
extern cEAsource powerSource;

extern cPlcStackPub plc;
extern cCodicoPilot controlPilot;
extern cExiIso15118_2013_openV2g codecIso;
extern cExiDin70121openV2g codecDin;

#endif
