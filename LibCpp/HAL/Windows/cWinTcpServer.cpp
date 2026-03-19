// UTF8 (ü) //
/**
\file cWinTcpServer.cpp

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2023-01-10

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL
@{

\class LibCpp::cWinTcpServer
\brief Windows implementation of the \ref G_LibCpp_HAL LibCpp::cTcpServer class.

// Link with Iphlpapi.lib
// Link with Ws2_32.lib
// Für Qt (*.pro):
//LIBS += $$PWD/LibCpp/HAL/Windows/iphlpapi.lib
//LIBS += $$PWD/LibCpp/HAL/Windows/ws2_32.lib

//C:/Users/Olaf Simon>ipconfig

//Windows-IP-Konfiguration


//Drahtlos-LAN-Adapter LAN-Verbindung* 1:

//   Medienstatus. . . . . . . . . . . : Medium getrennt
//   Verbindungsspezifisches DNS-Suffix:

//Drahtlos-LAN-Adapter LAN-Verbindung* 2:

//   Medienstatus. . . . . . . . . . . : Medium getrennt
//   Verbindungsspezifisches DNS-Suffix:

//Ethernet-Adapter VMware Network Adapter VMnet1:

//   Verbindungsspezifisches DNS-Suffix:
//   Verbindungslokale IPv6-Adresse  . : fe80::52cc:24f9:afc:a9dd%13
//   IPv4-Adresse  . . . . . . . . . . : 192.168.134.1
//   Subnetzmaske  . . . . . . . . . . : 255.255.255.0
//   Standardgateway . . . . . . . . . :

//Ethernet-Adapter VMware Network Adapter VMnet8:

//   Verbindungsspezifisches DNS-Suffix:
//   Verbindungslokale IPv6-Adresse  . : fe80::a30e:8447:d845:a8c6%19
//   IPv4-Adresse  . . . . . . . . . . : 192.168.19.1
//   Subnetzmaske  . . . . . . . . . . : 255.255.255.0
//   Standardgateway . . . . . . . . . :



HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Tcpip\Parameters\Interfaces\<Interface GUID>
Entry: TcpAckFrequency
Value Type: REG_DWORD, number
Valid Range: 0-255
Default: 2
Description: Specifies the number of ACKs that will be outstanding before the delayed ACK timer is ignored.
If you set the value to 1, every packet is acknowledged immediately.

 */

#ifdef PROJECTDEFS
    #include <ProjectDefs.h>
#endif

#ifndef LibCpp_cTcpServer_MAXCLIENTS
    #define LibCpp_cTcpServer_MAXCLIENTS 10
#endif

#undef UNICODE
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#include "cWinTcpServer.h"  // To be included before other system includes

//#include <ws2tcpip.h>
//#include <netioapi.h>
//#include <iphlpapi.h>
//#include <stdlib.h>
//#include <stdio.h>
//#include <string>

#include <cstring>
#include <chrono>

#include "../Tools.h"

using namespace LibCpp;
using namespace std;

/**
* @brief Constructor
* Hardware specific implementation of the LibCpp::cConnectedClient class
*/
cWinConnectedClient::cWinConnectedClient(bool open, int clientSocket, unSockAddr clientSockAddr, int ifIndex, enDebugLevel dbgLevel, bool showMessages) :
    dbg("cWinConnectedClient", dbgLevel)
{
    cDebug dbg("cWinConnectedClient", &this->dbg);

    this->clientSocket = clientSocket;
    this->clientSockAddress = clientSockAddr;
    this->showMessages = showMessages;
    this->ifIndex = ifIndex;
    messageLen = 0;
    pThread = 0;
    showMessages = false;
    threadEnabled = false;
    finishedThread = false;
    async = false;

    if (open)
    {
        cWinConnectedClient::open();
        dbg.printf(enDebugLevel_Info, "Client connected from remote address %s.", this->getIpAddressRemote().toString(true).c_str());
    }
    else
        dbg.printf(enDebugLevel_Info, "Client connected from remote address %s (not opened!).", this->getIpAddressRemote().toString(true).c_str());
}

/**
 * @brief Destructor
 */
