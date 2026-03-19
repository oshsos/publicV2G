// UTF8 (ü)
/**
\file   cWinUdpSocket.h

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2024-01-05
\brief  See cWinUdpSocket.cpp

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL_Windows
@{

\class LibCpp::cWinUdpSocket
**/

#ifndef CWINUDPSOCKET_H
#define CWINUDPSOCKET_H

#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include "cWinIp.h"     // To be included before other system includes

//#include <ws2tcpip.h>
//#include <netioapi.h>
//#include <iphlpapi.h>
//#include <winsock2.h>
//#include <windows.h>    // must be included after <winsock2.h>!

#include <thread>

#ifdef __MINGW32__
#include "atomic_patch/atomic.h"     // There is an ambigious conversion in <atomic> in line 367. This is corrected in the local library file.
#else
#include <atomic>
#endif

#include "../cUdpSocket.h"
#include "../cDebug.h"

namespace LibCpp
{

/**
 * @brief The windows implementation of the LibCpp::cUdpSocket hardware abstraction class.
 */
class cWinUdpSocket : public cUdpSocket
{
public:
    cWinUdpSocket(bool open = false, int localPort = LibCpp_cIp_UDP_PORT, std::string localInterfaceNameAddress = "", enAddressFamily family = enAddressFamily_IPv4, bool linkLocal = true, bool async = false, iFramePort* pCallback = nullptr);   ///< Constructor.
    ~cWinUdpSocket();                       ///< Destructor.

    enIpResult  open(int localPort = LibCpp_cIp_UDP_PORT, std::string localInterfaceNameAddress = "", enAddressFamily family = enAddressFamily_IPv4, bool linkLocal = true, bool async = false);    ///< Opens the instance.
    void        prepareClose();             ///< Prepares closing by ordering the background task to finish.
    void        close();                    ///< Closes the instance, frees resources and finishes background the task.

    int         send(const char* message, int messageLen, enBlocking blockingMode = enBlocking_BUFFER);                             ///> Sends a data frame.
    int         receive(char* buffer, int bufferSize, enBlocking blockingMode = enBlocking_NONE);                                   ///> Receives a data frame.
    int         receiveBuffer(char** pBuffer, enBlocking blockingMode = enBlocking_NONE);                                           ///> Receives a data frame without copying
    void        receiveAcknowledge();       ///> Acknowledges a reception of a message by 'receiveBuffer'
    void        operate();                  ///> Cyclic operation on callback polling mode usage.

    int         broadcast(const char* message, int messageLen, enBlocking blockingMode = enBlocking_BUFFER);                        ///< Sends a message to all network clients.
    int         sendTo(const char* message, int messageLen, stIpAddress destination, enBlocking blockingMode = enBlocking_BUFFER);  ///< Sends a message to a defined destination.
    int         receiveFrom(char* buffer, int bufferSize, stIpAddress& source, enBlocking blockingMode = enBlocking_NONE);          ///< Receives a message and if successful sets the parameter 'sourceAddress' to the internal memorized source Address.

    enIpResult  setDestination(stIpAddress* pDestination = 0, int port = 0);   ///< Defines the destionation address the 'send' method is sending to. If called parameter less the destionation is set to the last source address.
    enIpResult  setDestination(stIpAddress destination, int port = 0);         ///< Sets the destination address for 'send' calls.
    stIpAddress getIpAddressLocal();        ///< Retrieves the local host ip address the cUdpSocket instance is receiving on.
    stIpAddress getIpAddressDestination();  ///< Sets the destination address, the send method is sending to. Will not be overwritten by sendTo.
    stIpAddress getIpAddressSource();       ///< Retrieves the ip address the last received message has been received from. The address is double buffered and can be retrieved after a 'receive' call.
    void        joinGroup(stIpAddress groupAddress);             ///< Joins the local host to a multicast ip Address.
    int         getLocalPort();                                  ///< Deliveres the port the object is listening on.

private:
    int         sendToInternal(const char* message, int messageLen, unSockAddr destination, enBlocking blockingMode = enBlocking_BUFFER);   ///< Helping method used by 'send' and 'sendTo'.
    void        internalOperate();          ///< Calls registered callback objects on message reception
    void        threadReceiving();          ///< Background thread.

public:
    cDebug dbg;                                     ///< Logger instance

private:

    int                 ifIndex;                    ///< Interface the udp socket is listening on (at IPv6 the same value is and must be stored in localSockAddr.in6.scope_id!
    int                 localSocket;                ///< Local socket the cUdpSocket class instance is using.
    unSockAddr          localSockAddress;           ///< Local host sockaddr the udp socket is listening on.
    unSockAddr          broadcastSockAddress;       ///< Broadcast address.
    unSockAddr          destinationSockAddress;     ///< Address to send messages to.
    unSockAddr          sourceSockAddress;          ///< Is the double buffer of messageSockAddr, set after 'receive' is called.
    unSockAddr          messageSockAddress;         ///< Source Address of the received message.
    char                message[LibCpp_cIp_UDP_BUFFERSIZE]; ///< Message buffer of received messages.
    std::atomic<int>    messageLen;                 ///< Length of the message stored in the message buffer (controls access between background process and main process.
    std::atomic<bool>   threadEnabled;              ///< Flag indicatin a started background process and to be used to friendly close the process.
    std::thread*        pThread;                    ///< Class instance representing the background task.
    bool                async;                      ///< Asynchronous call of callback objects
};

}

#endif

/** @} */
