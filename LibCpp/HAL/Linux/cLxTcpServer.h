// UTF8 (ü) //
/**
\file   cLxTcpServer.h

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2024-03-27
\brief  See cLxTcpServer.cpp

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL_Linux
@{

\class LibCpp::cWinTcpServer
**/

#ifndef CLXTCPSERVER_H
#define CLXTCPSERVER_H

#ifdef PROJECTDEFINES
    #include <ProjectDefines>
#endif

#ifndef BibCpp_cLxTcpServer_MAXCLIENTS
    #define BibCpp_cLxTcpServer_MAXCLIENTS 10
#endif

#include <string>

#include <cstdint>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>

#include "atomic.h"
#include "cLxIp.h"
#include "../cDebug.h"
#include "../cTcpServer.h"

namespace LibCpp
{

/**
 * @brief Hardware specific LibCpp::cConnectedClient class
 */
class cLxConnectedClient : public cConnectedClient
{
public:
    cLxConnectedClient(bool open, int clientSocket, unSockAddr clientSockAddr, int ifIndex, enDebugLevel dbgLevel, bool showMessages);   ///< Constructor
    ~cLxConnectedClient();                                  ///< Destructor

    bool open();                                            ///< Check for the client to be in active operation
    bool isOpen();                                          ///< Checks for the client to be in active operation.
    bool prepareClose();                                    ///< Prepares for closing by friendly closing the background task.

    int  send(const char* pMsg = nullptr, int messageLen = 0, enBlocking blockingMode = enBlocking_BUFFER);             ///> Sends a data frame.
    int  receive(char* pMsg = nullptr, int bufferSize = 0, enBlocking blockingMode = enBlocking_NONE);                  ///> Receives a data frame.
    int  receiveBuffer(char** ppMsg = nullptr, enBlocking blockingMode = enBlocking_NONE);                              ///> Receives a data frame without copying
    void receiveAcknowledge();                              ///> Acknowledges a reception of a message by 'receiveBuffer'
    void operate();                                         ///> Cyclic operation on callback polling mode usage.

    int  getSocketDescriptor();                             ///> Returns the socket file descriptor
    stIpAddress getIpAddressRemote();                       ///< Retrieves the ip address of the remote client host

private:
    void internalOperate();                                 ///< Handles calls to registered callback objects
    void threadReceiving();                                 ///< Receiving thread

public:
    cDebug              dbg;                                ///< Logger intstance
    bool                showMessages;                       ///< Prints all messages in case the debug level is 'debug'

    int                 ifIndex;                            ///< Interface id the object is receiving on
    int                 clientSocket;                       ///< Socket the object is using
    unSockAddr          clientSockAddress;                  ///< Remote client ip Address
    char                message[LibCpp_cIp_TCP_BUFFERSIZE]; ///< Receive buffer
    std::atomic<int>    messageLen;                         ///< Message length in the buffer
    std::atomic<bool>   threadEnabled;                      ///< Flag to finish the background thread
    std::atomic<bool>   finishedThread;                     ///< Flag indicating a finished thread
    std::thread*        pThread;                            ///< Thread object
    bool                async;                              ///< Flag to indicate reception by callback will be handled by the background thread.
};

/**
 * @brief Hardware specific LibCpp::cTcpServer class
 */
class cLxTcpServer : public cTcpServer
{
public:
    cLxTcpServer(bool open = false, int localPort = LibCpp_cIp_TCP_PORT, std::string localInterfaceOrAddressName = std::string(""), enAddressFamily family = enAddressFamily_IPv4, bool linkLocal = true, int acceptedClients = 0, bool openClients = true);  ///< Constructor
    ~cLxTcpServer();                                        ///< Destructor

    enIpResult           open(int localPort = LibCpp_cIp_UDP_PORT, std::string localInterfaceNameAddress = "", enAddressFamily family = enAddressFamily_IPv4, bool linkLocal = true, int acceptedClients = 0, bool openClients = true);   ///< Sets the server in operational mode.
    void                 prepareClose();                    ///< Prepares for closing by friendly closing the background task.
    void                 close();                           ///< Frees recources and finishes the background task
    cConnectedClient*    newClient();                       ///< Method to check for new connected client hosts (accept)
    stIpAddress          getIpAddressLocal();               ///< Deliveres the interface ip address the UDP socket is bound to.
    int                  getPort();                         ///< Deliveres the port the object is listening on.

    void                 threadAccepting();                 ///< Thread to accept further clients.

public:
    cDebug dbg;                                             ///< Logger intstance

//private:
    int                  ifIndex;                           ///< Interface id the object is receiving on
    unSockAddr           serverSockAddress;                 ///< Ip Address the server is connected to for receiving
    int                  serverSocket;                      ///< Socket the server is accepting on.
    bool                 openClients;                       ///< If true, all new clients are automatically opened.

    std::atomic<cLxConnectedClient*> newConnectedClient;    ///< Buffer for newly accepted clients
    std::atomic<bool>    threadEnabled;                     ///< Flag to finish the background thread
    std::thread*         pThread;                           ///< Thread object
};

}

#endif

/** @} */
