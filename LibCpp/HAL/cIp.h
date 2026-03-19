// UTF8 (ü) //
/**
\file   cIp.h

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2024-01-02
\brief  See cIp.cpp

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL
@{

\class LibCpp::cIp
**/

#ifndef CIP_H
#define CIP_H

#ifdef PROJECTDEFINES
    #include <ProjectDefines>
#endif

#ifndef LibCpp_cIp_UDP_PORT
    #define LibCpp_cIp_UDP_PORT 55000
#endif
#ifndef LibCpp_cIp_TCP_PORT
#define LibCpp_cIp_TCP_PORT 55100
#endif
#ifndef LibCpp_cIp_RAW_PORT
#define LibCpp_cIp_RAW_PORT 55200
#endif
//#ifndef LibCpp_cLxIP_INTERFACENAMESIZE
//    #define LibCpp_cLxIP_INTERFACENAMESIZE 64
//#endif
#ifndef LibCpp_cIp_TCP_BUFFERSIZE
    #define LibCpp_cIp_TCP_BUFFERSIZE 4096
#endif
#ifndef LibCpp_cIp_UDP_BUFFERSIZE
#define LibCpp_cIp_UDP_BUFFERSIZE 4096
#endif
#ifndef LibCpp_cIp_RAW_BUFFERSIZE
#define LibCpp_cIp_PACKET_BUFFERSIZE 1024
#endif
#ifndef LibCpp_cIp_RAW_BUFFERSIZE
#define LibCpp_cIp_PACKET_BUFFERSIZE 1024
#endif
#if not ( defined (LANGUAGE_EN) || defined (LANGUAGE_DE) )
    #define LANGUAGE_EN // Options are EN, DE
#endif

#define INVALID_INTERFACE 0
#define INVALID_FAMILY    0
#define SOCKET_ERROR      (-1)

#include <string>
#include <vector>
#include <stdint.h>

#include "cDebug.h"

namespace LibCpp
{

/** @brief Results of ip related methods */
enum enIpResult
{
    enIpResult_None,                ///< None
    enIpResult_Success,             ///< Success
    enIpResult_Failure,             ///< Failure
    enIpResult_InvalidIpAddress,    ///< Invalid
    enIpResult_SocketFailed,        ///< Socket failed
    enIpResult_SocketOptionFailed,  ///< Socket option failed
    enIpResult_InvalidInterfaceId,  ///< Invalid interface id
    enIpResult_InterfaceListFailed, ///< Interface list generation failed
    enIpResult_AddrInfoFailed,      ///< Receiving addr info failed
    enIpResult_BindFailed,          ///< Bind failed
    enIpResult_ConnectFailed,       ///< Connection failed
    enIpResult_ListenFailed,        ///< Listen failed
    enIpResult_InvalidFamily,       ///< Invalid ip family
    enIpResult_InvalidInterface,    ///< Not existing interface
    enIpResult_NoInterface          ///< No ethernet interface is present on the system
};

std::string enIpResult_toString(enIpResult result); ///< Converts an ip method return result to a string

/** @brief Enumeration for socket family definition */
enum enAddressFamily
{
    enAddressFamily_None,   ///< Not specified address family (interpreted as IPv4)
    enAddressFamily_IPv4,   ///< IPv4
    enAddressFamily_IPv6    ///< IPv6
};

std::string enAddressFamily_toString(enAddressFamily family);   ///< Converts an enAddressFamily to a string

/** @brief Hardware independent binary ip address description */
struct stIpAddress
{
    enAddressFamily family;     ///< Ip Address family
    unsigned int    address[8]; ///< Ip Address numbers in local machine (host) order. Index 0 represents the first printed number. The last printed number is at index 3 (IPv4) or index 7 (IPv6).
    unsigned int    port;       ///< The port a source address node has been sent from, or the destination port of the node indicated by the address is listening. Zero if undefined.
    unsigned int    ifId;       ///< Interface ID, to be used to reach the address for sending and the local machine interface the source node address has been received from. Zero if unknown.

