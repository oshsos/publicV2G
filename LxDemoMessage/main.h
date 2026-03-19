#ifndef MAIN_H
#define MAIN_H

#define INTERFACE_NAME "wlo1"
//#define INTERFACE_NAME "Ethernet"
//#define INTERFACE_NAME "Ethernet 6"

//#define SCHEMAID 3  // Codec Iso an Codico-stack
//#define SCHEMAID 2  // Codec Din an Codico-stack
#define SCHEMAID 10   // Iso an dSpace, Iso an RiseV2G
//#define SCHEMAID 1    // Switch python reference stack

#define EVCC
#define SKIP_PRECHARGE_CHECK   // Use for Switch python secc stack. It does not reply the target voltage as actual voltage.

#include "../LibCpp/HAL/cDebug.h"

#include "../LibCpp/HAL/Linux/cLxUdpSocket.h"
#include "../LibCpp/HAL/Linux/cLxConsoleInput.h"

#include "../LibIso15118/types.h"
#include "../LibIso15118/cExiIso15118_2013_openV2g.h"
#include "../LibIso15118/cExiDin70121openV2g.h"

using namespace std;
using namespace LibCpp;
using namespace Iso15118;

extern enStackType stackType;

extern cDebug dbg;
extern cLxConsoleInput console;
extern cLxUdpSocket udp;

extern cExiDin70121openV2g codecDin;
extern cExiIso15118_2013_openV2g codecIso;

#endif
