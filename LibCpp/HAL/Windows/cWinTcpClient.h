// UTF8 (ü)
/**
\file   cWinTcpClient.h

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2024-01-23
\brief  See cWinTcpClient.cpp

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL_Windows
@{

\class LibCpp::cWinTcpClient

Example program for polling operation:
@code
#include "../LibCpp/HAL/Windows/cWinTcpClient.h"

cWinTcpClient port;

bool autoconnect = true;

int cnt = 0;

int main()
{
    cDebug dbg("main");
    dbg.setDebugLevel();
    dbg.setNewLineOutput();

    port.dbg.setInstanceName("port", &dbg);
    port.dbg.setDebugLevel(enDebugLevel_Info);
    if (autoconnect)
        port.verbose = false;

    printf_flush("press 'q' for quit, 's' for send, 'c' for close, 'o' for open\n\n");

    // Starting 'port' operation
    int serverPort = 55100;
    //std::string serverName = "192.168.178.59";
    std::string serverName = "127.0.0.1";
    port.open(serverPort, serverName);

    char receiveBuffer[1024];
    while (1)
    {
        printf_flush("Count = %i\r", cnt++);

        if (autoconnect)
        {
            port.open(serverPort, serverName);
        }

        int len = port.receive(receiveBuffer, 1024);
        if (len>0)
            printf_flush("Received: %s\n", receiveBuffer);
        else if (len==-1)   // Server disconnection
        {
            if (!autoconnect)
                printf_flush("Server %s disconnected.\n", port.getIpAddressServer().toString(true, true).c_str());
            port.close();   // It is good practice to call close to free the socket. Although done here, you might leave it out for autoconnect operation. The call is non blocking as the background thread is already finished after a disconnect.
        }
        else if (len==-2)   // Client cosed
        {
            if (!autoconnect)
                printf_flush("Client closed or connecting to server %s failed.\n", port.getIpAddressServer().toString(true, true).c_str());
            port.close();   // See close() above.
        }

        if (_kbhit())
        {
            input = getch();
            printf_flush("\n");
            if (input == 'q')
                break;
            if (input == 's')
            {
                const char* sendBuffer = "Hallo";
                int len = port.send(sendBuffer, strlen(sendBuffer));
                if (len>0)
                    printf_flush("Sent    : %s\n", sendBuffer);
                else
                    printf_flush("Sending impossible as server is not connected!\n");
            }
            if (input == 'o')
            {
                printf_flush("Opening\n");
                port.open(serverPort, serverName);
            }
            if (input == 'c')
            {
                printf_flush("Closing\n");
                port.close();
            }
        }
        Sleep(100);
    }

    port.close();
    printf_flush("\nready\n");
    return 0;
}
@endcode
Required library files are:
@code
SOURCES += \
        ../LibCpp/HAL/Tools.cpp \
        ../LibCpp/HAL/Windows/HW_Tools.cpp \
        ../LibCpp/HAL/Windows/cWinIp.cpp \
        ../LibCpp/HAL/Windows/cWinTcpClient.cpp \
        ../LibCpp/HAL/Windows/cWinTcpServer.cpp \
        ../LibCpp/HAL/Windows/cWinUdpSocket.cpp \
        ../LibCpp/HAL/cDebug.cpp \
        ../LibCpp/HAL/cFramePort.cpp \
        ../LibCpp/HAL/cIp.cpp \
        ../LibCpp/HAL/cTcpClient.cpp \
        ../LibCpp/HAL/cTcpServer.cpp \
        ../LibCpp/HAL/cUdpSocket.cpp \
        main.cpp

HEADERS += \
    ../LibCpp/HAL/HW_Tools.h \
    ../LibCpp/HAL/Tools.h \
    ../LibCpp/HAL/Windows/cWinIp.h \
    ../LibCpp/HAL/Windows/cWinTcpClient.h \
    ../LibCpp/HAL/Windows/cWinTcpServer.h \
    ../LibCpp/HAL/Windows/cWinUdpSocket.h \
    ../LibCpp/HAL/cDebug.h \
    ../LibCpp/HAL/cFramePort.h \
    ../LibCpp/HAL/cIp.h \
    ../LibCpp/HAL/cTcpClient.h \
    ../LibCpp/HAL/cTcpServer.h \
    ../LibCpp/HAL/cUdpSocket.h

LIBS += $$PWD/../LibCpp/HAL/Windows/iphlpapi.lib
LIBS += $$PWD/../LibCpp/HAL/Windows/ws2_32.lib
@endcode
**/


