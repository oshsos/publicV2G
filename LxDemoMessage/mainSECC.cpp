//#include <cstring>
#include <unistd.h>

#include "main.h"

#include "../LibCpp/HAL/Tools.h"
#include "../LibCpp/HAL/Linux/cLxTcpServer.h"
#include "../LibIso15118/cIsoProcessSECC.h"

void printHelpSECC()
{
    printf_flush("'q' to finish\n");
    printf_flush("'h' to help\n");
    printf_flush("'r' to reset from out of operation\n");
    //printf_flush("'s' to friendly stop charging\n");
}

cLxTcpServer server;

cIsoProcessSECC isoProcessSECC(&codecIso, (cUdpSocket*)&udp, (cTcpServer*)&server);

cConnectedClient* pClient = nullptr;

int mainSECC()
{
    printf_flush("KEA Wired Power SECC Test Charger. Supply equipment on message level.\n");
    printHelpSECC();

    dbg.setInstanceName("mainSECC");
    udp.dbg.setInstanceName("port", &dbg, enDebugLevel_Info);
    server.dbg.setInstanceName("server", &dbg, enDebugLevel_Info);
    codecDin.dbg.setInstanceName("codecDin", &dbg, enDebugLevel_Info);
    codecIso.dbg.setInstanceName("codecIso", &dbg, enDebugLevel_Info);
    isoProcessSECC.dbg.setInstanceName("isoProcess", &dbg, enDebugLevel_Info);

    // server.showMessages = true;

    if (udp.open(15118, INTERFACE_NAME, LibCpp::enAddressFamily_IPv6) != LibCpp::enIpResult_Success)
    {
        cInterfaces interfaces(true);
        printf("Opening '%s' failed! Available interfaces are:\n%s\n", INTERFACE_NAME, interfaces.toString().c_str());
        interfaces.close();
    }
    server.open(55100, INTERFACE_NAME, LibCpp::enAddressFamily_IPv6);

    isoProcessSECC.setServerIpAddress(server.getIpAddressLocal());
    isoProcessSECC.setupCommunication();
    isoProcessSECC.evccInfo.energyTransferMode = iso1EnergyTransferModeType_DC_core;    //! Should be changed. Better to use seccInfo.energyTransferModes.

    while (1)
    {
//        // UDP Port Operation
//        char* receiveMessage;
//        int receiveMessageLen = udp.receiveBuffer(&receiveMessage);
//        if (receiveMessageLen>0)
//        {
//            int len = isoProcessSECC.evaluate(receiveMessage, receiveMessageLen);
//            udp.receiveAcknowledge();
//            if (len)
//            {
//                //dbg.printf(enDebugLevel_Info, ":\nSDP requested by %s.", udp.getIpAddressSource().toString(true, true).c_str());
//                udp.setDestination();
//                //dbg.printf(enDebugLevel_Debug, "Sending UDP message %s", cIp::instance().messageString(isoProcess.sendMessage, len).c_str());
//                udp.send(isoProcessSECC.sendMessage, len);
//            }
//        }

//        // TCP Server operation
//        // - Accepting clients
//        cConnectedClient* pNewClient = server.newClient();
//        if (pNewClient)
//        {
//            if (!pClient)
//            {
//                pClient = pNewClient;
//                stIpAddress addr = pClient->getIpAddressRemote();
//                dbg.printf(enDebugLevel_Info, "Vehicle established communication from %s.", addr.toString(true, true).c_str());
//                isoProcessSECC.v2gState = enV2gState_awaitSupportedAppProtocol;
//            }
//            else
//            {
//                dbg.printf(enDebugLevel_Error, "Unexpected connection request to TCP server!");
//                delete pNewClient;
//            }
//        }
//        // - Communication to clients
//        else if (pClient)                   // 'else' used to avoid reception of messages bevore V2gState has been changed
//        {
//            receiveMessageLen = pClient->receiveBuffer(&receiveMessage);
//            if (receiveMessageLen == -1)
//            {
//                dbg.printf(enDebugLevel_Info, "Vehicle lost communication with supply equipment.");
//                delete pClient;
//                pClient = nullptr;
//                if (isoProcessSECC.v2gState != Iso15118::enV2gState_finished)
//                    isoProcessSECC.nextV2gState = enV2gState_failure;
//            }
//            else if (receiveMessageLen>0 && (isoProcessSECC.v2gState == enV2gState_idle || isoProcessSECC.v2gState == Iso15118::enV2gState_finished))
//            {
//                dbg.printf(enDebugLevel_Error, "Unexpected request message from the vehicle.");
//                pClient->receiveAcknowledge();
//            }
//            else if (receiveMessageLen > 0)
//            {
//                //stStreamControl stream(receiveMessage, receiveMessageLen);
//                //dbg.printf(enDebugLevel_Debug, ":\nReceived request %s", stream.toString().c_str());

//                int sendMessageLen = isoProcessSECC.evaluate(receiveMessage, receiveMessageLen);

//                pClient->receiveAcknowledge();
//                if (sendMessageLen)
//                {
//                    //dbg.printf(enDebugLevel_Debug, "Sending TCP message %s", cIp::instance().messageString(isoProcess.sendMessage, sendMessageLen).c_str());
//                    pClient->send(isoProcessSECC.sendMessage, sendMessageLen);
//                }
//            }
//        }

        udp.operate();
        isoProcessSECC.operate(isoProcessSECC.isOperational());

        if (console.hasKey())
        {
            char input = console.getKey();
            printf_flush("\n", input);
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

        usleep(1000);
    }

    if (pClient) delete pClient;

    udp.close();
    server.close();

    printf_flush("Ready\n");
    return 0;
}
