// utf-8 (ü)
/**
\file cLxTcpClient.cpp

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2023-03-27

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL_Linux
@{

\class LibCpp::cLxTcpClient
\brief Windows implementation of the \ref G_LibCpp_HAL hardware abstraction layer interface LibCpp::cLxClient class


\code
#include <stdio.h>

#define BUFFERSIZE 256

using namespace LibCpp;
using namespace std;

char buffer[BUFFERSIZE];
int bufferLen = 0;

int main()
{
    cDebug dbg("main", 0, enDebugLevel_Debug);

    printf("Test Client\n");
    printf("'q' to finish\n");
    printf("'s' to send\n");
    printf("'o' to open\n");
    printf("'c' to close\n");
    fflush(stdout);

    cLxTcpClient client;
    client.dbg.setInstanceName("client", &dbg);

    client.open();

    if (client.isConnected())
        printf("Client connected.\n");
    else
        printf("Client not connected!\n");
    printf("Running...\n");
    fflush(stdout);

    char input = 0;
    while (1)
    {
        bufferLen = client.receive(buffer, BUFFERSIZE);
        if (bufferLen>0)
        {
            buffer[BUFFERSIZE-1] = 0; // To be absolutely shure of zero termination
            printf("Received message: %s", buffer);
            fflush(stdout);
        }
        if (bufferLen == -1)
        {
            printf("Server disconnected.\n");
            fflush(stdout);
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
                const char* message = "Hallo";
                printf("Send message: %s\n", message);
                fflush(stdout);
                client.send(message, strlen(message));
            }
            if (input == 'o')
            {
                if (client.open() == enIpResult_Success)
                    printf("Connected.\n");
                else
                    printf("Opening client failed!\n");
                fflush(stdout);
            }
            if (input == 'c')
            {
                client.close();
                printf("Client closed.\n");
            }
        }
    }

    client.close();

    printf("Ready\n");
    fflush(stdout);
    return 0;
}
\endcode
**/

#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
//#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if_link.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <limits.h>
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
//#include <cstring>
#include <string>
#include <chrono>

#include "../cDebug.h"
#include "cLxTcpClient.h"

using namespace LibCpp;
using namespace std;

/**
 * @brief Constructor
 * @param open
 * @param serverPort
 * @param serverAddressName
 * @param localInterfaceOrAddressName
 * @param family
 * @param linkLocal
 * @param async
 * @param pCallback
 */
cLxTcpClient::cLxTcpClient(bool open, int serverPort, std::string serverAddressName, int localPort, std::string localInterfaceOrAddressName, bool async, iFramePort* pCallback) :
    dbg("cLxTcpClient")
{
    pThread = nullptr;
    localSocket = INVALID_SOCKET;
    threadEnabled = false;
    lastConnError = 0;
    cLxTcpClient::close();
    if (pCallback)
        setCallback(pCallback);
    if (open)
        cLxTcpClient::open(serverPort, serverAddressName, localPort, localInterfaceOrAddressName, async);
}

/**
 * @brief Destructor
 * Closes and destroyes the client.
 */
cLxTcpClient::~cLxTcpClient()
{
    cLxTcpClient::close();
}

/**
 * @brief Prepares for closing by friendly closing the background task.
 * This way several objects requiring to finish a background task can do so in parallel.
 * Just calling 'close' requires much time as each 'close' needs to wait for the backgrond task actually
 * having finished.
 */
bool cLxTcpClient::prepareClose()
{
    threadEnabled = false;
    return !threadIsRunning;
}

/**
 * @brief Closes or releases hardware ressources.
 * See LibCpp::cUdpSocket::close.
 */
void cLxTcpClient::close()
{
    threadEnabled = false;
    if (pThread)
    {
        pThread->join();
        delete pThread;
        pThread = nullptr;
        //dbg.printf(enDebugLevel_Info, "Disconnected.\n");
    }
    if (localSocket > INVALID_SOCKET)
    {
        shutdown(localSocket, SHUT_RDWR);
        ::close(localSocket);
        localSocket = INVALID_SOCKET;
    }
    isConnectedToServer = false;
    threadIsRunning = false;
    memset(&serverSockAddress, 0, sizeof(unSockAddr));
    messageLen = -2;
}