cWinConnectedClient::~cWinConnectedClient()
{
    cDebug dbg("~cWinConnectedClient", &this->dbg);

    threadEnabled = false;
    if (pThread)
    {
        pThread->join();
        delete pThread;
        pThread = nullptr;
    }
    shutdown(clientSocket, SD_BOTH);
    closesocket(clientSocket);
    dbg.printf(enDebugLevel_Debug, "Disconnection (3/3): Client destroyed.");
}

/**
 * @brief Check for the client to be in active operation
 * See LibCpp::cConnectedClient::open for further details.
 * @return
 */
bool cWinConnectedClient::open()
{
    if (threadEnabled)
        return true;

    // Setting socket options
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 500;
    int value = 1;
    if (setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof (struct timeval)) ||
        setsockopt(clientSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof (struct timeval)) ||
        setsockopt(clientSocket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof (int)) != 0)
    {
        dbg.printf(enDebugLevel_Fatal, "Client socket options failed!\n");
    };

    unsigned long nonBlocking = 1;
    if (ioctlsocket(clientSocket, FIONBIO, &nonBlocking) != 0)
    {
        dbg.printf(enDebugLevel_Error, "%s\n", enIpResult_toString(enIpResult_SocketOptionFailed).c_str());
    }

    finishedThread = false;
    threadEnabled = true;  // setting within the tread is not fast enough, if 'receive' is called after construction.
    pThread = new std::thread(&cWinConnectedClient::threadReceiving, this);
    return true;
}

/**
 * @brief Checks for the client to be in active operation.
 * @return
 */
bool cWinConnectedClient::isOpen() {return threadEnabled;}

/**
 * @brief Prepares for closing by friendly closing the background task.
 * See LibCpp::cConnectedClient::prepareClose for further details.
 * @return 'true' if background task has finished.
 */
