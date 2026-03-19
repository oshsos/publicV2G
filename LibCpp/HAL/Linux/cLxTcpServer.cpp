// utf-8 (ü)
/**
\file cLxTcpServer.cpp

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2023-01-11

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL_Linux
@{

\class LibCpp::cLxTcpServer
\brief Virtual class for data transmission using TCP protocol from the server side

\code
#include <stdio.h>

#include "../LibCpp/HAL/Linux/conio.h"
#include "../LibCpp/HAL/Linux/cLxTcpServer.h"

#define BUFFERSIZE 256

using namespace LibCpp;
using namespace std;

char buffer[BUFFERSIZE];
int bufferLen = 0;
cLxConnectedClient* pClient = nullptr;

int main()
{
    cLxTcpServer server;
    server.open();

    printf("Start (press 'q' to finish, 's' to send)\n");
    fflush(stdout);
    char input = 0;
    while (1)
    {
        if (!pClient)
        {
            pClient = server.acceptClient();
        }

        if (pClient)
        {
            bufferLen = pClient->receive(buffer, BUFFERSIZE);
            if (bufferLen>0)
            {
                buffer[BUFFERSIZE-1] = 0; // To be absolutely shure of zero termination
                printf("Received message: %s", buffer);
                fflush(stdout);
            }
            else if (bufferLen == -1)
            {
                delete pClient;
                pClient = nullptr;
            }
        }

        if (_kbhit())
        {
            input = getchar();
            printf("\n");
            fflush(stdout);
            if (input == 'q')
                break;
            if (input == 's')
            {
                if (pClient)
                {
                    const char* message = "Hallo";
                    printf("Send message: %s\n", message);
                    fflush(stdout);
                    pClient->send(message, strlen(message));
                }
            }
        }
    }

    delete pClient;
    server.close();

    printf("Ready\n");
    fflush(stdout);
    return 0;
}
\endcode
**/

#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if_link.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
//#include <limits.h>
#include <sys/ioctl.h>
#include <linux/filter.h>
#include <net/if.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <thread>
#include <cstring>
#include <string.h>
#include <chrono>

#include "cLxTcpServer.h"
#include "../Tools.h"

using namespace LibCpp;
using namespace std;

/**
* @brief Constructor
* Hardware specific implementation of the LibCpp::cConnectedClient class
*/
cLxConnectedClient::cLxConnectedClient(bool open, int clientSocket, unSockAddr clientSockAddr, int ifIndex, enDebugLevel dbgLevel, bool showMessages) :
    dbg("cLxConnectedClient", dbgLevel)
{
    cDebug dbg("cLxConnectedClient", &this->dbg);

    this->clientSocket = clientSocket;
    this->clientSockAddress = clientSockAddr;
    this->ifIndex = ifIndex;
    this->showMessages = showMessages;
    messageLen = 0;
    pThread = 0;
    threadEnabled = false;
    finishedThread = false;
    async = false;

    if (open)
    {
        cLxConnectedClient::open();
        dbg.printf(enDebugLevel_Info, "Client connected from remote address %s.", this->getIpAddressRemote().toString(true).c_str());
    }
    else
        dbg.printf(enDebugLevel_Info, "Client connected from remote address %s (not opened!).", this->getIpAddressRemote().toString(true).c_str());
}

/**
 * @brief Destructor
 */
cLxConnectedClient::~cLxConnectedClient()
{
    cDebug dbg("~cLxConnectedClient", &this->dbg);

    threadEnabled = false;
    if (pThread)
    {
        pThread->join();
        delete pThread;
        pThread = nullptr;
    }
    shutdown(clientSocket, SHUT_RDWR);
    close(clientSocket);
    dbg.printf(enDebugLevel_Debug, "Disconnecton (3/3): Client destroyed.\n");
}

/**
 * @brief Check for the client to be in active operation
 * See LibCpp::cConnectedClient::open for further details.
 * @return
 */
