// utf-8 (ü)
/**
\file cLxUdpSocket.h

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2023-01-05

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL_Linux
@{

\class LibCpp::cLxUdpSocket
\brief Linux implementation of the \ref G_LibCpp_HAL hardware abstraction layer interface LibCpp::cUdpSocket class

This class is also tested under Mac OS.

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

ens33: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 192.168.178.99  netmask 255.255.255.0  broadcast 192.168.178.255
        inet6 fe80::9659:5a0b:b513:2a9b  prefixlen 64  scopeid 0x20<link>
        inet6 2003:c1:9f1b:7200:5acf:2c9a:3012:5b2c  prefixlen 64  scopeid 0x0<global>
        inet6 2003:c1:9f1b:7200:8bd9:9429:2081:1b2a  prefixlen 64  scopeid 0x0<global>
        ether 00:0c:29:d7:38:e7  txqueuelen 1000  (Ethernet)
        RX packets 189736  bytes 236406865 (236.4 MB)
        RX errors 0  dropped 23895  overruns 0  frame 0
        TX packets 22331  bytes 2449768 (2.4 MB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

lo: flags=73<UP,LOOPBACK,RUNNING>  mtu 65536
        inet 127.0.0.1  netmask 255.0.0.0
        inet6 ::1  prefixlen 128  scopeid 0x10<host>
        loop  txqueuelen 1000  (Lokale Schleife)
        RX packets 448  bytes 50198 (50.1 KB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 448  bytes 50198 (50.1 KB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
**/

#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include <linux/if_link.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/ioctl.h>
//#include <linux/filter.h>
#include <net/if.h>
#include <errno.h>
#include <netinet/in.h>
//#include <linux/if_ether.h>
//#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <thread>
#include <cstring>
#include <string.h>
#include <chrono>

#include "../cDebug.h"
#include "../Tools.h"
#include "cLxUdpSocket.h"

using namespace LibCpp;
//using namespace std;

/**
 * @brief Constructor
 */
cLxUdpSocket::cLxUdpSocket(bool open, int localPort, std::string localInterfaceNameAddress, enAddressFamily family, bool linkLocal, bool async, iFramePort* pCallback) :
    dbg("cLxUdpSocket")
{
    pThread = nullptr;
    localSocket = INVALID_SOCKET;
    cLxUdpSocket::close();
    if (pCallback)
        setCallback(pCallback);
    if (open)
        cLxUdpSocket::open(localPort, localInterfaceNameAddress, family, linkLocal, async);
}

/**
 * @brief Destructor
 */
cLxUdpSocket::~cLxUdpSocket()
{
    cLxUdpSocket::close();
}

/**
 * @brief Prepares for closing by friendly closing the background task.
 * This way several objects requiring to finish a background task can do so in parallel.
 * Just calling 'close' requires much time as each 'close' needs to wait for the backgrond task actually
 * having finished.
 */
void cLxUdpSocket::prepareClose()
{
    threadEnabled = false;
}

/**
 * @brief Closes or releases hardware ressources.
 * See LibCpp::cUdpSocket::close.
 */