bool cWinConnectedClient::prepareClose()
{
    threadEnabled = false;
    return finishedThread;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
/**
 * @brief Sends a message to the remote client.
 * @param message
 * @param messageLen
 * @param blockingMode
 * @return Len of actually sent bytes, -1 on failure.
 */
int cWinConnectedClient::send(const char* message, int messageLen, enBlocking blockingMode)
{
    if (threadEnabled)
    {
        if (showMessages)
            dbg.printf(enDebugLevel_Debug, "Sent     %4u bytes:\n%s", messageLen, ByteArrayToString(message, messageLen).c_str());
        //Send the message to the client
        if (!pExternalSndRcv)
            return ::send(clientSocket, message, messageLen, 0);
        else
            return pExternalSndRcv->onExternalSend(message, messageLen);
    }
    return 0;
}

/**
 * @brief Receives a message
 * @param buffer
 * @param bufferSize
 * @param blockingMode Not implemented, method is not blocking
 * @return Length of received message, 0 if no message is present, -1 in case the client disconnected (only once), -2 if no client is present
 */
int cWinConnectedClient::receive(char* buffer, int bufferSize, enBlocking blockingMode)
{
    cDebug dbg("receive", &this->dbg);

    if (!threadEnabled)
    {
        if (messageLen == -1)
        {
            dbg.printf(enDebugLevel_Debug, "Disconnection (2/3): Application informed about client requested disconnection.");
        }
        messageLen = -2;
        return -1;
    }
    int len = messageLen;
    if (len!=0)
    {
        if (len>bufferSize) len=bufferSize;
        memcpy(buffer, message, len);
        if (len < bufferSize-1) buffer[len]=0;
        if (!callbacks.size())
            messageLen = 0;
    }
    return len;
}

/**
 * @brief Receives a message without copying from the receive buffer
 * In case the message is processed, call LibCpp::cLxUdpSocket::receiveAcknowledge to free the receive buffer.
 * @param ppBuffer
 * @param blockingMode blockingMode Not implemented, method is not blocking
 * @return Length of received message, 0 if no message is present, -1 in case the client disconnected (only once), -2 if no client is present
 */
int cWinConnectedClient::receiveBuffer(char** ppBuffer, enBlocking blockingMode)
{
    *ppBuffer = message;
    return messageLen;
}

/**
 * @brief Frees the receive Buffer
 * Method is to be called after calling LibCpp::cWinUdpSocket::receiveBuffer to enable the reception of further messages.
 */
void cWinConnectedClient::receiveAcknowledge()
{
    if (!callbacks.size())
        if (messageLen>0)              // do not access for writing, if the receiving task has control of the buffer
            messageLen = 0;
}
#pragma GCC diagnostic warning "-Wunused-parameter"

/**
 * @brief Retrieves the IP address of the remote host represented by this object
 * @return
 */
stIpAddress cWinConnectedClient::getIpAddressRemote()
{
    return cWinIp::instance().fromSockAddr(&clientSockAddress, ifIndex);
}

/**
 * @brief Method to be called cycelic to synchronously call registered callbacks
 * This type of operation mode is a polling method.
 */
void cWinConnectedClient::operate()
{
    if (!clientSocket || async)
        return;
    internalOperate();
}

/**
 * @brief Method to call registered callback objects
 * The method is called either by 'operate' or the background thread
 * depending of 'async' is unset or set.
 */
void cWinConnectedClient::internalOperate()
{
    if (messageLen && callbacks.size())
    {
        for (iFramePort* call : callbacks)
            call->onFrameReceive((cFramePort*)this);
        messageLen = 0;
    }
}

/**
 * @brief Returns the socket file descriptor
 * @return
 */
int cWinConnectedClient::getSocketDescriptor()
{
    return clientSocket;
}

/**
 * @brief Receiving background thread.
 * @return
 */
void cWinConnectedClient::threadReceiving()
{
    cDebug dbg("threadReceiving", &this->dbg);

    threadEnabled = true;
    int read_size = 1;

    while(threadEnabled && read_size>0)
    {
        struct   timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 500;
        fd_set fdSet;
        FD_ZERO(&fdSet);
        FD_SET((unsigned int)clientSocket, &fdSet);
        int selectResult = select(clientSocket+1, &fdSet, NULL, NULL, &tv);

        if (selectResult>0)
        {
            // Receive a message from the client
            if (!pExternalSndRcv)
                read_size = recv(clientSocket , message , LibCpp_cIp_TCP_BUFFERSIZE , 0);
            else
                read_size = pExternalSndRcv->onExternalReceive(message, LibCpp_cIp_TCP_BUFFERSIZE);
            // int err = WSAGetLastError();
            if (read_size>0)
            {
                // Ensure zero termination of message
                if (read_size < LibCpp_cIp_TCP_BUFFERSIZE)
                    message[read_size] = 0;
                // Message output
                if (showMessages)
                    dbg.printf(enDebugLevel_Debug, "Received %4u bytes:\n%s", read_size, ByteArrayToString(message, read_size).c_str());
                messageLen = read_size;
                if (async)
                {
                    internalOperate();
                }
                while (messageLen > 0 && threadEnabled)  // wait for message buffer to get read
                    this_thread::sleep_for(chrono::milliseconds(1));
            }
            //else if ((read_size == -1) && (errno == EAGAIN  || errno == EWOULDBLOCK))
            else if ((read_size == -2) || ((read_size == -1) && (errno == EWOULDBLOCK || errno == EINTR || errno == NOERROR)))   // -2 necessary for external snd / rcv.; || err == WSAEWOULDBLOCK
            {   // timeout for thread termination polling
                read_size = 1;
            }
            else if (read_size <= 0)
            {
                if (read_size == 0)
                    dbg.printf(enDebugLevel_Debug, "Disconnection (1/3): Client requested disconnection.");
                else
                {
                    char* buffer;
                    int err = WSAGetLastError();
                    if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, err, 0, (LPTSTR)&buffer, 128, nullptr) > 0)
                    {
                        dbg.printf(enDebugLevel_Debug, "Disconnection (1/3): Receiving failed with %i: '%s'! Disconnecting the client.", err, buffer);
                    }
                    else
                        dbg.printf(enDebugLevel_Debug, "Disconnection (1/3): Receiving failed with '%i'! Disconnecting the client.", err);
                }
                threadEnabled = false;
            }
        }
        else if (selectResult<0)
        {
            dbg.printf(enDebugLevel_Error, "Select call returned with error %i: %s!\n", errno, strerror(errno));
            threadEnabled = false;
        }
    }
    messageLen = -1;
    finishedThread = true;
    return;
}

/**
 * @brief Constructor
 */
cWinTcpServer::cWinTcpServer(bool open, int localPort, string localInterfaceNameAddress, enAddressFamily family, bool linkLocal, int acceptedClients, bool openClients) :
    dbg("cWinTcpServer")
{
    showMessages = false;
    newConnectedClient = nullptr;
    serverSocket = (int)INVALID_SOCKET;
    pThread = nullptr;
    this->openClients = openClients;
    cWinTcpServer::close();
    if (open)
        cWinTcpServer::open(localPort, localInterfaceNameAddress, family, linkLocal, acceptedClients);
}

