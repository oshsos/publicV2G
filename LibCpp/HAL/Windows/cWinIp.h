// UTF8 (ü) //
/**
\file   cWinIp.h

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2023-11-16
\brief  See cWinIp.cpp

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL_Windows
@{

\class LibCpp::cWinIp
**/

#ifndef CWINIP_H
#define CWINIP_H

#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#define cWinIp_INTERFACEBUFFERSIZE 64000

#include <winsock2.h>   // To be included before <windows.h>
#include <ws2ipdef.h>   // To be included before <iphlpapi.h> (at Visual Studio compiler)
#include <iphlpapi.h>   
//#include <netioapi.h>  // Should be included by <iphlpapi.h>
#include <ws2tcpip.h>
#include <windows.h>  // Must be included after <winsock2.h>, which is used by above includes of #define _WINSOCKAPI_ before.

#include "../cIp.h"

namespace LibCpp
{
void cIpCheckInstance();

// global functions matching linux system calls
int inet_pton(int af, const char* src, void* dst);                              ///< Converts a binary ip address to a string representation.
int setsockopt(int socket, int level, int optname, void* optval, int optlen);   ///< Sets socket options using void* instead of char*.

/** @brief Union definition of the different struct sockaddr types */
union unSockAddr
{
    struct sockaddr     addr;       ///< General access to address family
    struct sockaddr_in  in;         ///< Access to IPv4 addresses
    struct sockaddr_in6 in6;        ///< Access to IPv6 addresses
};

/** @brief Implementation of general ethernet support methods for Windows */
class cWinIp : public cIp
{
public:
    cWinIp();                           ///< Constructor
    ~cWinIp();                          ///< Destructor

    static cWinIp&       instance();    ///< Access to hardware dependent "static" methods
    //static cWinIp*       pMethods();    ///< Access to hardware dependent "static" methods

    virtual enIpResult   getInterfaces(cInterfaces* result);                    ///< Receives a list of available interfaces of the local system
    virtual enIpResult   getIpAddress(stIpAddress& address, std::string addressName, int port = 0, int ifId = 0);     ///< Generates a hardware independent ip address from an address string

    stIpAddress          fromSockAddr(unSockAddr* pSockAddr, int ifId = 0);     ///< Converts system sockaddr structs to the hardware independent address representation
    unSockAddr           toSockAddr(stIpAddress* pIpAddress, int* pIfId = 0);    ///< Converts the hardware independent ethernet address to the system like representation.

    //    int                  getInterfaceId(struct sockaddr* address);
    //    static enIpResult    getInterfaceSockAddr(struct sockaddr* address, int ifId);
    //    static enIpResult    getSockAddr(struct sockaddr_in6* sockAddr, const char* address, int port = 0);  // Argument should be struct sockaddr, but sockaddr_in6 is required to have enough space allocated.
    //    static enIpResult    getSockAddr(struct sockaddr_in6* sockAddr, int* ifIndex, int port, const char* interfaceNameAddress, enAddressFamily addressFamily = enAddressFamily_IPv4);  // ifIndex only used for IPv4, family only used, if interface is specified with 'interfaceNameAddress' Argument should be struct sockaddr, but sockaddr_in6 is required to have enough space allocated.
    //    static std::string   getAddress(struct sockaddr_in6* sockAddr);                                      // You can cast from sockAddr_in also! Usable for IPv4 and IPv6

    // getAddressString
    //    static const char*   enIpResultToString(enIpResult result);
    //    static const char*   enAddressFamilyToString(enAddressFamily family);
    cDebug dbg;
};

}
#endif
/** @} */
