/**
\file cWinUdpSocket.cpp

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2023-01-05

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL_Windows
@{

\class LibCpp::cWinUdpSocket
\brief Windows implementation of the \ref G_LibCpp_HAL hardware abstraction layer interface LibCpp::cUdpSocket class

// Link with Iphlpapi.lib
// Link with Ws2_32.lib
// Für Qt (*.pro):
//LIBS += $$PWD/LibCpp/HAL/Windows/iphlpapi.lib
//LIBS += $$PWD/LibCpp/HAL/Windows/ws2_32.lib

//C:\Users\Olaf Simon>ipconfig

// use 'SocketTest' for testing.


cWinUdpSocket port;
port.open();
port.send("sendBuffer"Hallo", 5);

int len = 0;
char receiveBuffer[1024];
while (!len)
{
    len = port.receive(receiveBuffer, 1024);
    printf("%s", receiveBuffer);
}

port.open(55000, "WLAN");

----------------------

#include <stdio.h>
#include <conio.h>

#include "../LibCpp/HAL/Windows/cWinUdpSocket.h"
#include "../LibCpp/HAL/Tools.h"

using namespace LibCpp;


cWinUdpSocket port;


int main()
{
    cDebug dbg("main");
    dbg.setDebugLevel();
    port.dbg.setInstanceName("port", &dbg);

    port.open(55000, "WLAN");

    char receiveBuffer[1024];
    while (1)
    {
        int len = port.receive(receiveBuffer, 1024);
        if (len)
            printf_flush("Received: %s\n", receiveBuffer);

        if (_kbhit())
        {
            input = getch();
            if (input == 'q')
                break;
            if (input == 's')
            {
                const char* sendBuffer = "Hallo";
                port.send(sendBuffer, strlen(sendBuffer));
                printf_flush("Sent    : %s\n", sendBuffer);
            }
        }
    }
    printf_flush("\nready\n");
    return 0;



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
**/

//#undef UNICODE
//#define WIN32_LEAN_AND_MEAN

#define _CRT_SECURE_NO_WARNINGS

#include "cWinUdpSocket.h"      // To be included before other system includes

#include <chrono>

using namespace LibCpp;
using namespace std;

/**
 * @brief Constructor
 */
cWinUdpSocket::cWinUdpSocket(bool open, int localPort, string localInterfaceNameAddress, enAddressFamily family, bool linkLocal, bool async, iFramePort* pCallback) :
    dbg("cWinUdpSocket")
{
    pThread = nullptr;
    localSocket = (int)INVALID_SOCKET;
    cWinUdpSocket::close();
    if (pCallback)
        setCallback(pCallback);
    if (open)
        cWinUdpSocket::open(localPort, localInterfaceNameAddress, family, linkLocal, async);
}

/**
 * @brief Destructor
 */
cWinUdpSocket::~cWinUdpSocket()
{
    cWinUdpSocket::close();
}

/**
 * @brief Prepares for closing by friendly closing the background task.
 * This way several objects requiring to finish a background task can do so in parallel.
 * Just calling 'close' requires much time as each 'close' needs to wait for the backgrond task actually
 * having finished.
 */
void cWinUdpSocket::prepareClose()
{
    threadEnabled = false;
}

/**
 * @brief Closes or releases hardware ressources.
 * See LibCpp::cUdpSocket::close.
 */
void cWinUdpSocket::close()
{
    threadEnabled = false;
    if (pThread)
    {
        pThread->join();
        delete pThread;
        pThread = nullptr;
    }
    if (localSocket > (int)INVALID_SOCKET)
    {
        shutdown(localSocket, SD_BOTH);
        closesocket(localSocket);
        localSocket = (int)INVALID_SOCKET;
    }
    ifIndex = INVALID_INTERFACE;
    memset(&localSockAddress, 0, sizeof(unSockAddr));
    memset(&broadcastSockAddress, 0, sizeof(unSockAddr));
    memset(&sourceSockAddress, 0, sizeof(unSockAddr));
    memset(&messageSockAddress, 0, sizeof(unSockAddr));
    messageLen = 0;
    async = false;
}

/**
 * @brief Opens or aquires hardware ressources.
 * See LibCpp::cUdpSocket::open.
 * @param localPort
 * @param localInterfaceOrAddressName
 * @param family
 * @param linkLocal If set and IPv6, only link local addresses (fe80:...) are accepted. Otherwise any address is taken.
 * @return Error code
 */
