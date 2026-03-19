#include "main.h"

#ifdef EVCC
    enStackType stackType = enStackType::evcc;
    extern int mainEVCC();
#else
    enStackType stackType = enStackType::secc;
    extern int mainSECC();
#endif

cDebug dbg("main", enDebugLevel_Debug);
cLxConsoleInput console;
cLxUdpSocket udp;

cExiDin70121openV2g codecDin;
cExiIso15118_2013_openV2g codecIso;

int main()
{

#ifdef EVCC
    return mainEVCC();
#else
    return mainSECC();
#endif
}
