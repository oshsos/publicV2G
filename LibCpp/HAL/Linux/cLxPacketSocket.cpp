/**
\file cLxPacketSocket.cpp

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2023-05-16

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL_Linux
@{

\class LibCpp::cLxPacketSocket
\brief Linux implementation of the \ref G_LibCpp_HAL hardware abstraction layer interface LibCpp::cPacketSocket class


//C:\Users\Olaf Simon>ipconfig

cLxPacketSocket port;
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

#include "../LibCpp/HAL/Windows/cLxPacketSocket.h"
#include "../LibCpp/HAL/Tools.h"

using namespace LibCpp;


cLxPacketSocket port;


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
#include <limits.h>
#include <sys/ioctl.h>
#include <linux/filter.h>
#include <net/if.h>
#include <errno.h>
#include <netinet/in.h>
#include <net/ethernet.h>
#include <thread>
#include <cstring>
#include <string.h>
#include <chrono>
#include <net/if_arp.h>
#include <netpacket/packet.h>
#include <sys/ioctl.h>
#include <poll.h>

#include "cLxPacketSocket.h"

#include "cLxIp.h"
#include "../cDebug.h"

using namespace LibCpp;
using namespace std;

/**
 * @brief Constructor
 */
cLxPacketSocket::cLxPacketSocket(bool open, string localInterfaceNameAddress, bool async, iFramePort* pCallback) :
    dbg("cLxPacketSocket")
{
    pThread = nullptr;
    localSocket = INVALID_SOCKET;
    cLxPacketSocket::close();
    if (pCallback)
        setCallback(pCallback);
    if (open)
        cLxPacketSocket::open(localInterfaceNameAddress, async);
}

/**
 * @brief Destructor
 */
cLxPacketSocket::~cLxPacketSocket()
{
    cLxPacketSocket::close();
}

/**
 * @brief Prepares for closing by friendly closing the background task.
 * This way several objects requiring to finish a background task can do so in parallel.
 * Just calling 'close' requires much time as each 'close' needs to wait for the backgrond task actually
 * having finished.
 */
void cLxPacketSocket::prepareClose()
{
    threadEnabled = false;
}

/**
 * @brief Closes or releases hardware ressources.
 * See LibCpp::cPacketSocket::close.
 */
void cLxPacketSocket::close()
{
    threadEnabled = false;
    if (pThread)
    {
        pThread->join();
        delete pThread;
        pThread = nullptr;
    }
    if (localSocket != INVALID_SOCKET)
    {
        shutdown(localSocket, SHUT_RDWR);
        ::close(localSocket);
        localSocket = INVALID_SOCKET;
    }
    memset(&localMacAddress, 0, ETHER_ADDR_LEN);
    messageLen = 0;
    async = false;
}

/**
 * @brief Opens or aquires hardware ressources.
 * See LibCpp::cPacketSocket::open.
 * @param localPort
 * @param localInterfaceOrAddressName
 * @param family
 * @param linkLocal If set and IPv6, only link local addresses (fe80:...) are accepted. Otherwise any address is taken.
 * @return Error code
 */
