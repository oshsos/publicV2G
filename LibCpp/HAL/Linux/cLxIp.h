
// UTF8 (ü) //
/**
\file   cLxIp.h

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2023-11-16
\brief  See cLxIp.cpp

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL_Linux
@{

\class LibCpp::cLxIp
**/

#ifndef CLXIP_H
#define CLXIP_H

#define INVALID_SOCKET -1

#include <sys/socket.h>
#include <netinet/in.h>

#include "../cIp.h"

namespace LibCpp
{
void cIpCheckInstance();

/** @brief Union definition of the different struct sockaddr types */
union unSockAddr
{
    struct sockaddr     addr;       ///< General access to address family
    struct sockaddr_in  in;         ///< Access to IPv4 addresses
    struct sockaddr_in6 in6;        ///< Access to IPv6 addresses
};

/** @brief Implementation of general ethernet support methods for Windows */
class cLxIp : public cIp
{
public:
    cLxIp();                           ///< Constructor
    ~cLxIp();                          ///< Destructor

    static cLxIp&        instance();    ///< Access to hardware dependent "static" methods

    virtual enIpResult   getInterfaces(cInterfaces* result);                    ///< Receives a list of available interfaces of the local system
    virtual enIpResult   getIpAddress(stIpAddress& address, std::string addressName, int port = 0, int ifId = 0);     ///< Generates a hardware independent ip address from an address string

    stIpAddress          fromSockAddr(unSockAddr* pSockAddr, int ifId = 0);     ///< Converts system sockaddr structs to the hardware independent address representation
    unSockAddr           toSockAddr(stIpAddress* pIpAddress, int* pIfId = 0);    ///< Converts the hardware independent ethernet address to the system like representation.

    cDebug dbg;
};

}

#endif

/** @} */