enIpResult cWinUdpSocket::open(int localPort, string localInterfaceOrAddressName, enAddressFamily family, bool linkLocal, bool async)
{
    cDebug dbg("open", &this->dbg);

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

    if (localInterfaceOrAddressName.empty() || localInterfaceOrAddressName == "0.0.0.0")
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

    // The only necessary localSockAddress (including port) and ifIndex is known at this stage!
    localSockAddress = cWinIp::instance().toSockAddr(&localIpAddr, &ifIndex);

    int af_family = localSockAddress.addr.sa_family;

    // Create socket
    localSocket = (int)socket(af_family , SOCK_DGRAM , 0);
    if (localSocket == -1)
    {
        dbg.printf(enDebugLevel_Fatal, "%s: %i %s!", enIpResult_toString(enIpResult_SocketFailed).c_str(), errno, strerror(errno));
        cWinUdpSocket::close();
        return enIpResult_SocketFailed;
    }

    // Setting socket options
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    int broadcast = 1;
    int reuse = 1;

    //if (setsockopt(localSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof (struct timeval)) ||
    if (setsockopt(localSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof (struct timeval)) ||
        setsockopt(localSocket, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(int)) ||
        setsockopt(localSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) )
    {
        dbg.printf(enDebugLevel_Fatal, "%s", enIpResult_toString(enIpResult_SocketOptionFailed).c_str(), errno, strerror(errno));
        cWinUdpSocket::close();
        return enIpResult_SocketOptionFailed;
    };

    unsigned long nonBlocking = 1;
    if (ioctlsocket(localSocket, FIONBIO, &nonBlocking) != 0)
    {
        dbg.printf(enDebugLevel_Error, "%s\n", enIpResult_toString(enIpResult_SocketOptionFailed).c_str());
        cWinUdpSocket::close();
        return enIpResult_SocketOptionFailed;
    }

    if (af_family == AF_INET)
    {
        DWORD loopch = 1;
        if (setsockopt(localSocket, IPPROTO_IP, IP_MULTICAST_LOOP, &loopch, sizeof(loopch)))                        // This option has different default values under Linux and Windows, thus explicit setting is necessary.
        {
            dbg.printf(enDebugLevel_Fatal, "IPv4 Proto options: %s", enIpResult_toString(enIpResult_SocketOptionFailed).c_str());
            cWinUdpSocket::close();
            return enIpResult_SocketOptionFailed;
        };
    }
    else if (af_family == AF_INET6)
    {
        // Scope id (interface id) must be set for IPv6
        // in6addr_any is required for Linux (IN6ADDR_ANY_INIT does not work for broadcast over the network), but must not be set at Windows.
        // do not set: localSockAddress.sin6_addr = in6addr_any;        // in6addr_loopback muss noch aus der IPv6 ::1 adresse generiert werden.
        // erledigt mit toSockAddr()! localSockAddress.in6.sin6_scope_id = ifIndex;

        // Set local interface for outbound multicast datagrams.
        DWORD loopch = 1;
        int on = 0;
        if (setsockopt(localSocket, IPPROTO_IPV6, IPV6_MULTICAST_IF, &ifIndex, sizeof(ifIndex)) ||
            setsockopt(localSocket, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &loopch, sizeof(loopch)) ||
            setsockopt(localSocket, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on)) )                        // This option has different default values under Linux and Windows, thus explicit setting is necessary.
        {
            dbg.printf(enDebugLevel_Fatal, "IPv6 Proto options: %s", enIpResult_toString(enIpResult_SocketOptionFailed).c_str());
            cWinUdpSocket::close();
            return enIpResult_SocketOptionFailed;
        };
    };

    // bind (for receiving operations)
    if (bind(localSocket, &localSockAddress.addr, sizeof(unSockAddr)) < 0)
    {
        dbg.printf(enDebugLevel_Fatal, "%s '%s': %i %s!", enIpResult_toString(enIpResult_BindFailed).c_str(), localInterfaceOrAddressName.c_str(), errno, strerror(errno));
        cWinUdpSocket::close();
        return enIpResult_BindFailed;
    }

    if (af_family == AF_INET)
    {
        // Prepare broadcast socket address for sending operations
        struct sockaddr_in* pBroadAddr = &broadcastSockAddress.in;
        pBroadAddr->sin_family = AF_INET;
        LibCpp::inet_pton(AF_INET, "255.255.255.255", &pBroadAddr->sin_addr);
        pBroadAddr->sin_port = htons(localPort);

        // Initializing destination address
        stIpAddress destAddr;
        if (!ifIndex && localInterfaceOrAddressName.empty())
        {
            stInterfaceInfo ifcInfo = interfaces.getInterfaceInfo("", localIpAddr.family, linkLocal);
            destAddr = ifcInfo.ipMulticast;
            destAddr.port = localPort;
            if (destAddr.family != enAddressFamily_None)
                destinationSockAddress = cWinIp::instance().toSockAddr(&destAddr);
        }
        else if (ifIndex)
        {
            stInterfaceInfo ifc = interfaces.getInterfaceInfo(ifIndex, enAddressFamily_IPv4);
            destAddr = ifc.ipMulticast;
            destAddr.port = localPort;
            destinationSockAddress = cWinIp::instance().toSockAddr(&destAddr);
        }
    }
    else if (af_family == AF_INET6)
    {
        // Prepare broadcast socket address for sending operations
        broadcastSockAddress.in6.sin6_family = AF_INET6;
        LibCpp::inet_pton(AF_INET6, "ff02::1", &broadcastSockAddress.in6.sin6_addr);
        broadcastSockAddress.in6.sin6_port = htons(localPort);
        broadcastSockAddress.in6.sin6_scope_id = ifIndex;

        // setting multicast (broadcast) group membership
        struct ipv6_mreq group;
        LibCpp::inet_pton(AF_INET6, "ff02::1", &group.ipv6mr_multiaddr);
        group.ipv6mr_interface = ifIndex;
        if (//setsockopt(localSocket, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, &group, sizeof(struct ipv6_mreq)) || // IPV6_JOIN_GROUP
            setsockopt(localSocket, IPPROTO_IPV6, IPV6_JOIN_GROUP, &group, sizeof(struct ipv6_mreq))) // IPV6_JOIN_GROUP
        {
            int err = WSAGetLastError();
            dbg.printf(enDebugLevel_Fatal, "%s Setting group membership failed with code %i!", enIpResult_toString(enIpResult_SocketOptionFailed).c_str(), err);
            cWinUdpSocket::close();
            return enIpResult_SocketOptionFailed;
        };

        // Initializing destination address
        destinationSockAddress = broadcastSockAddress;
    }

    stIpAddress addr = cWinUdpSocket::getIpAddressLocal();
    stInterfaceInfo ifc = interfaces.getInterfaceInfo(ifIndex);
    dbg.printf(enDebugLevel_Info, "Listening at interface '%s' local address %s", ifc.frientlyName.c_str(), addr.toString().c_str());

    this->async = async;

    threadEnabled = true;     // To be fast enough in case a direct 'receive' follows after 'open'
    pThread = new std::thread(&cWinUdpSocket::threadReceiving, this);
    return enIpResult_Success;
}