#ifndef CWINTCPCLIENT_H
#define CWINTCPCLIENT_H

#ifdef PROJECTDEFS
    #include <ProjectDefs.h>
#endif

#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include "cWinIp.h"

// The following is included by cWinIp.h
//#include <winsock2.h>   // To be sure to include before <windows.h>, as required.
//#include <ws2tcpip.h>
//#include <netioapi.h>
//#include <iphlpapi.h>
//#include "cWinIp.h"
//#include <windows.h>    // must be included after <winsock2.h>!

#include <thread>
//#include <atomic>
#ifdef __MINGW32__
#include "atomic_patch/atomic.h"     // There is an ambigious conversion in <atomic> in line 367. This is corrected in the local library file.
#else
#include <atomic>
#endif

#include "../cTcpClient.h"
#include "../cDebug.h"

namespace LibCpp
{

/**
 * @brief The windows implementation of the LibCpp::cUdpSocket hardware abstraction class.
 */
class cWinTcpClient : public cTcpClient
{
public:
    cWinTcpClient(bool open = false, int serverPort = LibCpp_cIp_TCP_PORT, std::string serverAddressName = "", int localPort = 0, std::string localInterfaceOrAddressName = "", bool async = false, iFramePort* pCallback = nullptr); ///< Constructor
    virtual ~cWinTcpClient();           ///< Destructor.

    enIpResult  open(int serverPort = LibCpp_cIp_TCP_PORT, std::string serverAddressName = "", int localPort = 0, std::string localInterfaceOrAddressName = "", bool async = false); ///< Sets the Udp socket into operation
    bool        prepareClose();         ///< Prepares for closing by friendly closing the background task.
    void        close();                ///< Frees the resources and finishes the background task

    int         send(const char* pMsg = nullptr, int messageLen = 0, enBlocking blockingMode = enBlocking_BUFFER);             ///> Sends a data frame.
    int         receive(char* pMsg = nullptr, int bufferSize = 0, enBlocking blockingMode = enBlocking_NONE);                  ///> Receives a data frame.
    int         receiveBuffer(char** ppMsg = nullptr, enBlocking blockingMode = enBlocking_NONE);                              ///> Receives a data frame without copying
    void        receiveAcknowledge();   ///> Acknowledges a reception of a message by 'receiveBuffer'
    void        operate();              ///> Cyclic operation on callback polling mode usage.

    bool        isConnected();          ///< Checks for the connection state to the server
    bool        isClosed();             ///< Checks whether the disconnection is completely processed.

    stIpAddress getIpAddressServer();   ///< Reads the configured destionation ip address.
    virtual int getSocketDescriptor();  ///> Returns the socket file descriptor

private:
    void        internalOperate();      ///< Handles the callback instances
    void        threadReceiving();      ///< Background receiving thread

public:
    cDebug              dbg;                        ///< Logging interface.

private:
    int                 localSocket;                ///< Local socket the cUdpSocket class instance is using.
    unSockAddr          serverSockAddress;          ///< Address to send messages to.
    char                message[LibCpp_cIp_TCP_BUFFERSIZE]; ///< Message buffer of received messages.
    std::atomic<int>    messageLen;                 ///< Length of the message stored in the message buffer (controls access between background process and main process.
    std::atomic<bool>   threadEnabled;              ///< Flag indicating to start the background process or to friendly close the process.
    std::atomic<bool>   threadIsRunning;            ///< Flag indicating if the background process is running.
    std::atomic<bool>   isConnectedToServer;        ///< Indicates a successful connect process until a server sided disconnect or client side disconnecting process.
    std::thread*        pThread;                    ///< Class instance representing the background task.
    bool                async;                      ///< Asynchronous call of callback objects
};

}

#endif

/** @} */
