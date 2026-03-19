//#include <cstring>
#include "main.h"

#include "../LibCpp/HAL/Tools.h"
#include "../LibCpp/HAL/Windows/cWinTcpServer.h"
#include "../LibIso15118/cIsoProcessSECC.h"

void printHelpSECC()
{
    printf_flush("'q' to finish\n");
    printf_flush("'h' to help\n");
    printf_flush("'r' to reset from out of operation\n");
    //printf_flush("'s' to friendly stop charging\n");
}

cWinTcpServer server;
cIsoProcessSECC isoProcessSECC(&codecIso, &udp, &server, SCHEMAID);

cConnectedClient* pClient = nullptr;

int mainSECC()
{

    dbg.setInstanceName("mainSECC");

    printf_flush("\nKEA publicV2G stack: Supply equipment sample program on message level\n\n");
    printHelpSECC();

    dbg.setInstanceName("mainSECC");
    udp.dbg.setInstanceName("port", &dbg, enDebugLevel_Info);
    server.dbg.setInstanceName("server", &dbg, enDebugLevel_Info);
    isoProcessSECC.dbg.setInstanceName("isoProcess", &dbg, enDebugLevel_Info);

    if (udp.open(15118, "WLAN", LibCpp::enAddressFamily_IPv6) != LibCpp::enIpResult_Success)
    {
        cInterfaces interfaces(true);
        printf("Opening '%s' failed!\n", config.interfaceName.c_str());
        printf_flush("Available Interfaces are:\n %s", interfaces.toString().c_str());
        interfaces.close();
    }

    // For TLS operation the TCP server is required to be configured in non automatic opening mode of connected client instances.
    server.open(55100, config.interfaceName, LibCpp::enAddressFamily_IPv6, true, 0, false);

    isoProcessSECC.seccInfo.seccId.set("XX00000", 7);
    isoProcessSECC.seccInfo.energyTransferModes.clear();
    isoProcessSECC.seccInfo.energyTransferModes.push_back(config.transferMode);
    isoProcessSECC.seccInfo.nominalVoltage = 400;
    isoProcessSECC.seccInfo.maxVoltage = 500;
    isoProcessSECC.seccInfo.maxCurrent = 16;
    isoProcessSECC.seccInfo.maxPower = 10000;
    isoProcessSECC.seccInfo.schemaId = config.schemaId;
    isoProcessSECC.provideTls = config.provideTls;

    isoProcessSECC.setServerIpAddress(server.getIpAddressLocal());
    isoProcessSECC.setupCommunication();

    isoProcessSECC.seccInfo.energyTransferModes.clear();
    isoProcessSECC.seccInfo.energyTransferModes.push_back(config.transferMode);

    while (1)
    {

        udp.operate();
        isoProcessSECC.operate();

        if (console.hasKey())
        {
            char input = console.getKey();
            printf_flush("\n");
            if (input == 'q')
                break;
            if (input == 'h')
            {
                printHelpSECC();
            }
            if (input == 'r')
            {
            }
            if (input == 's')
            {
            }
        }
    }

    if (pClient) delete pClient;

    udp.close();
    server.close();

    printf_flush("Ready\n");
    return 0;
}