/**
 * @brief cWinUdpSocket::setDestination
 * @param pDestination
 * @param port If port is set, it overrules and sets the port defined within the stIpAddress of 'destination'.
 * @return Error code: enIpResult_Success, enIpResult_InvalidIpAddress
 */
enIpResult cWinUdpSocket::setDestination(stIpAddress* pDestination, int port)
{
    cDebug dbg("setDestination", &this->dbg);

    if (!pDestination)
    {
        destinationSockAddress = sourceSockAddress;
        return enIpResult_Success;
    }
    if (port) pDestination->port = port;
    if (pDestination->family == enAddressFamily_None)
    {
        dbg.printf(enDebugLevel_Error, "Address %s is not valid!", pDestination->toString().c_str());
        return enIpResult_InvalidIpAddress;
    }
    if (!pDestination->port) pDestination->port = getLocalPort();
    destinationSockAddress = cWinIp::instance().toSockAddr(pDestination);
    dbg.printf(enDebugLevel_Info, "Destination set to: %s", pDestination->toString().c_str());
    return enIpResult_Success;
}

/**
 * @brief cWinUdpSocket::setDestination
 * @param pDestination
 * @param port If port is set, it overrules the port defined within the stIpAddress of 'destination'.
 * @return Error code: enIpResult_Success, enIpResult_InvalidIpAddress
 */
enIpResult cWinUdpSocket::setDestination(stIpAddress destination, int port)
{
    return setDestination(&destination, port);
}

/**
 * @brief Sets a group address the host is listening on.
 * @param groupAddress
 */
