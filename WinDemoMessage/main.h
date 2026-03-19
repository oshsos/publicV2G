#ifndef MAIN_H
#define MAIN_H

//#define INTERFACE_NAME "WLAN"
//#define INTERFACE_NAME "Ethernet"
//#define INTERFACE_NAME "Ethernet 6"

//#define SCHEMAID 3  // Codec Iso an Codico-stack
//#define SCHEMAID 2  // Codec Din an Codico-stack
//#define SCHEMAID 10   // Iso an dSpace
//#define SCHEMAID 1    // Switch python reference stack

//#define EVCC

#include <conio.h>

#include "../LibCpp/HAL/cDebug.h"
#include "../LibCpp/HAL/Windows/cWinUdpSocket.h"
#include "../LibCpp/HAL/Windows/cWinConsoleInput.h"

#include "../LibIso15118/types.h"
#include "../LibIso15118/cExiIso15118_2013_openV2g.h"
#include "../LibIso15118/cExiDin70121openV2g.h"
#include "../LibIso15118/Support/Configuration.h"

using namespace std;
using namespace LibCpp;
using namespace Iso15118;

//extern enStackType stackType;

extern stConfiguration config;

extern cDebug dbg;
extern cWinConsoleInput console;
extern cWinUdpSocket udp;

extern cExiDin70121openV2g codecDin;
extern cExiIso15118_2013_openV2g codecIso;

int mainEVCC();
int mainSECC();

#endif