void cLxUdpSocket::close()
{
    threadEnabled = false;
    if (pThread)
    {
        pThread->join();
        pThread = nullptr;
    }
    if (localSocket > INVALID_SOCKET)
    {
        shutdown(localSocket, SHUT_RDWR);
        ::close(localSocket);
        localSocket = INVALID_SOCKET;
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
enIpResult cLxUdpSocket::open(int localPort, std::string localInterfaceOrAddressName, enAddressFamily family, bool linkLocal, bool async)
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
    localSockAddress = cLxIp::instance().toSockAddr(&localIpAddr, &ifIndex);

    int af_family = localSockAddress.addr.sa_family;

    // Create socket
    localSocket = socket(af_family , SOCK_DGRAM , 0);
    if (localSocket == -1)
    {
        dbg.printf(enDebugLevel_Fatal, "%s: %i %s!", enIpResult_toString(enIpResult_SocketFailed).c_str(), errno, strerror(errno));
        cLxUdpSocket::close();
        return enIpResult_SocketFailed;
    }

    // Setting socket options
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 500 * 1000;
    int broadcast = 1;
    int reuse = 1;

    if (setsockopt(localSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof (struct timeval)) ||
        setsockopt(localSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof (struct timeval))  ||
        setsockopt(localSocket, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(int)) ||
        setsockopt(localSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) )
    {
        dbg.printf(enDebugLevel_Fatal, "%s", enIpResult_toString(enIpResult_SocketOptionFailed).c_str());
        cLxUdpSocket::close();
        return enIpResult_SocketOptionFailed;
    };

    if (af_family == AF_INET)
    {
        int loopch = 1;
        if (setsockopt(localSocket, IPPROTO_IP, IP_MULTICAST_LOOP, &loopch, sizeof(loopch)))                        // This option has different default values under Linux and Windows, thus explicit setting is necessary.
        {
            dbg.printf(enDebugLevel_Fatal, "IPv4 Proto options: %s", enIpResult_toString(enIpResult_SocketOptionFailed).c_str());
            cLxUdpSocket::close();
            return enIpResult_SocketOptionFailed;
        };
    }
    else if (af_family == AF_INET6)
    {
        // Scope id (interface id) must be set for IPv6
        // in6addr_any is required for Linux (IN6ADDR_ANY_INIT does not work for broadcast over the network), but must not by set at Windows.
        // localSockAddress.sin6_addr = in6addr_loopback;
        localSockAddress.in6.sin6_addr = in6addr_any;
        localSockAddress.in6.sin6_scope_id = ifIndex;

        // Join multi-cast group. This is required only, to receive multi cast messages sent from the local host.
        struct ipv6_mreq mreq;
        inet_pton(AF_INET6, "ff02::1", &mreq.ipv6mr_multiaddr);
        mreq.ipv6mr_interface = ifIndex;
        // Set local interface for outbound multicast datagrams.
        // Set loop on. This way the behavior of ipv6 multicast (used for broadcast) is identical to ipv4 broadcast.
        // If this is not wanted, the user must set this option from outside this class.
        int loopch = 1;
        int on = 0;
        if (setsockopt(localSocket, IPPROTO_IPV6, IPV6_MULTICAST_IF, &ifIndex, sizeof(ifIndex)) ||
            setsockopt(localSocket, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &loopch, sizeof(loopch)) ||
            setsockopt(localSocket, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on)) )                     // This option has different default values under Linux and Windows, thus explicit setting is necessary.
        {
            dbg.printf(enDebugLevel_Fatal, "IPv6 options: %s", enIpResult_toString(enIpResult_SocketOptionFailed).c_str());
            cLxUdpSocket::close();
            return enIpResult_SocketOptionFailed;
        };
    };

    // bind (for receiving operations)
    if (bind(localSocket, (struct sockaddr*)&localSockAddress, sizeof(struct sockaddr_in6)) < 0)
    {
        dbg.printf(enDebugLevel_Error, "%s: %i %s!", enIpResult_toString(enIpResult_BindFailed).c_str(), errno, strerror(errno));
        cLxUdpSocket::close();
        return enIpResult_BindFailed;
    }

    if (af_family == AF_INET)
    {
        // Prepare broadcast socket address for sending operations
        struct sockaddr_in* pBroadAddr = (struct sockaddr_in*)&broadcastSockAddress;
        pBroadAddr->sin_family = AF_INET;
        inet_pton(AF_INET, "255.255.255.255", &pBroadAddr->sin_addr);
        pBroadAddr->sin_port = htons(localPort);

        // Initializing destination address
        stIpAddress destAddr;
        if (!ifIndex && localInterfaceOrAddressName.empty())
        {
            stInterfaceInfo ifcInfo = interfaces.getInterfaceInfo("", localIpAddr.family, linkLocal);
            destAddr = ifcInfo.ipMulticast;
            destAddr.port = localPort;
            if (destAddr.family != enAddressFamily_None)
                destinationSockAddress = cLxIp::instance().toSockAddr(&destAddr);
        }
        else if (ifIndex)
        {
            stInterfaceInfo ifc = interfaces.getInterfaceInfo(ifIndex, enAddressFamily_IPv4);
            destAddr = ifc.ipMulticast;
            destAddr.port = localPort;
            destinationSockAddress = cLxIp::instance().toSockAddr(&destAddr);
        }
    }
    else if (af_family == AF_INET6)
    {
        // Prepare broadcast socket address for sending operations
        broadcastSockAddress.in6.sin6_family = AF_INET6;
        inet_pton(AF_INET6, "ff02::1", &broadcastSockAddress.in6.sin6_addr);
        broadcastSockAddress.in6.sin6_port = htons(localPort);
        broadcastSockAddress.in6.sin6_scope_id = ifIndex;

        // setting multicast (broadcast) group membership
        struct ipv6_mreq group;
        inet_pton(AF_INET6, "ff02::1", &group.ipv6mr_multiaddr.s6_addr);
        group.ipv6mr_interface = ifIndex;
        if(setsockopt(localSocket, IPPROTO_IPV6, IPV6_JOIN_GROUP, &group, sizeof group))
        {
            dbg.printf(enDebugLevel_Error, "%s Setting group membership failed!", enIpResult_toString(enIpResult_SocketOptionFailed).c_str());
            cLxUdpSocket::close();
            return enIpResult_SocketOptionFailed;
        };

        // Initializing destination address
        destinationSockAddress = broadcastSockAddress;
    }

    stIpAddress addr = cLxUdpSocket::getIpAddressLocal();
    stInterfaceInfo ifc = interfaces.getInterfaceInfo(ifIndex);
    dbg.printf(enDebugLevel_Info, "Listening at interface '%s' local address %s", ifc.frientlyName.c_str(), addr.toString().c_str());

    this->async = async;

    threadEnabled = true;     // To be fast enough in case a direct 'receive' follows after 'open'
    pThread = new std::thread(&cLxUdpSocket::threadReceiving, this);
    return enIpResult_Success;
}