void cWinUdpSocket::joinGroup(stIpAddress groupAddress)
{
    cDebug dbg("joinGroup", &this->dbg);
    if (!localSocket)
    {
        dbg.printf(enDebugLevel_Error, "Method must be called, after object is opened!");
        return;
    }
    unSockAddr groupSockAddr = cWinIp::instance().toSockAddr(&groupAddress);
    if (localSockAddress.addr.sa_family !=  groupSockAddr.addr.sa_family)
    {
        dbg.printf(enDebugLevel_Error, "Group address familiy must be equal to local address family!");
        return;
    }
    if (groupAddress.family == enAddressFamily_IPv6)
    {
        struct ipv6_mreq group;
        for (int i=0; i<8; i++)
            group.ipv6mr_multiaddr.u.Word[i] = groupSockAddr.in6.sin6_addr.u.Word[i];
        group.ipv6mr_interface = ifIndex;

        if(setsockopt(localSocket, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, &group, sizeof(struct ipv6_mreq))) // IPV6_JOIN_GROUP
        {
            int err = WSAGetLastError();
            dbg.printf(enDebugLevel_Fatal, "%s Setting group membership failed with code %i!", enIpResult_toString(enIpResult_SocketOptionFailed).c_str(), err);
            close();
            return;
        }
    }
    else if (groupAddress.family == enAddressFamily_IPv4)
    {
        struct ip_mreq group;
        group.imr_multiaddr.S_un.S_addr = groupSockAddr.in.sin_addr.S_un.S_addr;
        group.imr_interface.S_un.S_addr = localSockAddress.in.sin_addr.S_un.S_addr;
        //group.imr_multiaddr.S_un.S_addr = inet_addr("224.0.0.11");
        //group.imr_interface.S_un.S_addr = inet_addr("192.168.178.59");

        if(setsockopt(localSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &group, sizeof(struct ip_mreq))) // IP_JOIN_GROUP
        {
            int err = WSAGetLastError();
            dbg.printf(enDebugLevel_Fatal, "%s Setting group membership failed with code %i!", enIpResult_toString(enIpResult_SocketOptionFailed).c_str(), err);
            close();
            return;
        }
    }
}

/**
 * @brief Deliveres the port the object is listening on.
 * @return
 */