enIpResult cLxPacketSocket::open(string localInterfaceOrAddressName, bool async)
{
    cDebug dbg("open", &this->dbg);

//    cInterfaces interfaces(true);

//    if (interfaces.getInterfacesResult != enIpResult_Success)
//    {
//        if (interfaces.getInterfacesResult == enIpResult_NoInterface)
//        {
//            dbg.printf(enDebugLevel_Error, "No ethernet interface available! Check for external hardware to be plugged in.");
//            return interfaces.getInterfacesResult;
//        }
//        dbg.printf(enDebugLevel_Fatal, "Ethernet interfaces could not be retrieved from the host! %s", enIpResult_toString(interfaces.getInterfacesResult).c_str());
//        return interfaces.getInterfacesResult;
//    }

    // Open socket
    if ((localSocket = socket (PF_PACKET, SOCK_RAW, 0)) == INVALID_SOCKET)  // ETH_P_ALL
    {
        dbg.printf(enDebugLevel_Fatal, "Socket opening failed! %s: %i %s!", enIpResult_toString(enIpResult_SocketFailed).c_str(), errno, strerror(errno));
        cLxPacketSocket::close();
        return enIpResult_SocketFailed;
    }

    // Get interface index
    struct ifreq ifreq;
    memset (&ifreq, 0, sizeof (ifreq));
    memcpy (ifreq.ifr_name, localInterfaceOrAddressName.c_str(), sizeof (ifreq.ifr_name));
    if (ioctl (localSocket, SIOCGIFINDEX, &ifreq) == -1)
    {
        dbg.printf(enDebugLevel_Fatal, "Failed to retrieve interface index of '%s'! %s", localInterfaceOrAddressName.c_str(), enIpResult_toString(enIpResult_NoInterface).c_str());
        cLxPacketSocket::close();
        return enIpResult_NoInterface;
    }
    ifIndex = ifreq.ifr_ifindex;

    // Get hardware address (MAC Address)
    if (ioctl (localSocket, SIOCGIFHWADDR, &ifreq) == -1)
    {
        dbg.printf(enDebugLevel_Fatal, "Failed to retrieve local hardware MAC address of %s! %s", localInterfaceOrAddressName.c_str(), enIpResult_toString(enIpResult_SocketOptionFailed).c_str());
        cLxPacketSocket::close();
        return enIpResult_SocketOptionFailed;
    }

    // Setting configuration struct
    struct sockaddr_ll sockaddress_ll =
    {
        PF_PACKET,
        0x0000,
        0x0000,
        ARPHRD_ETHER,
        PACKET_HOST,
        ETHER_ADDR_LEN,
        {
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00
        }
    };

    sockaddress_ll.sll_protocol = htons (ETH_P_ALL);  // Setting MTYPE!  0x88E1
    sockaddress_ll.sll_ifindex = ifIndex;
    memcpy (sockaddress_ll.sll_addr, ifreq.ifr_ifru.ifru_hwaddr.sa_data, sizeof (sockaddress_ll.sll_addr));

    // Setting socket options
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    if (setsockopt(localSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof (struct timeval)) ||
        setsockopt(localSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof (struct timeval)) )
    {
        dbg.printf(enDebugLevel_Error, "%s", enIpResult_toString(enIpResult_SocketOptionFailed).c_str());
    };

    // Bind the socket
    if (bind (localSocket, (struct sockaddr *) (&sockaddress_ll), sizeof (sockaddr_ll)) == -1)
    {
        dbg.printf(enDebugLevel_Fatal, "Failed to bind interface '%s'! %s", localInterfaceOrAddressName.c_str(), enIpResult_toString(enIpResult_BindFailed).c_str());
        cLxPacketSocket::close();
        return enIpResult_BindFailed;
    }

    // Memorize local MAC address
    memcpy (localMacAddress, sockaddress_ll.sll_addr, ETHER_ADDR_LEN);

    // Check the interface being up
    if (ioctl (localSocket, SIOCGIFFLAGS, &ifreq) == -1)
    {
        dbg.printf(enDebugLevel_Error, "Failed to retrieve state of interface '%s'!", localInterfaceOrAddressName.c_str());
    }
    int ifstate = ifreq.ifr_flags;
    if (!(ifstate & IFF_UP))
    {
        dbg.printf(enDebugLevel_Error, "Interface '%s' is not up!", localInterfaceOrAddressName.c_str());
    }

    dbg.printf(enDebugLevel_Info, "Listening as packet socket at '%s' with MAC address %s.", localInterfaceOrAddressName.c_str(), cPacketSocket::mac_toString((char*)getMacAddressLocal()).c_str());

    this->async = async;

    threadEnabled = true;     // To be fast enough in case a direct 'receive' follows after 'open'
    pThread = new std::thread(&cLxPacketSocket::threadReceiving, this);
    return enIpResult_Success;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"

/**
 * @brief Sends a message to a unSockAddr type destination.
 * @param message
 * @param messageLen
 * @param blockingMode
 * @return Len of actually sent bytes, -1 on failure.
 */
int cLxPacketSocket::send(const char* message, int messageLen, enBlocking blockingMode)
{
    if (localSocket != INVALID_SOCKET)
    {
        return ::sendto (localSocket, message, messageLen, 0, (struct sockaddr *) (0), (socklen_t) (0));
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
int cLxPacketSocket::receive(char* buffer, int bufferSize, enBlocking blockingMode)
{
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
 * @return Number of bytes available in the receive buffer.
 */
int cLxPacketSocket::receiveBuffer(char** ppBuffer, enBlocking blockingMode)
{
    *ppBuffer = message;
    return messageLen;
}

#pragma GCC diagnostic warning "-Wunused-parameter"

/**
 * @brief Frees the receive Buffer
 * Method is to be called after calling LibCpp::cLxUdpSocket::receiveBuffer to enable the reception of further messages.
 */
void cLxPacketSocket::receiveAcknowledge()
{
    if (messageLen>0 && !callbacks.size())              // do not access for writing, if the receiving task has control of the buffer
        messageLen = 0;
}

/**
 * @brief Method to be called cycelic to synchronously call registered callbacks
 * This type of operation mode is a polling method.
 */
void cLxPacketSocket::operate()
{
    if (localSocket==INVALID_SOCKET || async)
        return;

//    if (!messageLen)
//    {
//        struct pollfd pollfd =
//        {
//            localSocket,
//            POLLIN,
//            0
//        };
//        signed status = poll (&pollfd, 1, 0);
//        if ((status < 0) && (errno != EINTR))
//        {
//            cDebug dbg("internalReceive", &this->dbg);
//            dbg.printf(enDebugLevel_Error, "Receiving failed with %i '%s'!", errno, strerror(errno));
//            return;
//        }

//        if (status > 0)
//        {
//            int res = recvfrom (localSocket, message, LibCpp_cIp_PACKET_BUFFERSIZE, 0, (struct sockaddr *) (0), (socklen_t *)(0));
//            if (res>0)
//            {
//                int len = res;
//                // Ensure zero termination of message
//                if (len < LibCpp_cIp_PACKET_BUFFERSIZE)
//                    message[len] = 0;
//                messageLen = len;
//            }
//            else if (res<0)
//            {
//                cDebug dbg("internalReceive", &this->dbg);
//                dbg.printf(enDebugLevel_Error, "Receiving failed with %i '%s'! Closing.", errno, strerror(errno));
//            }
//        }
//    }

    internalOperate();
}

/**
 * @brief Method to call registered callback objects
 * The method is called either by 'operate' or the background thread
 * depending of 'async' is unset or set.
 */
void cLxPacketSocket::internalOperate()
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
void cLxPacketSocket::threadReceiving()
{
    threadEnabled = true;

    char buffer[LibCpp_cIp_PACKET_BUFFERSIZE];
    cDebug dbg("threadReceiving", &this->dbg);

    while(threadEnabled)
    {
        // Receive a message

//        struct pollfd pollfd =
//        {
//            localSocket,
//            POLLIN,
//            0
//        };
//        signed status = poll (&pollfd, 1, 0);
//        if ((status < 0) && (errno != EINTR))
//        {
//            cDebug dbg("internalReceive", &this->dbg);
//            dbg.printf(enDebugLevel_Error, "Receiving failed with %i '%s'!", errno, strerror(errno));
//            return;
//        }

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
            int len = recvfrom (localSocket, buffer, LibCpp_cIp_PACKET_BUFFERSIZE, 0, (struct sockaddr *) (0), (socklen_t *)(0));
            if (len>0)
            {
                // Ensure zero termination of message
                if (len < LibCpp_cIp_PACKET_BUFFERSIZE)
                    message[len] = 0;

                // Check message to be a relevant one
                bool relevant = true;
                if (callbacks.size())
                {
                    relevant = false;
                    for (iFramePort* pPort : callbacks)
                        if (pPort->onReceiveFrameCheck(this, buffer, (unsigned int*)&len))
                            relevant = true;
                }
                if (relevant && len)
                {
                    memcpy(message, buffer, len);
                    messageLen = len;

                    if (async)
                    {
                        internalOperate();
                    }

                    while (messageLen > 0 && threadEnabled)  // wait for message buffer to get read
                        this_thread::sleep_for(chrono::milliseconds(1));
                }
            }
            else if (len<0)
            {
                dbg.printf(enDebugLevel_Error, "Receiving failed with %i '%s'! Closing.", errno, strerror(errno));
                threadEnabled = false;
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
unsigned char* cLxPacketSocket::getMacAddressLocal()
{
    return localMacAddress;
}

/**
 * @brief Sets the socket messaage filter (e.g. receives just messages to own ethernet (mac) address)
 */
void cLxPacketSocket::setFilter()
{
}

/** @} */