/**
 * @brief Destructor
 */
cWinTcpServer::~cWinTcpServer()
{
//    if (newConnectedClient)
//        delete newConnectedClient;
    cWinTcpServer::close();
}

/**
 * @brief Prepares for closing by friendly closing the background task.
 * See LibCpp::cWinTcpServer::prepareClose for further details.
 * @return 'true' if background task has finished.
 */
void cWinTcpServer::prepareClose()
{
    threadEnabled = false;
}

/**
 * @brief Frees the ressources and finishes the background task
 */
void cWinTcpServer::close()
{
    threadEnabled = false;
    if (pThread)
    {
        pThread->join();
        delete pThread;
        pThread = nullptr;
    }
    if (serverSocket > (int)INVALID_SOCKET)
    {
        ::closesocket(serverSocket);
        serverSocket = (int)INVALID_SOCKET;
        stIpAddress addr = cWinTcpServer::getIpAddressLocal();
        dbg.printf(enDebugLevel_Info, "Closed listening server from local address %s", addr.toString(true, true).c_str());
    }
    ifIndex = INVALID_INTERFACE;
    memset(&serverSockAddress, 0, sizeof(unSockAddr));
    if (newConnectedClient)
        delete newConnectedClient;
    newConnectedClient = nullptr;
}

/**
 * @brief cWinTcpServer::open
 * Opens the TCP socket and starts the background accepting task.
 * @param localPort
 * @param localInterfaceOrAddressName
 * @param family
 * @param linkLocal
 * @param acceptedClients
 * @return
 */
enIpResult cWinTcpServer::open(int localPort, string localInterfaceOrAddressName, enAddressFamily family, bool linkLocal, int acceptedClients, bool openClients)
{
    cDebug dbg("open", &this->dbg);

    this->openClients = openClients;

    stIpAddress localIpAddr;
    enIpResult  res;
    cInterfaces interfaces(true);

    if (interfaces.getInterfacesResult != enIpResult_Success)
    {
        if (interfaces.getInterfacesResult == enIpResult_NoInterface)
        {
            dbg.printf(enDebugLevel_Fatal, "No ethernet interface available! Check for external hardware to be plugged in.");
            return interfaces.getInterfacesResult;
        }
        dbg.printf(enDebugLevel_Fatal, "Ethernet interfaces could not be retrieved from the host! %s", enIpResult_toString(interfaces.getInterfacesResult).c_str());
        return interfaces.getInterfacesResult;
    }

    if (localInterfaceOrAddressName.empty())
    {
        localIpAddr.family = family;
    }
    else
    {
        res = interfaces.getInterfaceIpAddress(localIpAddr, localInterfaceOrAddressName, family, linkLocal);
        if (res != enIpResult_Success)
        {
            dbg.printf(enDebugLevel_Fatal, "'%s' is not a valid address or interface name! Check cable connection and check with command 'ipconfig' on command line.");
            return enIpResult_InvalidIpAddress;
        }
    }
    localIpAddr.port = localPort;
    ifIndex = localIpAddr.ifId;

    // The only necessary localSockAddress and ifIndex is known at this stage!
    serverSockAddress = cWinIp::instance().toSockAddr(&localIpAddr, &ifIndex);

    int af_family = serverSockAddress.addr.sa_family;

    // Create socket
    serverSocket = (int)socket(af_family , SOCK_STREAM , 0);
    if (serverSocket == -1)
    {
        cWinTcpServer::close();
        dbg.printf(enDebugLevel_Fatal, "%s", enIpResult_toString(enIpResult_SocketFailed).c_str());
        return enIpResult_SocketFailed;
    }

    // Setting socket options
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    int reuse = 1;
    int value = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof (struct timeval)) ||
        setsockopt(serverSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof (struct timeval)) ||
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) ||
        setsockopt(serverSocket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof (int)))
    {
        cWinTcpServer::close();
        dbg.printf(enDebugLevel_Error, "%s", enIpResult_toString(enIpResult_SocketOptionFailed).c_str());
        return enIpResult_SocketOptionFailed;
    };

    // SO_KEEPALIVE https://stackoverflow.com/questions/4142012/how-to-find-the-socket-connection-state-in-c

    ULONG NonBlock = 1;
    if (ioctlsocket(serverSocket, FIONBIO, &NonBlock) == SOCKET_ERROR)
    {
        cWinTcpServer::close();
        dbg.printf(enDebugLevel_Error, "Setting 'NonBlock' failed. %s", enIpResult_toString(enIpResult_SocketOptionFailed).c_str());
        return enIpResult_SocketOptionFailed;
    }

    // fcntl(socket, F_SETFL, O_NONBLOCK);
    // WSAIoctl() and FONBIO),

    // Bind
    if (bind(serverSocket, &serverSockAddress.addr, sizeof(unSockAddr)) < 0)
    {
        cWinTcpServer::close();
        dbg.printf(enDebugLevel_Fatal, "%s", enIpResult_toString(enIpResult_BindFailed).c_str());
        return enIpResult_BindFailed;
    }
    int len = sizeof(unSockAddr);
    getsockname(serverSocket, &serverSockAddress.addr, &len);

    // Listen
    if (acceptedClients <= 0) acceptedClients = SOMAXCONN;
    if (listen(serverSocket, acceptedClients) == -1)
    {
        cWinTcpServer::close();
        dbg.printf(enDebugLevel_Fatal, "%s", enIpResult_toString(enIpResult_ListenFailed).c_str());
        return enIpResult_ListenFailed;
    }

    pThread = new std::thread(&cWinTcpServer::threadAccepting, this);

    stIpAddress addr = cWinTcpServer::getIpAddressLocal();
    dbg.printf(enDebugLevel_Info, "Listening at local address %s", addr.toString(true, true).c_str());
    return enIpResult_Success;
}