bool cLxConnectedClient::open()
{
    if (threadEnabled)
        return true;

    // Setting socket options
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 500 * 1000;
    int onValue = 1;
    if (setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof (struct timeval)) ||
        setsockopt(clientSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof (struct timeval)) ||
        setsockopt(clientSocket, IPPROTO_TCP, TCP_NODELAY, &onValue, sizeof(int)) ||
        setsockopt(clientSocket, IPPROTO_TCP, TCP_QUICKACK, &onValue, sizeof(int)) )
    {
        dbg.printf(enDebugLevel_Fatal, "Client socket options failed!\n");
    };

    int flags = fcntl(clientSocket, F_GETFL, 0);
    if (fcntl(clientSocket, F_SETFL, flags | O_NONBLOCK) != 0)
    {
        dbg.printf(enDebugLevel_Error, "%s\n", enIpResult_toString(enIpResult_SocketOptionFailed).c_str());
    }

    finishedThread = false;
    threadEnabled = true;  // setting within the tread is not fast enough, if 'receive' is called after construction.
    pThread = new std::thread(&cLxConnectedClient::threadReceiving, this);
    return true;
}

/**
 * @brief Checks for the client to be in active operation.
 * @return
 */
bool cLxConnectedClient::isOpen() {return threadEnabled;}

/**
 * @brief Prepares for closing by friendly closing the background task.
 * See LibCpp::cConnectedClient::prepareClose for further details.
 * @return 'true' if background task has finished.
 */
