#include "main.h"
#include "../LibCpp/HAL/Windows/cWinTcpClient.h"

#include "../LibCpp/HAL/Tools.h"

#include "../LibIso15118/cExiIso15118_2013_openV2g.h"
#include "../LibIso15118/cIsoProcessEVCC.h"

cWinTcpClient client;
cIsoProcessEVCC isoProcessEVCC(&codecIso, &udp, &client, config.interfaceName);

void printHelpEVCC()
{
    printf_flush("'q' to finish\n");
    printf_flush("'h' to help\n");
    printf_flush("'s' to friendly stop charging\n");
    printf_flush("\n");
}

int mainEVCC()
{

    printf_flush("\nKEA publicV2G stack: Electric vehicle sample program on message level\n\n");
    printHelpEVCC();

    dbg.setInstanceName("mainEVCC");
    udp.dbg.setInstanceName("port", &dbg, enDebugLevel_Info);
    client.dbg.setInstanceName("client", &dbg, enDebugLevel_Info);
    codecIso.dbg.setInstanceName("codecIso", &dbg, enDebugLevel_Info);
    codecDin.dbg.setInstanceName("codecDin", &dbg, enDebugLevel_Info);
    isoProcessEVCC.dbg.setInstanceName("isoProcessEVCC", &dbg, enDebugLevel_Info);

    if (udp.open(0, config.interfaceName.c_str(), LibCpp::enAddressFamily_IPv6) != LibCpp::enIpResult_Success)
    {
        cInterfaces interfaces(true);
        printf_flush("Opening '%s' failed!\n", config.interfaceName.c_str());
        printf_flush("Available Interfaces are:\n %s", interfaces.toString().c_str());
        interfaces.close();
    }
    udp.setDestination(stIpAddress("FF02::1"), 15118);

    isoProcessEVCC.evccInfo.energyTransferMode = config.transferMode;
    isoProcessEVCC.clientInterface = config.interfaceName;
    isoProcessEVCC.evccInfo.schemaId = config.schemaId;

    isoProcessEVCC.setupCommunication();

    while (1)
    {
//        // UDP Port Operation
//        udp.operate();
//        char* receiveMessage;
//        int receiveMessageLen = udp.receiveBuffer(&receiveMessage);
//        if (receiveMessageLen>0)
//        {
//            isoProcessEVCC.evaluateMessage(receiveMessage, receiveMessageLen);
//            udp.receiveAcknowledge();
//        }

//        if (isoProcessEVCC.v2gState == Iso15118::enV2gState_awaitSdp && isoProcessEVCC.sendMessageLen > 0)
//        {
//            udp.send(isoProcessEVCC.sendMessage, isoProcessEVCC.sendMessageLen);
//            isoProcessEVCC.sendMessageLen = 0;
//        }

//        // TCP Client operation
//        client.operate();
//        receiveMessageLen = client.receiveBuffer(&receiveMessage);
//        if (receiveMessageLen>0)
//        {
//            isoProcessEVCC.evaluateMessage(receiveMessage, receiveMessageLen);
//            client.receiveAcknowledge();
//        }
//        else if (receiveMessageLen == -1)
//        {
//            dbg.printf(enDebugLevel_Info, "Supply equipment disconnected.");
//            client.close();
//        }
//        else if (receiveMessageLen == -2)
//        {
//            dbg.printf(enDebugLevel_Info, "Client failed to open.");
//        }

//        if (isoProcessEVCC.sendMessageLen)
//        {
//            client.send(isoProcessEVCC.sendMessage, isoProcessEVCC.sendMessageLen);
//            isoProcessEVCC.sendMessageLen = 0;
//        }

        // Message process operation
        udp.operate();
        client.operate();
        isoProcessEVCC.operate();

        if (console.hasKey())
        {
            char input = console.getKey();
            printf_flush("\n%c\n", input);
            if (input == 'q')
                break;
            if (input == 'h')
            {
                printHelpEVCC();
            }
            if (input == 's')
            {
                isoProcessEVCC.evccInput.enable = false;
            }
        }
    }
    
    udp.close();
    client.close();

    printf_flush("Ready\n");
    return 0;
}