cConnectedClient* cWinTcpServer::newClient()
{
    cDebug dbg("newClient", &this->dbg);

    cWinConnectedClient* pClient = nullptr;
    if (newConnectedClient)
    {
        pClient = newConnectedClient;
        newConnectedClient = nullptr;
        dbg.printf(enDebugLevel_Debug, "Connection (2/2): Application accepted new client.");
    }
    return pClient;
}

void cWinTcpServer::threadAccepting()
{
    cDebug dbg("cWinTcpServer::threadAccepting", this->dbg.getDebugLevel());

    threadEnabled = true;

    int clientSocket;
    unSockAddr clientAddress;
    socklen_t sockaddrlen;

    // dbg.printf(enDebugLevel_Debug, "Waiting for connections...");
    while (threadEnabled)
    {
        sockaddrlen = sizeof(unSockAddr);
        clientSocket = (int)accept(serverSocket, &clientAddress.addr, &sockaddrlen);
        if (clientSocket < 0)
        {
            int err = WSAGetLastError();
            if (err == WSAEWOULDBLOCK)
            {
                // dbg.printf(enDebugLevel_Debug, "Timeout");
            }
            else
            {
                if (threadEnabled)
                {
                    if (err == 10093)
                        dbg.printf(enDebugLevel_Error, "It is required to 'close' the object in case it is globally instanciated!");
                    else
                        dbg.printf(enDebugLevel_Error, "Accepting client failed with %i!", err);
                }
                threadEnabled = false;
            }
        }
        else
        {
            dbg.printf(enDebugLevel_Debug, "Connection (1/2): Client connection accepted.");
            newConnectedClient = new cWinConnectedClient(openClients, clientSocket, clientAddress, ifIndex, this->dbg.getDebugLevel(), showMessages);
//            ((cConnectedClient*)newConnectedClient)->dbg.setDebugLevel(dbg.getDebugLevel());
//            ((cConnectedClient*)newConnectedClient)->showMessages = showMessages;

            while (newConnectedClient != nullptr && threadEnabled)
            {
                this_thread::sleep_for(chrono::milliseconds(10));
            }
            //dbg.printf(enDebugLevel_Debug, "Waiting for connections...");
        }
        this_thread::sleep_for(chrono::milliseconds(10));
    }
    dbg.printf(enDebugLevel_Debug, "Accepting thread terminated.");
}

stIpAddress cWinTcpServer::getIpAddressLocal()
{
    return cWinIp::instance().fromSockAddr(&serverSockAddress, ifIndex);
}

/** @} */
