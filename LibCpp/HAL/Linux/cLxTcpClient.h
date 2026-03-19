// UTF8 (ü)
/**
\file   cLxTcpClient.h

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2024-03-27
\brief  See cWinTcpClient.cpp

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL_Linux
@{

\class LibCpp::cWinTcpClient
**/

#ifndef CLXTCPCLIENT_H
#define CLXTCPCLIENT_H

#ifdef PROJECTDEFINES
    #include <ProjectDefines>
#endif

#include <string>
#include <cstdint>
#include <sys/socket.h>
#include <netinet/in.h>
#include <atomic>
#include <thread>

#include "cLxIp.h"
#include "../cTcpClient.h"
#include "../cDebug.h"

namespace LibCpp
{

class cLxTcpClient : public cTcpClient
{
public:
    cLxTcpClient(bool open = false, int serverPort = LibCpp_cIp_TCP_PORT, std::string serverAddressName = std::string(""), int localPort = 0, std::string localInterfaceOrAddressName = std::string(""), bool async = false, iFramePort* pCallback = nullptr); ///< Constructor
    virtual ~cLxTcpClient();           ///< Destructor.

    enIpResult  open(int serverPort = LibCpp_cIp_TCP_PORT, std::string serverAddressName = std::string(""), int localPort = 0, std::string localInterfaceOrAddressName = std::string(""), bool async = false); ///< Sets the Udp socket into operation
    bool        prepareClose();         ///< Prepares for closing by friendly closing the background task.
    void        close();                ///< Frees the resources and finishes the background task

    int         send(const char* pMsg = nullptr, int messageLen = 0, enBlocking blockingMode = enBlocking_BUFFER);             ///> Sends a data frame.
    int         receive(char* pMsg = nullptr, int bufferSize = 0, enBlocking blockingMode = enBlocking_NONE);                  ///> Receives a data frame.
    int         receiveBuffer(char** ppMsg = nullptr, enBlocking blockingMode = enBlocking_NONE);                              ///> Receives a data frame without copying
    void        receiveAcknowledge();   ///> Acknowledges a reception of a message by 'receiveBuffer'
    void        operate();              ///> Cyclic operation on callback polling mode usage.

    bool        isConnected();          ///< Checks for the connection state to the server.
    bool        isClosed();             ///< Checks whether the client is closed, meaning the background task has stopped.

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
    std::atomic<bool>   threadEnabled;              ///< Flag indicating a started background process and to be used to friendly close the process.
    std::atomic<bool>   threadIsRunning;            ///< Flag indicating a running and not yet finished background process.
    std::atomic<bool>   isConnectedToServer;        ///< Indicates a successful connect process until a server sided disconnect or client side disconnecting process.
    std::thread*        pThread;                    ///< Class instance representing the background task.
    bool                async;                      ///< Asynchronous call of callback objects
    int                 lastConnError;              ///< Flag indicating the last printed connection failure.
};

}

#endif

/** @} */