bool cLxConnectedClient::prepareClose()
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
int cLxConnectedClient::send(const char* message, int messageLen, enBlocking blockingMode)
{
    if (threadEnabled)
    {
        if (showMessages && dbg.getDebugLevel()<=enDebugLevel_Debug)
        {
            cDebug dbg("send", &this->dbg);
            dbg.printf(enDebugLevel_Debug, "\n%s", ByteArrayToString(message, messageLen).c_str());
        }
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
int cLxConnectedClient::receive(char* buffer, int bufferSize, enBlocking blockingMode)
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
    if (len>0)
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
int cLxConnectedClient::receiveBuffer(char** ppBuffer, enBlocking blockingMode)
{
    *ppBuffer = message;
    return messageLen;
}

/**
 * @brief Frees the receive Buffer
 * Method is to be called after calling LibCpp::cLxUdpSocket::receiveBuffer to enable the reception of further messages.
 */
void cLxConnectedClient::receiveAcknowledge()
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
stIpAddress cLxConnectedClient::getIpAddressRemote()
{
    return cLxIp::instance().fromSockAddr(&clientSockAddress, ifIndex);
}

/**
 * @brief Method to be called cycelic to synchronously call registered callbacks
 * This type of operation mode is a polling method.
 */
void cLxConnectedClient::operate()
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
void cLxConnectedClient::internalOperate()
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
int cLxConnectedClient::getSocketDescriptor()
{
    return clientSocket;
}

/**
 * @brief Receiving background thread.
 * @return
 */
void cLxConnectedClient::threadReceiving()
{
    cDebug dbg("threadReceiving", &this->dbg);

    threadEnabled = true;
    int read_size = 1;

    while(threadEnabled && read_size>0)
    {
        dbg.setDebugLevel(this->dbg.getDebugLevel());
        /* Note: timeout must be (re)set every time before call to select() */
        struct   timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 500;
        fd_set fdSet;
        FD_ZERO(&fdSet);
        FD_SET((unsigned int)clientSocket, &fdSet);
        int selectResult = select(clientSocket+1, &fdSet, NULL, NULL, &tv);

        if (selectResult>0)
        {
            // Receive a message from client
            if (!pExternalSndRcv)
                read_size = recv(clientSocket , message , LibCpp_cIp_TCP_BUFFERSIZE , 0);
            else
                read_size = pExternalSndRcv->onExternalReceive(message, LibCpp_cIp_TCP_BUFFERSIZE);
            if (read_size>0)
            {
                // Ensure zero termination of message
                if (read_size < LibCpp_cIp_TCP_BUFFERSIZE)
                    message[read_size] = 0;
                // Message output
                if (showMessages && dbg.getDebugLevel()<=enDebugLevel_Debug)
                {
                    dbg.printf(enDebugLevel_Debug, "\n%s", ByteArrayToString(message, read_size).c_str());
                }
                messageLen = read_size;
                if (async)
                {
                    internalOperate();
                }
                while (messageLen > 0 && threadEnabled)  // wait for message buffer to get read
                    this_thread::sleep_for(chrono::milliseconds(1));
            }
            else if ((read_size == -2) || ((read_size == -1) && (errno == EAGAIN  || errno == EWOULDBLOCK)))  // EINTR = 4 ist eventuell auch abzufangen, -2 necessary for external snd / rcv.
            {   // timeout for thread termination polling
                read_size = 1;
            }
            else if (read_size <= 0)
            {
                if (read_size == 0)
                    dbg.printf(enDebugLevel_Debug, "Disconnection (1/3): Client requested disconnection.");
                else
                    dbg.printf(enDebugLevel_Debug, "Disconnection (1/3): Receiving failed with '%i'! Disconnecting the client.", errno);
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
cLxTcpServer::cLxTcpServer(bool open, int localPort, string localInterfaceNameAddress, enAddressFamily family, bool linkLocal, int acceptedClients, bool openClients) :
    dbg("cLxTcpServer")
{
    newConnectedClient = nullptr;
    serverSocket = (int)INVALID_SOCKET;
    pThread = nullptr;
    this->openClients = openClients;
    showMessages = false;
    cLxTcpServer::close();
    if (open)
        cLxTcpServer::open(localPort, localInterfaceNameAddress, family, linkLocal, acceptedClients);
}

/**
 * @brief Destructor
 */
cLxTcpServer::~cLxTcpServer()
{
//    if (newConnectedClient)
//        delete newConnectedClient;
    cLxTcpServer::close();
}

/**
 * @brief Prepares for closing by friendly closing the background task.
 * See LibCpp::cWinTcpServer::prepareClose for further details.
 * @return 'true' if background task has finished.
 */
void cLxTcpServer::prepareClose()
{
    threadEnabled = false;
}

/**
 * @brief Frees the ressources and finishes the background task
 */
void cLxTcpServer::close()
{
    threadEnabled = false;
    if (pThread)
    {
        pThread->join();
        delete pThread;
        pThread = nullptr;
    }
    if (serverSocket > INVALID_SOCKET)
    {
        ::close(serverSocket);
        serverSocket = INVALID_SOCKET;
        stIpAddress addr = cLxTcpServer::getIpAddressLocal();
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
enIpResult cLxTcpServer::open(int localPort, string localInterfaceOrAddressName, enAddressFamily family, bool linkLocal, int acceptedClients, bool openClients)
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
            dbg.printf(enDebugLevel_Error, "No ethernet interface available! Check for external hardware to be plugged in.");
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
            dbg.printf(enDebugLevel_Error, "'%s' is not a valid address or interface name! Check cable connection and check with command 'ipconfig' on command line.", localInterfaceOrAddressName.c_str());
            return enIpResult_InvalidIpAddress;
        }
    }
    localIpAddr.port = localPort;
    ifIndex = localIpAddr.ifId;

    // The only necessary localSockAddress and ifIndex is known at this stage!
    serverSockAddress = cLxIp::instance().toSockAddr(&localIpAddr, &ifIndex);

    int af_family = serverSockAddress.addr.sa_family;

    // Create socket
    serverSocket = socket(af_family , SOCK_STREAM , 0);
    if (serverSocket == -1)
    {
        cLxTcpServer::close();
        //dbg.printf(enDebugLevel_Error, "Socket could not be opened!");
        printf("%s\n", enIpResult_toString(enIpResult_SocketFailed).c_str());
        return enIpResult_SocketFailed;
    }

    // Setting socket options
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    int reuse = 1;
    int onValue = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof (struct timeval)) ||
        setsockopt(serverSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof (struct timeval)) ||
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) ||
        setsockopt(serverSocket, IPPROTO_TCP, TCP_NODELAY, &onValue, sizeof(int)) ||
        setsockopt(serverSocket, IPPROTO_TCP, TCP_QUICKACK, &onValue, sizeof(int)) )
    {
        cLxTcpServer::close();
        dbg.printf(enDebugLevel_Error, "%s", enIpResult_toString(enIpResult_SocketOptionFailed).c_str());
        return enIpResult_SocketOptionFailed;
    };

    // SO_KEEPALIVE https://stackoverflow.com/questions/4142012/how-to-find-the-socket-connection-state-in-c

    unsigned long NonBlock = 1;
    if (ioctl(serverSocket, FIONBIO, &NonBlock) == SOCKET_ERROR)
    {
        cLxTcpServer::close();
        dbg.printf(enDebugLevel_Error, "Setting 'NonBlock' failed. %s", enIpResult_toString(enIpResult_SocketOptionFailed).c_str());
        return enIpResult_SocketOptionFailed;
    }
    // fcntl(socket, F_SETFL, O_NONBLOCK);

    // Bind
    if (bind(serverSocket, &serverSockAddress.addr, sizeof(unSockAddr)) < 0)
    {
        cLxTcpServer::close();
        //dbg.printf(enDebugLevel_Error, "Socket could not be opened!");
        printf("%s\n", enIpResult_toString(enIpResult_BindFailed).c_str());
        return enIpResult_BindFailed;
    }
    socklen_t len = sizeof(unSockAddr);
    getsockname(serverSocket, &serverSockAddress.addr, &len);

    // Listen
    if (acceptedClients <= 0) acceptedClients = SOMAXCONN;
    if (listen(serverSocket, acceptedClients) == -1)
    {
        cLxTcpServer::close();
        dbg.printf(enDebugLevel_Error, "%s", enIpResult_toString(enIpResult_ListenFailed).c_str());
        return enIpResult_ListenFailed;
    }

    pThread = new std::thread(&cLxTcpServer::threadAccepting, this);

    stIpAddress addr = cLxTcpServer::getIpAddressLocal();
    dbg.printf(enDebugLevel_Info, "Listening at local address %s", addr.toString(true, true).c_str());
    return enIpResult_Success;
}

cConnectedClient* cLxTcpServer::newClient()
{
    cDebug dbg("newClient", &this->dbg);

    cLxConnectedClient* pClient = nullptr;
    if (newConnectedClient)
    {
        pClient = newConnectedClient;
        newConnectedClient = nullptr;
        dbg.printf(enDebugLevel_Debug, "Connection (2/2): Application accepted new client.");
    }
    return pClient;
}

void cLxTcpServer::threadAccepting()
{
    cDebug dbg("cLxTcpServer::threadAccepting");

    threadEnabled = true;

    int clientSocket;
    unSockAddr clientAddress;
    socklen_t sockaddrlen;

    // dbg.printf(enDebugLevel_Debug, "Waiting for connections...");
    while (threadEnabled)
    {
        sockaddrlen = sizeof(unSockAddr);
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &sockaddrlen);
        if (clientSocket < 0)
        {
            if (errno == EAGAIN  || errno == EWOULDBLOCK)
            {
                // dbg.printf(enDebugLevel_Debug, "Timeout");
            }
            else
            {
                dbg.printf(enDebugLevel_Error, "Accepting client failed with %i!", errno);
                threadEnabled = false;
            }
        }
        else
        {
            dbg.printf(enDebugLevel_Debug, "Connection (1/2): Client connection accepted.");
            newConnectedClient = new cLxConnectedClient(openClients, clientSocket, clientAddress, ifIndex, this->dbg.getDebugLevel(), showMessages);

            while (newConnectedClient != nullptr && threadEnabled)
            {
                this_thread::sleep_for(chrono::milliseconds(10));
            }
            //dbg.printf(enDebugLevel_Debug, "Waiting for connections...");
        }
    }
    dbg.printf(enDebugLevel_Debug, "Accepting thread terminated.");
}

stIpAddress cLxTcpServer::getIpAddressLocal()
{
    return cLxIp::instance().fromSockAddr(&serverSockAddress, ifIndex);
}

/** @} */