/**
 * @brief cWinUdpSocket::setDestination
 * @param pDestination
 * @param port If port is set, it overrules and sets the port defined within the stIpAddress of 'destination'.
 * @return Error code: enIpResult_Success, enIpResult_InvalidIpAddress
 */
enIpResult cLxUdpSocket::setDestination(stIpAddress* pDestination, int port)
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
    dbg.printf(enDebugLevel_Info, "Sending to address %s", pDestination->toString().c_str());
    destinationSockAddress = cLxIp::instance().toSockAddr(pDestination);
    return enIpResult_Success;
}

/**
 * @brief cWinUdpSocket::setDestination
 * @param pDestination
 * @param port If port is set, it overrules the port defined within the stIpAddress of 'destination'.
 * @return Error code: enIpResult_Success, enIpResult_InvalidIpAddress
 */
enIpResult cLxUdpSocket::setDestination(stIpAddress destination, int port)
{
    return setDestination(&destination, port);
}

/**
 * @brief Sets a group address the host is listening on.
 * @param groupAddress
 */
void cLxUdpSocket::joinGroup(stIpAddress groupAddress)
{
    cDebug dbg("joinGroup", &this->dbg);
    if (!localSocket)
    {
        dbg.printf(enDebugLevel_Error, "Method must be called, after object is opened!");
        return;
    }
    unSockAddr groupSockAddr = cLxIp::instance().toSockAddr(&groupAddress);
    if (localSockAddress.addr.sa_family !=  groupSockAddr.addr.sa_family)
    {
        dbg.printf(enDebugLevel_Error, "Group address familiy must be equal to local address family!");
        return;
    }
    if (groupAddress.family == enAddressFamily_IPv6)
    {
        struct ipv6_mreq group;
        for (int i=0; i<16; i++)
            group.ipv6mr_multiaddr.s6_addr[i] = groupSockAddr.in6.sin6_addr.s6_addr[i];
        group.ipv6mr_interface = ifIndex;

        if(setsockopt(localSocket, IPPROTO_IPV6, IPV6_JOIN_GROUP, &group, sizeof(struct ipv6_mreq))) // IPV6_JOIN_GROUP, IPV6_ADD_MEMBERSHIP
        {
            dbg.printf(enDebugLevel_Fatal, "%s Setting group membership failed!", enIpResult_toString(enIpResult_SocketOptionFailed).c_str());
            close();
            return;
        }
    }
    else if (groupAddress.family == enAddressFamily_IPv4)
    {
        struct ip_mreq group;
        group.imr_multiaddr.s_addr = groupSockAddr.in.sin_addr.s_addr;
        group.imr_interface.s_addr = localSockAddress.in.sin_addr.s_addr;
        //group.imr_multiaddr.s_addr = inet_addr("224.0.0.11");
        //group.imr_interface.s_addr = inet_addr("192.168.178.59");

        if(setsockopt(localSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &group, sizeof(struct ip_mreq))) // IP_JOIN_GROUP
        {
            dbg.printf(enDebugLevel_Fatal, "%s Setting group membership failed!", enIpResult_toString(enIpResult_SocketOptionFailed).c_str());
            close();
            return;
        }
    }
}

/**
 * @brief Deliveres the port the object is listening on.
 * @return
 */
int cLxUdpSocket::getLocalPort()
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
int cLxUdpSocket::sendToInternal(const char* message, int messageLen, unSockAddr destination, enBlocking blockingMode)
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
* @return Number of actually sent bytes, -1 on failure.
*/
int cLxUdpSocket::send(const char* message, int messageLen, enBlocking blockingMode)
{
   return sendToInternal(message, messageLen, destinationSockAddress, blockingMode);
}

/**
* @brief Sends a message to a stIpAddress type destination.
* @param message
* @param messageLen
* @param destination
* @param blockingMode
* @return Number of actually sent bytes, -1 on failure.
*/
int cLxUdpSocket::sendTo(const char* message, int messageLen, stIpAddress destination, enBlocking blockingMode)
{
   return sendToInternal(message, messageLen, cLxIp::instance().toSockAddr(&destination), blockingMode);
}

