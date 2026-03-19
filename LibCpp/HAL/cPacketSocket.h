// UTF8 (ü) //
/**
\file   cPacketSocket.h

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2024-04-22
\brief  See cPacketSocket.cpp

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL
@{

\class LibCpp::cPacketSocket
**/

#ifndef cPacketSocket_H
#define cPacketSocket_H

#ifndef ETHER_ADDR_LEN
#define	ETHER_ADDR_LEN 6
#endif

#include "cFramePort.h"
#include "cIp.h"

namespace LibCpp
{

extern uint8_t MAC_BROADCAST[ETHER_ADDR_LEN];
extern uint8_t MAC_EMPTY[ETHER_ADDR_LEN];

#pragma pack (push,1)

/** Header for layer 2 ethernet MAC based messages (packages) */
typedef struct stEthernetHeader
{
    uint8_t ODA [ETHER_ADDR_LEN];   ///< Ethernet destination MAC address.
    uint8_t OSA [ETHER_ADDR_LEN];   ///< Ethernet source MAC address.
    uint16_t MTYPE;                 ///< Ethernet message (packet) type.
} stEthernetHeader;

/** Ethernet message consisting of header and payload access */
typedef struct stEthernetMsg
{
    stEthernetHeader header;        ///< Header
    uint8_t          payload;       ///< Payload
} stEthernetMsg;

#pragma pack (pop)

/**
*   @brief Abstract interface class for ethernet ip socket using layer 2 mac-address messages
*/
class cPacketSocket : public cFramePort
{
public:
    cPacketSocket();                                                ///< Constructor.
    virtual ~cPacketSocket();                                       ///< Destructor.

    virtual enIpResult  open(std::string localInterfaceNameAddress = "", bool async = false); ///< Sets the Udp socket into operation
    virtual void        prepareClose();                             ///< Prepares for closing by friendly closing the background task.
    virtual void        close();                                    ///< Frees the resources and finishes the background task

    virtual unsigned char*      getMacAddressLocal();               ///< Retrieves the local host mac address the cPacketSocket instance is sending and receiving on.
    virtual void                setFilter();                        ///< Sets the socket messaage filter (e.g. receives just messages to own ethernet (mac) address)

    static std::string  mac_toString(char* macAddress);             ///< Converts a MAC address to a readable string
    };

}
#endif

/** @} */
