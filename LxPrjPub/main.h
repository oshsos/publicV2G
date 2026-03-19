#ifndef MAIN_H
#define MAIN_H

#define STDCONFIGNAME "standard"
// This is the standard configuration being set if no configuration file (config.xml) is present
#define STACKTYPE enStackType::secc
//#define STACKTYPE "evcc"

//#define INTERFACE_NAME "WLAN"
#define INTERFACE_NAME "Ethernet"
//#define INTERFACE_NAME "Ethernet 3"
//#define INTERFACE_NAME "Ethernet 6"

//#define SCHEMAID 10  // Codec Iso an dSpace
#define SCHEMAID 3  // Codec Iso an Codico-stack
//#define SCHEMAID 2  // Codec Din an Codico-stack / ISO an IONIQ5
//#define SCHEMAID 1  // Codec Din an IONIQ5, ID.3
//#define SCHEMAID 0  // Codec Iso an ID.3

#define TRANSFERMODE iso1EnergyTransferModeType_AC_three_phase_core
#define PROVIDETLS false


//#define EASOURCE_IP_ADDRESS "192.168.178.74"
//#define EASOURCE_IP_ADDRESS "192.168.0.3"
//#define EASOURCE_IP_ADDRESS "192.168.0.5"
#define EASOURCE_IP_ADDRESS "192.168.0.6"

#define VOLTAGE_FAKTOR 1
#define CURRENT_FAKTOR 1

//#include <conio.h>

#include "../LibCpp/HAL/cDebug.h"

#include "../LibCpp/HAL/Linux/cLxPacketSocket.h"
#include "../LibCpp/HAL/Linux/cLxUdpSocket.h"
#include "../LibCpp/HAL/Linux/cLxTcpClient.h"
#include "../LibCpp/HAL/Linux/cLxConsoleInput.h"

#include "../LibIso15118/types.h"
#include "../LibIso15118/PlcStack/cPlcStackPub.h"
#include "../LibIso15118/Codico/cCodicoPilot.h"
#include "../LibIso15118/cExiIso15118_2013_openV2g.h"
#include "../LibIso15118/cExiDin70121openV2g.h"
#include "../LibCpp/Lab/cEAsource.h"

using namespace std;
using namespace LibCpp;
using namespace Iso15118;

enum class enHardwareType
{
    NONE,
    EA_SOURCE,
    ADVANTECH
};

typedef struct stConfiguration
{
    string      instance         = string(STDCONFIGNAME) + " " + (STACKTYPE == enStackType::secc ? string("secc") : string("evcc"));
    enStackType stackType        = STACKTYPE;
    string      interfaceName    = INTERFACE_NAME;
    int         schemaId         = SCHEMAID;
    string      eaSourceIp       = EASOURCE_IP_ADDRESS;
    bool        provideTls       = PROVIDETLS;
    iso1EnergyTransferModeType transferMode = TRANSFERMODE;
    enHardwareType hardware     = enHardwareType::NONE;
} stConfiguration;

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

extern cLxConsoleInput console;
extern cDebug dbg;
extern cLxPacketSocket packetSocket;
extern cLxUdpSocket udp;
extern cLxTcpClient clientPower;
extern cEAsource powerSource;

extern cPlcStackPub plc;
extern cCodicoPilot controlPilot;
extern cExiIso15118_2013_openV2g codecIso;
extern cExiDin70121openV2g codecDin;

#endif
