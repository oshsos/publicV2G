#include "main.h"
#include "../LibCpp/HAL/Linux/cLxTcpClient.h"

#include "../LibCpp/HAL/Tools.h"

#include "../LibIso15118/cExiIso15118_2013_openV2g.h"
#include "../LibIso15118/cIsoProcessEVCC.h"

cLxTcpClient client;
cIsoProcessEVCC isoProcessEVCC(&codecIso, &udp, &client);

void printHelpEVCC()
{
    printf_flush("'q' to finish\n");
    printf_flush("'h' to help\n");
    printf_flush("'s' to friendly stop charging\n");
    printf_flush("'e' enable charging\n");
    printf_flush("\n");
}

int mainEVCC()
{

    printf_flush("KEA publicV2G: Electric Vehicle Sample Program on message Level\n\n");
    printHelpEVCC();

    dbg.setInstanceName("mainEVCC");
    udp.dbg.setInstanceName("port", &dbg, enDebugLevel_Info);
    client.dbg.setInstanceName("client", &dbg, enDebugLevel_Info);
    codecIso.dbg.setInstanceName("codecIso", &dbg, enDebugLevel_Info);
    codecDin.dbg.setInstanceName("codecDin", &dbg, enDebugLevel_Info);
    isoProcessEVCC.dbg.setInstanceName("isoProcessEVCC", &dbg, enDebugLevel_Info);

    if (udp.open(55100, INTERFACE_NAME, LibCpp::enAddressFamily_IPv6) != LibCpp::enIpResult_Success)
    {
        cInterfaces interfaces(true);
        printf("Opening '%s' failed! Available interfaces are:\n%s\n", INTERFACE_NAME, interfaces.toString().c_str());
        interfaces.close();
    }

    udp.setDestination(stIpAddress("FF02::1"), 15118);
    isoProcessEVCC.setupCommunication();
    isoProcessEVCC.enableCharging = true;

    while (1)
    {
        // Message process operation
        udp.operate();
        client.operate();

        isoProcessEVCC.operate(isoProcessEVCC.isOperational() && isoProcessEVCC.enableCharging);

        if (console.hasKey())
        {
            char input = console.getKey();
            printf_flush("\n", input);
            if (input == 'q')
                break;
            if (input == 'h')
            {
                printHelpEVCC();
            }
            if (input == 's')
            {
                isoProcessEVCC.enableCharging = false;
            }
            if (input == 'e')
            {
                isoProcessEVCC.enableCharging = true;
            }
        }
    }
    
    udp.close();
    client.close();

    printf_flush("Ready\n");
    return 0;
}