int cWinUdpSocket::getLocalPort()
{
    if (localSockAddress.addr.sa_family == AF_INET)
    {
        return htons(localSockAddress.in.sin_port);
    }
    if (localSockAddress.addr.sa_family == AF_INET6)
    {
        return htons(localSockAddress.in6.sin6_port);
    }
    return 0;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
/**
 * @brief Sends a message to a unSockAddr type destination.
 * @param message
 * @param messageLen
 * @param destination
 * @param blockingMode
 * @return Len of actually sent bytes, -1 on failure.
 */
int cWinUdpSocket::sendToInternal(const char* message, int messageLen, unSockAddr destination, enBlocking blockingMode)
{
    if (threadEnabled)
    {
        return ::sendto(localSocket, message, messageLen, 0, &destination.addr, sizeof(unSockAddr));
    }
    return -1;
}

/**
 * @brief Sends a message to the internal set destination.
 * @param message
 * @param messageLen
 * @param blockingMode
 * @return Len of actually sent bytes, -1 on failure.
 */
int cWinUdpSocket::send(const char* message, int messageLen, enBlocking blockingMode)
{
    return sendToInternal(message, messageLen, destinationSockAddress, blockingMode);
}

/**
 * @brief Sends a message to a stIpAddress type destination.
 * @param message
 * @param messageLen
 * @param destination
 * @param blockingMode
 * @return Len of actually sent bytes, -1 on failure.
 */
int cWinUdpSocket::sendTo(const char* message, int messageLen, stIpAddress destination, enBlocking blockingMode)
{
    return sendToInternal(message, messageLen, cWinIp::instance().toSockAddr(&destination), blockingMode);
}

/**
 * @brief Receives a message
 * @param buffer
 * @param bufferSize
 * @param blockingMode Not implemented, method is not blocking
 * @return Number of bytes received.
 */
int cWinUdpSocket::receive(char* buffer, int bufferSize, enBlocking blockingMode)
{
    cDebug dbg("receive", &this->dbg);
    int len = messageLen;
    if (len>0)
    {
        if (len>bufferSize) len=bufferSize;
        memcpy(buffer, message, len);
        if (len < bufferSize-1) buffer[len]=0;
        sourceSockAddress = messageSockAddress;
        if (!callbacks.size())
            messageLen = 0;
    }
    return len;
}

/**
 * @brief Receives a message and the message source
 * @param buffer
 * @param bufferSize
 * @param source
 * @param blockingMode blockingMode Not implemented, method is not blocking
 * @return Number of bytes received.
 */
int cWinUdpSocket::receiveFrom(char* buffer, int bufferSize, stIpAddress& source, enBlocking blockingMode)
{
    char* data;
    int len = receiveBuffer(&data);
    if (len<=0)
        return len;
    source = cWinIp::instance().fromSockAddr(&sourceSockAddress);
    return receive(buffer, bufferSize, blockingMode);
}

/**
 * @brief Receives a message without copying from the receive buffer
 * In case the message is processed, call LibCpp::cLxUdpSocket::receiveAcknowledge to free the receive buffer.
 * @param ppBuffer
 * @param blockingMode blockingMode Not implemented, method is not blocking
 * @return Number of bytes available in the receive buffer.
 */
int cWinUdpSocket::receiveBuffer(char** ppBuffer, enBlocking blockingMode)
{
    *ppBuffer = message;
    int len = messageLen;
    if (len)
        sourceSockAddress = messageSockAddress;
    return len;
}

/**
 * @brief Frees the receive Buffer
 * Method is to be called after calling LibCpp::cLxUdpSocket::receiveBuffer to enable the reception of further messages.
 */
void cWinUdpSocket::receiveAcknowledge()
{
    if (messageLen>0 && !callbacks.size())              // do not access for writing, if the receiving task has control of the buffer
        messageLen = 0;
}

/**
 * @brief Sends a message to all hosts within the network
 * @param message
 * @param messageLen
 * @param blockingMode blockingMode Not implemented, method is not blocking
 * @return Number of actually sent bytes, -1 on failure.
 */
int cWinUdpSocket::broadcast(const char* message, int messageLen, enBlocking blockingMode)
{
    if (threadEnabled)
    {
        return ::sendto(localSocket, message, messageLen, 0, (struct sockaddr *)&broadcastSockAddress, sizeof (struct sockaddr_in6));
    }
    return -1;
}
#pragma GCC diagnostic warning "-Wunused-parameter"

/**
 * @brief Method to be called cycelic to synchronously call registered callbacks
 * This type of operation mode is a polling method.
 */
void cWinUdpSocket::operate()
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
void cWinUdpSocket::internalOperate()
{
    if (messageLen && callbacks.size())
    {
        for (iFramePort* call : callbacks)
            call->onFrameReceive((cFramePort*)this);
        messageLen = 0;
    }
}

/**
 * @brief Background thread for socket message reception
 */
void cWinUdpSocket::threadReceiving()
{
    cDebug dbg("threadReceiving", &this->dbg);

    threadEnabled = true;
    int read_size = 1;

    while(threadEnabled)
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
            int addrLen6 = sizeof(struct sockaddr_in6);
            read_size = recvfrom(localSocket , message , LibCpp_cIp_UDP_BUFFERSIZE , 0, (struct sockaddr *)&messageSockAddress, &addrLen6);
            if (read_size>0)
            {
                // Ensure zero termination of message
                if (read_size < LibCpp_cIp_UDP_BUFFERSIZE)
                    message[read_size] = 0;
                messageLen = read_size;
            }
            else if ((read_size == -1) && !(errno == NOERROR  || errno == EWOULDBLOCK || errno == EINTR))  // EINTR = 4 wird beim Debugging benötigt
            {
                dbg.printf(enDebugLevel_Error, "Receiving failed with code %i '%s'! Closing.", errno, strerror(errno));
                threadEnabled = false;
            }
            if (messageLen > 0)
            {
                if (async)
                {
                    internalOperate();
                }
                while (messageLen > 0 && threadEnabled)  // wait for message buffer to get read
                    this_thread::sleep_for(chrono::milliseconds(1));
            }
        }
        else if (selectResult<0)
        {
            dbg.printf(enDebugLevel_Error, "Select call returned with error %i: %s!\n", errno, strerror(errno));
            threadEnabled = false;
        }
    }
    return;
}

/**
 * @brief Retrieves the local host IP address the object is receiving on.
 * @return
 */
stIpAddress cWinUdpSocket::getIpAddressLocal()
{
    return cWinIp::instance().fromSockAddr(&localSockAddress, ifIndex);
}

/**
 * @brief Retrieves the standard destination address.
 * @return
 */
stIpAddress cWinUdpSocket::getIpAddressDestination()
{
    return cWinIp::instance().fromSockAddr(&destinationSockAddress);
}

/**
 * @brief Retrieves the source address of the last received message.
 * The method is to be called after a call to any receive methods.
 * @return
 */
stIpAddress cWinUdpSocket::getIpAddressSource()
{
    return cWinIp::instance().fromSockAddr(&sourceSockAddress);
}

/** @} */
