// UTF8 (ü) //
/**
\file   cLxUdpSocket.h

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2024-01-05
\brief  See cLxUdpSocket.cpp

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL
@{

\class LibCpp::cLxUdpSocket
The class sets the socket options SO_BROADCAST and SO_REUSEADDR
to allow broadcast messages and several instances working on the
same machine at the same address and port.\n
The program 'SocketTest' does not set SO_REUSEADDR and behaves different,
not only for parallel program operation but also concerning message mirroring.\n
\n
To send and receive UDP unicast or broadcast messages the behavior or Linux
and Windows differs. For Linux, the reception of unicast or broadcast
messages requires the following rules:\n
\n
To receive unicast messages you can either bind to:\n
- all interfaces (Address: 0.0.0.0)
- the unicast address of the desired interface
Binding to the interface name (id) does not work\n
\n
To receive broadcast messages:\n
- it is required to bind to the broadcast
\n
Sending unicast or broadcast is less strict concerning the binding rules.\n
\n
Sent messages are received by other program instances (sockets) bound to the same interface and port.\n
Sent unicast messages are mirrored back to the sender.\n
Sent broadcast messages are also mirrored to the sender.\n
\n

**/


#ifndef CLXUDPSOCKET_H
#define CLXUDPSOCKET_H

#ifdef PROJECTDEFINES
    #include <ProjectDefines.h>
#endif

#include <string>
#include <cstdint>
#include <sys/socket.h>
#include <netinet/in.h>
#include <atomic>
#include <thread>

#include "../cDebug.h"
#include "../cUdpSocket.h"
#include "cLxIp.h"

namespace LibCpp
{

/**
 * @brief The linux implementation of the LibCpp::cUdpSocket hardware abstraction class.
 */
class cLxUdpSocket : public cUdpSocket
{
public:
    cLxUdpSocket(bool open = false, int localPort = LibCpp_cIp_UDP_PORT, std::string localInterfaceNameAddress = std::string(""), enAddressFamily family = enAddressFamily_IPv4, bool linkLocal = true, bool async = false, iFramePort* pCallback = nullptr);   ///< Constructor.
    ~cLxUdpSocket();                        ///< Destructor.

    enIpResult  open(int localPort = LibCpp_cIp_UDP_PORT, std::string localInterfaceNameAddress = std::string(""), enAddressFamily family = enAddressFamily_IPv4, bool linkLocal = true, bool async = false);
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
    cDebug      dbg;

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
