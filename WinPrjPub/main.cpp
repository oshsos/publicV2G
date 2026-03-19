// UTF8 (ü)
/**
\file   main.cpp

\author Dr. Olaf Simon (KEA)
\author KEA
\date   2024-07-16

\addtogroup G_publicV2G

\brief The publicV2G main file for Windows

\page Page_Main_Windows_Public The main code for the Windows implementation

\ref main.cpp

 *
 **/

#include "main.h"
#include "../LibCpp/Lab/cEAsource.h"

using namespace std;

extern int mainEVCC();
extern int mainSECC();

cDebug                      dbg("main", enDebugLevel_Debug);
cWinConsoleInput            console;

//enStackType                 stackType;
stConfiguration             config;

cWinPacketSocket            packetSocket;
cWinUdpSocket               udp;
cWinTcpClient               clientPower;

cPlcStackPub                plc(&packetSocket);
cCodicoPilot                controlPilot(enStackType::secc, &packetSocket, &plc);
cExiIso15118_2013_openV2g   codecIso;
cExiDin70121openV2g         codecDin;
cEAsource                   powerSource(&clientPower);

int main()
{
//    cDebug::instance().setDebugLevel(enDebugLevel_Debug);
//    cDebug dbg("main");

    // Read/Write XML file
    if (!readXML(config))
    {
        dbg.printf(enDebugLevel_Error, "Configuration file config.xml not available or requested configuration not found! Generating standard file.");
        writeXML(config);
    }
    config.hardware = enHardwareType::NONE;

    // Set operation parameters according to the configuration file
    controlPilot.setStackMode(config.stackType);
//    if (config.stackType == "evcc")
//        stackType = enStackType::evcc;
//    else
//        stackType = enStackType::secc;
//        controlPilot.setStackMode(enStackType::secc);

    // Run the application
    if (config.stackType == enStackType::evcc)
        return mainEVCC();
    else
        return mainSECC();
}

std::string enOperationState_toString(enOperationState state)
{
    switch (state)
    {
    case enOperationState::initialization: return "Initialization";
    case enOperationState::plug: return "Plug";
    case enOperationState::stateBtimer: return "stateBtimer";
    case enOperationState::slac: return "Slac";
    case enOperationState::communication: return "Communication";
    case enOperationState::processStopped: return "processStopped";
    case enOperationState::outOfOperation: return "Out of operation";
    default: return "<unknown>";
    }
}
