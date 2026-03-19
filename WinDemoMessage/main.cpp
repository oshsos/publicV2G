#include "main.h"

stConfiguration             config;

//#ifdef EVCC
//    enStackType stackType = enStackType::evcc;
//    extern int mainEVCC();
//#else
//    enStackType stackType = enStackType::secc;
//    extern int mainSECC();
//#endif

cDebug dbg("main", enDebugLevel_Debug);
cWinConsoleInput console;
cWinUdpSocket udp;

cExiDin70121openV2g codecDin;
cExiIso15118_2013_openV2g codecIso;

int main()
{
    // Read/Write XML file
    if (!readXML(config))
    {
        dbg.printf(enDebugLevel_Error, "Configuration file config.xml not available or requested configuration not found! Generating standard file.");
        writeXML(config);
    }

    if (config.stackType == enStackType::evcc)
        return mainEVCC();
    else
        return mainSECC();
}
