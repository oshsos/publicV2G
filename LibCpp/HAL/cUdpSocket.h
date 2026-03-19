// UTF8 (ü) //
/**
\file   cUdpSocket.h

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2024-01-01
\brief  See cUdpSocket.cpp

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL
@{

\class LibCpp::cUdpSocket
**/

#ifndef cUdpSocket_H
#define cUdpSocket_H

#include "cFramePort.h"
#include "cIp.h"

namespace LibCpp
{

/**
*   @brief Abstract interface class for ethernet ip socket with UDP protocol
*/
class cUdpSocket : public cFramePort
{
public:
    cUdpSocket();                                                   ///< Constructor.
    virtual ~cUdpSocket();                                          ///< Destructor.

    virtual enIpResult  open(int localPort = LibCpp_cIp_UDP_PORT, std::string localInterfaceNameAddress = "", enAddressFamily family = enAddressFamily_IPv4, bool linkLocal = true, bool async = false); ///< Sets the Udp socket into operation
    virtual void        prepareClose();                             ///< Prepares for closing by friendly closing the background task.
    virtual void        close();                                    ///< Frees the resources and finishes the background task

    virtual int         broadcast(const char* pMsg, int messageLen, enBlocking blockingMode = enBlocking_BUFFER);                               ///< Sends a message to all network clients.
    virtual int         sendTo(const char* message, int messageLen, stIpAddress destination, enBlocking blockingMode = enBlocking_BUFFER);      ///< Sends a message to a defined destination.
    virtual int         receiveFrom(char* buffer, int bufferSize, stIpAddress& source, enBlocking blockingMode = enBlocking_NONE);              ///< Receives a message and if successful sets the parameter 'sourceAddress' to the internal memorized source Address.

    virtual enIpResult  setDestination(stIpAddress* pDestination = 0, int port = 0);    ///< Sets the destination address for 'send' calls.
    virtual enIpResult  setDestination(stIpAddress destination, int port = 0);          ///< Sets the destination address for 'send' calls.
    virtual stIpAddress getIpAddressDestination();                  ///< Reads the configured destionation ip address.
    virtual stIpAddress getIpAddressLocal();                        ///< Deliveres the interface ip address the UDP socket is bound to.
    virtual stIpAddress getIpAddressSource();                       ///< Deliveres the ip Address of the sender of the last received message
    virtual int         getLocalPort();                             ///< Deliveres the port the object is listening on.
    virtual void        joinGroup(stIpAddress groupAddress);        ///< Joins the local host to a multicast ip Address.
    int                 broadcast(const uint8_t* pMsg, int messageLen, enBlocking blockingMode = enBlocking_BUFFER);                            ///< Like 'broadcast'
    int                 sendTo(const uint8_t* message, int messageLen, stIpAddress destination, enBlocking blockingMode = enBlocking_BUFFER);   ///< Identical to 'sendTo', but using uint8_t as buffer.
    int                 receiveFrom(uint8_t* buffer, int bufferSize, stIpAddress& source, enBlocking blockingMode = enBlocking_NONE);           ///< Receives a message and if successful sets the parameter 'sourceAddress' to the internal memorized source Address.
};

}
#endif

/** @} */