    stIpAddress();                                  ///< Initializer
    stIpAddress(std::string ipAddressName, int port = 0, int ifId = 0);        ///< Initializer from string
    stIpAddress(const char* ipAddressName);        ///< Initializer from string
    stIpAddress(uint16_t ipAddressBytes[8], int port = 0, int ifId = 0);        ///< Initializer from string
    std::string toString(bool printPort = true, bool printInterface = true);   ///< String conversion (including 'port' or 'ifId' or not)
    bool hasEqualAddress(const stIpAddress& ref) const;   ///< Checks for equality of Address, not taking into account 'ifId')
    bool operator==(const stIpAddress& ref) const;        ///< Equality (if any address family is 'none', 'false' is returned.)
};

/** @brief Interface information data */
struct stInterfaceInfo
{
    unsigned int ifId;                              ///< Interface identifyer
    std::string  name;                              ///< Interface UUID name
    std::string  frientlyName;                      ///< Interface friendly name
    stIpAddress  ipAddress;                         ///< Interface IpAddress (including interface id)
    stIpAddress  ipMulticast;                       ///< Multicast address
    bool         up;                                ///< Is connected and not loop-back

    stInterfaceInfo();
    std::string toString();                         ///< String conversion
};

/** \brief Interface in order to exchange the operating system 'send' and 'recv' methods within implementations of TCP server and clients by external send and receive operations
 *  This interface is used e.g. at hardware independent TLS implementations.
 */
class iExternalSndRcv
{
public:
    virtual int onExternalReceive(char* message, int bufferSize) = 0;       ///< Method for receiving.
    virtual int onExternalSend(const char* message, int messageSize) = 0;   ///< Method for sending.
    virtual bool onPostConnect() {return true;};                            ///< Method being called after a client connect.
};

class cInterfaces;

/** @brief Virtual interface class holding ip support methods. */
class cIp
{
public:
    cIp();                           ///< Constructor
    virtual ~cIp();                  ///< Destructor

    static cIp&          instance(); ///< Access to the hardware dependent global instance providing the virtually static methods

    virtual enIpResult   getInterfaces(cInterfaces* result);                            ///< Receives a list of available interfaces of the local system
    virtual enIpResult   getIpAddress(stIpAddress& result, std::string addressName, int port = 0, int ifId = 0);     ///< Generates a hardware independent ip address from an address string

    std::string          messageString(const char* pMsg, unsigned int messageLen);      ///< Creates a string representation optimized for debugging: length=34 -newline- 0x 01 02
    std::string          messageString(const uint8_t* pMsg, unsigned int messageLen);   ///< Creates a string representation optimized for debugging: length=34 -newline- 0x 01 02

    cDebug dbg;                      ///< Debug output instance
};

/**
 * @brief Creates a list of ethernet interfaces present in the system
 * Actually the list carries one entry per ip address being assigned to any interface.
 * For that reason the same interface id may be multible in the list e.g. one for IPv4 address and one for IPv6 address.
 */
class cInterfaces
{
public:
    cInterfaces(bool open = false);
    ~cInterfaces();
    void            open();
    void            close();

    enIpResult      getInterfaceIpAddress(stIpAddress& result, std::string interfaceOrAddressName = "", enAddressFamily family = enAddressFamily_IPv4, bool linkLocal = true, int port = 0);  ///< Retrieves the hardware independent ethernet address representation from either an address string as "127.0.0.1" (parameter 'family' is ignored) or an interface friendly name string as "eth0".
    enIpResult      getInterfaceIpAddress(stIpAddress& result, unsigned int interfaceId, enAddressFamily family = enAddressFamily_IPv4, int port = 0);             ///< Retrieves the IpAddress of a given Interface Id
    enIpResult      completeInterfaceIpAddress(stIpAddress& address, int port = 0);   ///< Typically adds interface information to a given ip address, but might be used the other way round.
    stInterfaceInfo getInterfaceInfo(std::string interfaceName = "", enAddressFamily family = enAddressFamily_IPv4, bool linkLocal = true);   ///< Retrieves the LibCpp::stInterfaceInfo of a given name
    stInterfaceInfo getInterfaceInfo(unsigned int interfaceId, enAddressFamily family = enAddressFamily_IPv4, bool linkLocal = true);         ///< Retrieves the LibCpp::stInterfaceInfo of a given interface id
    std::string     toString();                                         ///< Converts the interface list to a string

private:
    bool        checkList(cDebug& dbgInst);                             ///< Checks a valid status of the member 'list'.

public:
    cDebug dbg;

    std::vector<stInterfaceInfo> list;                              ///< List of ethernet adapter interfaces
    enIpResult  getInterfacesResult;                                ///< Indicates whether a list of interfaces could be received
};

}
#endif

/** @} */