/**
 * @brief Receives a message
 * You may call LibCpp::cLxUdpSocket::getIpAddressSource after this operation.
 * @param buffer
 * @param bufferSize
 * @param blockingMode Not implemented, method is not blocking
 * @return Number of bytes received.
 */
int cLxUdpSocket::receive(char* buffer, int bufferSize, enBlocking blockingMode)
{
    cDebug dbg("receive", &this->dbg);
    int len = messageLen;
    if (len>0)
    {
        if (len>bufferSize) len=bufferSize;
        // dbg.printf(enDebugLevel_Debug, "Fetched message len=%i: %s", len, ByteArrayToString(message, len).c_str());
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
int cLxUdpSocket::receiveFrom(char* buffer, int bufferSize, stIpAddress& source, enBlocking blockingMode)
{
    char* data;
    int len = receiveBuffer(&data);
    if (len<=0)
        return len;
    source = cLxIp::instance().fromSockAddr(&sourceSockAddress);
    return receive(buffer, bufferSize, blockingMode);
}

/**
 * @brief Receives a message without copying from the receive buffer
 * In case the message is processed, call LibCpp::cLxUdpSocket::receiveAcknowledge to free the receive buffer.
 * @param ppBuffer
 * @param blockingMode blockingMode Not implemented, method is not blocking
 * @return Number of bytes available in the receive buffer.
 */
int cLxUdpSocket::receiveBuffer(char** ppBuffer, enBlocking blockingMode)
{
    *ppBuffer = message;
    int len = messageLen;
    if (len)
    {
        // dbg.printf(enDebugLevel_Debug, "Fetched message len=%i: %s", len, ByteArrayToString(message, len).c_str());
        sourceSockAddress = messageSockAddress;
    }
    return len;
}

/**
 * @brief Frees the receive Buffer
 * Method is to be called after calling LibCpp::cLxUdpSocket::receiveBuffer to enable the reception of further messages.
 */
void cLxUdpSocket::receiveAcknowledge()
{
    if (messageLen>0 && callbacks.size())              // do not access for writing, if the receiving task has control of the buffer
        messageLen = 0;
}

/**
 * @brief Sends a message to all hosts within the network
 * @param message
 * @param messageLen
 * @param blockingMode blockingMode Not implemented, method is not blocking
 * @return Number of actually sent bytes, -1 on failure.
 */
int cLxUdpSocket::broadcast(const char* message, int messageLen, enBlocking blockingMode)
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
void cLxUdpSocket::operate()
{
    if (!localSocket || async)
        return;
    internalOperate();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

/**
 * @brief Method to call registered callback objects
 * The method is called either by 'operate' or the background thread
 * depending of 'async' is unset or set.
 */
void cLxUdpSocket::internalOperate()
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
void cLxUdpSocket::threadReceiving()
{
    cDebug dbg("threadReceiving", &this->dbg);

    threadEnabled = true;
    int read_size = 1;

    while(threadEnabled)
    {
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
            // Receive a message from server
            socklen_t addrLen = sizeof (unSockAddr);
            read_size = recvfrom(localSocket , message , LibCpp_cIp_UDP_BUFFERSIZE , 0, &messageSockAddress.addr, &addrLen);
            if (read_size>0)
            {
                // Ensure zero termination of message
                if (read_size < LibCpp_cIp_UDP_BUFFERSIZE)
                    message[read_size] = 0;
                messageLen = read_size;
            }
            else if ((read_size == -1) && !(errno == 0 || errno == EAGAIN  || errno == EWOULDBLOCK|| errno == EINTR))  // EINTR = 4 wird beim Debugging benötigt
            {
                dbg.printf(enDebugLevel_Error, "Receiving failed with %i '%s'! Closing.", errno, strerror(errno));
                threadEnabled = false;
            }
            if (messageLen > 0)
            {
                if (async)
                {
                    internalOperate();
                }
                while (messageLen > 0 && threadEnabled)  // wait for message buffer to get read
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
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
stIpAddress cLxUdpSocket::getIpAddressLocal()
{
    return cLxIp::instance().fromSockAddr(&localSockAddress, ifIndex);
}

/**
 * @brief Retrieves the standard destination address.
 * @return
 */
stIpAddress cLxUdpSocket::getIpAddressDestination()
{
    return cLxIp::instance().fromSockAddr(&destinationSockAddress);
}

/**
 * @brief Retrieves the source address of the last received message.
 * The method is to be called after a call to any receive methods.
 * @return
 */
stIpAddress cLxUdpSocket::getIpAddressSource()
{
    return cLxIp::instance().fromSockAddr(&sourceSockAddress);
}

/** @} */