bool cLxTcpClient::isClosed()
{
    return !threadIsRunning;
}

/**
 * @brief Opens the client by reserving a socket and starting the background process
 * @param serverPort
 * @param serverAddressName
 * @param async
 * @return
 */
enIpResult cLxTcpClient::open(int serverPort, string serverAddressName, int localPort, std::string localInterfaceOrAddressName, bool async)
{
    cDebug dbg("open", &this->dbg);

    if (isConnectedToServer)
    {
        stIpAddress connectedServer = cLxTcpClient::getIpAddressServer();
        stIpAddress requestedServer = stIpAddress(serverAddressName, serverPort);
        if (requestedServer == connectedServer)
            return enIpResult_Success;
    }

    cLxTcpClient::close();

    if (serverAddressName.empty())
        serverAddressName = "127.0.0.1";

    stIpAddress serverIpAddress;
    cLxIp::instance().getIpAddress(serverIpAddress, serverAddressName, serverPort);
    serverSockAddress = cLxIp::instance().toSockAddr(&serverIpAddress);

    int af_family = serverSockAddress.addr.sa_family;

    // Create socket Inet
    localSocket = socket(af_family , SOCK_STREAM , 0);
    if (localSocket == -1)
    {
        dbg.printf(enDebugLevel_Fatal, "%s: %i %s!", enIpResult_toString(enIpResult_SocketFailed).c_str(), errno, strerror(errno));
        cLxTcpClient::close();
        return enIpResult_SocketFailed;
    }

    // Setting socket options
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 500 * 1000;
    int reuse = 1;
    int onValue = 1;
    if (
        setsockopt(localSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof (struct timeval)) ||
        setsockopt(localSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof (struct timeval)) ||
        setsockopt(localSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) ||
        setsockopt(localSocket, IPPROTO_TCP, TCP_NODELAY, &onValue, sizeof(int)) ||
        setsockopt(localSocket, IPPROTO_TCP, TCP_QUICKACK, &onValue, sizeof(int)) )
    {
        dbg.printf(enDebugLevel_Error, "%s\n", enIpResult_toString(enIpResult_SocketOptionFailed).c_str());
        cLxTcpClient::close();
        return enIpResult_SocketOptionFailed;
    };

    int flags = fcntl(localSocket, F_GETFL, 0);
    if (fcntl(localSocket, F_SETFL, flags | O_NONBLOCK) != 0)
    {
        dbg.printf(enDebugLevel_Error, "%s\n", enIpResult_toString(enIpResult_SocketOptionFailed).c_str());
        cLxTcpClient::close();
        return enIpResult_SocketOptionFailed;
    }

    // Bind (optional)
    if (!localInterfaceOrAddressName.empty())
    {
        cInterfaces interfaces(true);

        if (interfaces.getInterfacesResult != enIpResult_Success)
        {
            if (interfaces.getInterfacesResult == enIpResult_NoInterface)
            {
                dbg.printf(enDebugLevel_Fatal, "No ethernet interface available! Check for external hardware to be plugged in.");
                cLxTcpClient::close();
                return interfaces.getInterfacesResult;
            }
            dbg.printf(enDebugLevel_Fatal, "Ethernet interfaces could not be retrieved from the host! %s", enIpResult_toString(interfaces.getInterfacesResult).c_str());
            cLxTcpClient::close();
            return interfaces.getInterfacesResult;
        }

        stIpAddress localIpAddr;
        enIpResult res = interfaces.getInterfaceIpAddress(localIpAddr, localInterfaceOrAddressName, serverIpAddress.family, true);
        if (res != enIpResult_Success)
        {
            dbg.printf(enDebugLevel_Fatal, "'%s' is not a valid address or interface name! Check cable connection and check with command 'ipconfig' on command line.");
            cLxTcpClient::close();
            return enIpResult_InvalidIpAddress;
        }
        localIpAddr.port = localPort;

        unSockAddr localSockAddress = cLxIp::instance().toSockAddr(&localIpAddr);
        serverIpAddress.ifId = localIpAddr.ifId;

        if (bind(localSocket, &localSockAddress.addr, sizeof(unSockAddr)) < 0)
        {
            cLxTcpClient::close();
            dbg.printf(enDebugLevel_Fatal, "%s (%i) %s", enIpResult_toString(enIpResult_BindFailed).c_str(), errno, strerror(errno));
            return enIpResult_BindFailed;
        }

        if (af_family == AF_INET6)
            serverSockAddress.in6.sin6_scope_id = localSockAddress.in6.sin6_scope_id;
    }

    this->async = async;
    threadEnabled = true;       // To be fast enough in case a direct 'receive' follows after 'open'
    threadIsRunning = true;
    pThread = new std::thread(&cLxTcpClient::threadReceiving, this);

    return enIpResult_Success;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"

/**
 * @brief Sends a message to the server
 * @param message
 * @param messageLen
 * @param blockingMode
 * @return Len of actually sent bytes, -1 on failure (e.g. being unconnected).
 */
int cLxTcpClient::send(const char* message, int messageLen, enBlocking blockingMode)
{
    if (threadEnabled)
    {
        //Send the message to the server
        if (!pExternalSndRcv)
            return ::send(localSocket, message, messageLen, 0);
        else
            return pExternalSndRcv->onExternalSend(message, messageLen);
    }
    return -1;
}

/**
 * @brief Receives a message
 * @param buffer
 * @param bufferSize
 * @param blockingMode Not implemented, method is not blocking
 * @return Number of bytes received.
 */
int cLxTcpClient::receive(char* buffer, int bufferSize, enBlocking blockingMode)
{
    cDebug dbg("receive", &this->dbg);

    int len = messageLen;
    if (len<=0)
    {
        if (threadEnabled) return 0;
        if (len==0 || len == -1)
        {
            dbg.printf(enDebugLevel_Debug, "Parent thread informed about disconnected server.");
            messageLen = -2;
            return -1;
        }
        else
        {
            return 0;
        }
    }
    if (len>0)
    {
        if (len>bufferSize) len=bufferSize;
        memcpy(buffer, message, len);
        if (len < bufferSize-1) buffer[len]=0;
        messageLen = 0;
    }
    return len;
}

/**
 * @brief Receives a message without copying from the receive buffer
 * In case the message is processed, call LibCpp::cLxUdpSocket::receiveAcknowledge to free the receive buffer.
 * @param ppBuffer
 * @param blockingMode blockingMode Not implemented, method is not blocking
 * @return Number of bytes available in the receive buffer.
 */
int cLxTcpClient::receiveBuffer(char** ppBuffer, enBlocking blockingMode)
{
    *ppBuffer = message;
    return messageLen;
}

/**
 * @brief Frees the receive Buffer
 * Method is to be called after calling LibCpp::cLxUdpSocket::receiveBuffer to enable the reception of further messages.
 */
void cLxTcpClient::receiveAcknowledge()
{
    if (messageLen>0 && !callbacks.size())              // do not access for writing, if the receiving task has control of the buffer
        messageLen = 0;
}


/**
 * @brief Method to be called cycelic to synchronously call registered callbacks
 * This type of operation mode is a polling method.
 */
void cLxTcpClient::operate()
{
    if (!localSocket || async)
        return;
    internalOperate();
}

/**
 * @brief Method to call registered callback objects
 * The method is called either by 'operate' or the background thread
 * depending of 'async' is unset or set.
 */
void cLxTcpClient::internalOperate()
{
    if (messageLen && callbacks.size())
    {
        for (iFramePort* call : callbacks)
            call->onFrameReceive((cFramePort*)this);
        messageLen = 0;
    }
}

/**
 * @brief Checks whether the client is connected to the server
 * @return
 */
bool cLxTcpClient::isConnected()
{
    return isConnectedToServer;
}

/**
 * @brief Retrieves the source address of the last received message.
 * The method is to be called after a call to any receive methods.
 * @return
 */
stIpAddress cLxTcpClient::getIpAddressServer()
{
    return cLxIp::instance().fromSockAddr(&serverSockAddress);
}

/**
 * @brief Returns the socket file descriptor
 * @return
 */
int cLxTcpClient::getSocketDescriptor()
{
    return localSocket;
}

/**
 * @brief Background thread for receiving messages
 */
void cLxTcpClient::threadReceiving()
{
    cDebug dbg("threadReceiving", &this->dbg);

    threadEnabled = true;
    threadIsRunning = true;
    isConnectedToServer = false;

    int read_size = 1;

    if (!lastConnError)
        dbg.printf(enDebugLevel_Info, "Trying to connect to %s ...", getIpAddressServer().toString(true, true).c_str());

    // Connect (on a non blocking) -> Receive and send timeout set
    do
    {
        if (::connect(localSocket, &serverSockAddress.addr, sizeof(unSockAddr)) != 0)
        {
            int err = errno;
            if (!threadEnabled) err = -1;

            if (err == EINPROGRESS || err == EALREADY)
            {
                usleep(1000);
            }
            else if (err == EISCONN)
            {
                isConnectedToServer = true;
            }
            else
            {
                if (err != lastConnError && err != -1)
                {
                    lastConnError = errno;
                    dbg.printf(enDebugLevel_Info, "%s %i %s\n", enIpResult_toString(enIpResult_ConnectFailed).c_str(), errno, strerror(errno));
                }
                threadEnabled = false;
                isConnectedToServer = false;
                threadIsRunning = false;
                return;
            }
        }
    }
    while (!isConnectedToServer);

    dbg.printf(enDebugLevel_Info, "Connected to server %s.", getIpAddressServer().toString(true, true).c_str());
    messageLen = 0;     // clear possibly pending disconnection process states
    lastConnError = 0;

    if (pExternalSndRcv)
    {
        if (!pExternalSndRcv->onPostConnect())
        {
            threadEnabled = false;
            messageLen = -1;
        }
        else
            isConnectedToServer = true;
    }
    else
        isConnectedToServer = true;

    while(threadEnabled && read_size>0)
    {
        // Receive a message from server
        /* Note: timeout must be (re)set every time before call to select() */
        struct   timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 500;
        fd_set fdSet;
        FD_ZERO(&fdSet);
        FD_SET((unsigned int)localSocket, &fdSet);
        int selectResult = select(localSocket+1, &fdSet, NULL, NULL, &tv);

        if (selectResult>0)
        {
            // Receive a message from the server
            if (!pExternalSndRcv)
                read_size = recv(localSocket , message , LibCpp_cIp_TCP_BUFFERSIZE , 0);
            else
                read_size = pExternalSndRcv->onExternalReceive(message, LibCpp_cIp_TCP_BUFFERSIZE);
            if (read_size>0)
            {
                // Ensure zero termination of message
                if (read_size < LibCpp_cIp_TCP_BUFFERSIZE)
                    message[read_size] = 0;
                messageLen = read_size;
            }
            else if ((read_size == -2) || ((read_size == -1) && (errno == EAGAIN  || errno == EWOULDBLOCK)))  // EINTR = 4 ist eventuell auch abzufangen, -2 necessary for external snd / rcv.
            {   // timeout for thread termination polling
                read_size = 1;
            }
            else if (read_size <= 0)
            {
                threadEnabled = false;
                messageLen = -1;
                if (read_size == 0)
                    dbg.printf(enDebugLevel_Info, "Server %s disconnected.\n", getIpAddressServer().toString(true, true).c_str());
                else
                    dbg.printf(enDebugLevel_Error, "Receiving failed with %i '%s'! Closing.\n", errno, strerror(errno));
            }
            if (async)
                internalOperate();
            while (messageLen > 0 && threadEnabled)  // wait for message buffer to get read
                this_thread::sleep_for(chrono::milliseconds(1));
        }
        else if (selectResult<0)
        {
            dbg.printf(enDebugLevel_Error, "Select call returned with error %i: %s!\n", errno, strerror(errno));
            threadEnabled = false;
        }
    }

    if (messageLen >= 0)
        dbg.printf(enDebugLevel_Info, "Client disconnected from %s.\n", getIpAddressServer().toString(true, true).c_str());
    isConnectedToServer = false;
    threadIsRunning = false;
    return;
}

/** @} */
