// utf8 (ü)
/**
\file cUdpSocket.cpp

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2023-01-02

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL
@{
\class cWinIp
 * To call methods use e.g. 'cWinIp::instance().toSockAddr'.\n
**/

#define _WINSOCK_DEPRECATED_NO_WARNINGS         // required for Visual Studio compiler

#include "cWinIp.h"

#include <stdlib.h>
#include <stdio.h>
#include <string>


#include "../cDebug.h"

using namespace std;
using namespace LibCpp;

class cWinIpDeleter;

extern cIp* pGlobalIpInstance;  ///< cIp type pointer to the unique instance winIp, to be called by cIp::instance(). ... !
cWinIp*     pWinIp = nullptr;   ///< Unique instance of cWinIp class. Instance is to be called by cWinIp::instance(). ... !

class cWinIpDeleter
{
public:
    ~cWinIpDeleter()
    {
        if (pWinIp)
        {
            cWinIp* pLocal = pWinIp;
            pWinIp = nullptr;
            delete pLocal;
        }
    }
};

cWinIpDeleter winIpDeleter;

typedef struct _IP_ADAPTER_UNICAST_ADDRESS_LH2
{
  union {
    ULONGLONG Alignment;
    struct {
      ULONG Length;
      DWORD Flags;
    };
  };
  struct _IP_ADAPTER_UNICAST_ADDRESS_LH *Next;
  SOCKET_ADDRESS                        Address;
  IP_PREFIX_ORIGIN                      PrefixOrigin;
  IP_SUFFIX_ORIGIN                      SuffixOrigin;
  IP_DAD_STATE                          DadState;
  ULONG                                 ValidLifetime;
  ULONG                                 PreferredLifetime;
  ULONG                                 LeaseLifetime;
  UINT8                                 OnLinkPrefixLength;
} IP_ADAPTER_UNICAST_ADDRESS_LH2, *PIP_ADAPTER_UNICAST_ADDRESS_LH2;

void LibCpp::cIpCheckInstance()
{
    if (!pWinIp)
    {
        pWinIp = (cWinIp*)1;
        pWinIp = new(cWinIp);
        pGlobalIpInstance = pWinIp;
    }
}

/**
 * @brief Converts an ethernet ip address string to a binary representation (take care of result type described below)
 * @param dst The type of this parameter is not(!) struct sockaddr_in, it is of type struct in_addr or struct in6_addr, both just a part of struct sockaddr_in!
 * @returns 1=Success, 0=String does not represent a valid IP address
**/
int LibCpp::inet_pton(int af, const char* src, void* dst)
{
    // -> inetptonW()
    if (!dst) return 0;

    int size;
    char addrStr[64];
    strcpy_s(addrStr, src);

    if (af == AF_INET)
    {
        struct sockaddr_in sockAddr;
        memset(&sockAddr, 0, sizeof(struct sockaddr_in));
        size = sizeof(struct sockaddr_in);
        if (0 == WSAStringToAddressA(addrStr, AF_INET, NULL, (struct sockaddr*)&sockAddr, &size))
        {
            memcpy(dst, &sockAddr.sin_addr, sizeof(struct in_addr));
            return 1;
        }
    }
    else if (af == AF_INET6)
    {
        struct sockaddr_in6 sockAddr;
        memset(&sockAddr, 0, sizeof(struct sockaddr_in6));
        size = sizeof(struct sockaddr_in6);
        if (0 == WSAStringToAddressA(addrStr, AF_INET6, NULL, (struct sockaddr*)&sockAddr, &size))
        {
            memcpy(dst, &sockAddr.sin6_addr, sizeof(struct in6_addr));
            return 1;
        }
    }
    return 0;
}

/**
 * @brief As setsockopt on Windows but Linux like parameter types
 * See C++ reference.
 * @return 0=Success, -1=Error
 */
int LibCpp::setsockopt(int socket, int level, int optname, void* optval, int optlen)
{
    int res = ::setsockopt(socket, level, optname, (const char*) optval, optlen);
    return res;
}

cWinIp::cWinIp() :
    dbg("cWinIp")
{
    cDebug dbg("cWinIp", &this->dbg);
    this->dbg.setInstanceName("winIp");

    pGlobalIpInstance = this;

    WORD wVersionRequested = MAKEWORD(2, 2);
    WSADATA wsaData;
    if (WSAStartup(wVersionRequested, &wsaData) != 0)
    {
        dbg.printf(enDebugLevel_Fatal, "Socket functions could not be opened (WSAStartup)!");
    }
}

cWinIp::~cWinIp()
{
    WSACleanup();
    if (pWinIp && pWinIp != (cWinIp*)1)
    {
        cWinIp* pLocal = pWinIp;
        pWinIp = nullptr;
        delete pLocal;
    }
    pGlobalIpInstance = nullptr;
}

/**
 * @brief Retruns the global instance of windows related ethernet ip support functions
 * @return
 */
cWinIp& cWinIp::instance()
{
    cIpCheckInstance();
    return *pWinIp;
}

/**
 * @brief Windows implementation of LibCpp::cIp::getInterfaces.
 * @param result See LibCpp::cIp::getInterfaces.
 * @return
 */
enIpResult cWinIp::getInterfaces(cInterfaces* result)
{
    cDebug dbg("getInterfaces", &this->dbg);
    dbg.setInstanceName("winIp");

    PIP_ADAPTER_ADDRESSES pAdapterBuffer = NULL;
    PIP_ADAPTER_ADDRESSES pAdapter = NULL;
    ULONG outBufLen = cWinIp_INTERFACEBUFFERSIZE;
    DWORD dwRetVal = 0;
    ULONG flags = GAA_FLAG_INCLUDE_PREFIX;

    pAdapterBuffer = (IP_ADAPTER_ADDRESSES *) malloc(outBufLen);
    if (pAdapterBuffer == NULL)
    {
        dbg.printf(enDebugLevel_Fatal, "Memory allocation failed!");
        return enIpResult_Failure;
    }

    dwRetVal = GetAdaptersAddresses(AF_UNSPEC, flags, NULL, pAdapterBuffer, &outBufLen);
    if (dwRetVal == ERROR_BUFFER_OVERFLOW)
    {
        free(pAdapterBuffer);
        dbg.printf(enDebugLevel_Fatal, "Not enough memory allocated! Increase #define cWinIp_INTERFACEBUFFERSIZE from actual value %i!\n", (int)cWinIp_INTERFACEBUFFERSIZE);
        return enIpResult_Failure;
    }

    pAdapter = pAdapterBuffer;
    while (pAdapter)
    {
        PIP_ADAPTER_UNICAST_ADDRESS_XP pAddr = (PIP_ADAPTER_UNICAST_ADDRESS_XP) pAdapter->FirstUnicastAddress;
        while (pAddr)
        {
            stInterfaceInfo info;
            PIP_ADAPTER_UNICAST_ADDRESS_LH2 pAddr2 = (PIP_ADAPTER_UNICAST_ADDRESS_LH2) pAddr;

            wstring wcsHelp(pAdapter->FriendlyName);
            info.frientlyName = string(wcsHelp.begin(), wcsHelp.end());
            info.name = string(pAdapter->AdapterName);
            bool loopBack = pAdapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK;
            bool up = pAdapter->OperStatus == IfOperStatusUp;
            info.up = up && !loopBack;
            /// https://stackoverflow.com/questions/221181/how-can-i-access-netstat-like-ethernet-statistics-from-a-windows-program

            if (pAddr->Address.lpSockaddr->sa_family == AF_INET)
            {
                // Setting and storing adapter info struct
                info.ifId = pAdapter->IfIndex;
                unSockAddr* pIpAddr = (unSockAddr*)pAddr->Address.lpSockaddr;
                info.ipAddress = instance().fromSockAddr(pIpAddr, pAdapter->IfIndex);

                // Set multicast address
                unsigned long mask = 0;
                ConvertLengthToIpv4Mask(pAddr2->OnLinkPrefixLength, &mask);
                unsigned long multAddr = (pIpAddr->in.sin_addr.S_un.S_addr & mask) | ~mask;
                unSockAddr multSockAddr;
                memset(&multSockAddr, 0, sizeof(unSockAddr));
                multSockAddr.in.sin_addr.S_un.S_addr = multAddr;
                multSockAddr.in.sin_family = AF_INET;
                info.ipMulticast = instance().fromSockAddr(&multSockAddr, pAdapter->IfIndex);

                result->list.push_back(info);
            }
            if (pAddr->Address.lpSockaddr->sa_family == AF_INET6)
            {
                info.ifId = pAdapter->Ipv6IfIndex;
                info.ipAddress = instance().fromSockAddr((unSockAddr*)pAddr->Address.lpSockaddr, pAdapter->IfIndex);
                info.ipMulticast = stIpAddress("ff02::1");
                result->list.push_back(info);
            }
            pAddr = pAddr->Next;
        }
        pAdapter = pAdapter->Next;
    }

    free(pAdapterBuffer);

    if (result->list.size() == 0)
        return enIpResult_NoInterface;
    return enIpResult_Success;
}


/**
 * @brief Converts a ip address string to a hardware independent binary description
 * The input string looks like "127.0.0.1" and the conversion result will be stored in 'address'.
 * You may use 'port' to specify the port. In case 'port' is set, it will overrule
 * the interpretation possibly made from the string "127.0.0.1:55000".
 * It is not recommendet to use the parameter 'ifId'. This might lead to inconsistency
 * within the resulting struct. Use LibCpp:cInterfaces::completeInterfaceIpAddress
 * to do so in a safe manner.
 * @param address Output of the result
 * @param addressName
 * @param port
 * @param ifId
 * @return Error code enIpResult_Success, enIpResult_InvalidIpAddress
 */
enIpResult cWinIp::getIpAddress(stIpAddress& address, std::string addressName, int port, int ifId)
{
    char addrName[128];
    strcpy_s(addrName, addressName.c_str());
    unSockAddr sockaddr;
    bool isV4 = false;
    bool isV6 = false;
    int size = sizeof(unSockAddr);
    sockaddr.addr.sa_family = AF_INET;
    isV4 = WSAStringToAddressA(addrName, AF_INET, NULL, &sockaddr.addr, &size) == 0;
    if (!isV4)
    {
        size = sizeof(unSockAddr);
        sockaddr.addr.sa_family = AF_INET6;
        isV6 = WSAStringToAddressA(addrName, AF_INET6, NULL, &sockaddr.addr, &size) == 0;
        if (isV6)
        {
            sockaddr.in6.sin6_port = htons(port);
            sockaddr.in6.sin6_scope_id = ifId;
        }
    }
    else
        sockaddr.in.sin_port = htons(port);
    if (isV4 || isV6)
    {
        address = fromSockAddr(&sockaddr);
        return enIpResult_Success;
    }
    return enIpResult_InvalidIpAddress;
}

/**
 * @brief Converts system sockaddr structs to the hardware independent address representation
 * The use of parameter 'ifId' on IPv6 is not recommendet. In case it is set, it overrules the value stored in struct sockaddr_in6 (scope_id).
 * Manual setting of 'ifId' may lead to data inconsistency, use LibCpp::cInterfaces::completeInterfaceIpAddress instead.
 * @param pSockAddr
 * @param ifId Interface id required for IPv4 only.
 * @return
 */
stIpAddress cWinIp::fromSockAddr(unSockAddr* pSockAddr, int ifId)
{
    stIpAddress res;
    if (pSockAddr->addr.sa_family == AF_INET)
    {
        res.family = enAddressFamily_IPv4;
        res.address[0] = pSockAddr->in.sin_addr.S_un.S_un_b.s_b1;
        res.address[1] = pSockAddr->in.sin_addr.S_un.S_un_b.s_b2;
        res.address[2] = pSockAddr->in.sin_addr.S_un.S_un_b.s_b3;
        res.address[3] = pSockAddr->in.sin_addr.S_un.S_un_b.s_b4;
        res.port = htons(pSockAddr->in.sin_port);
        res.ifId = ifId;
    }
    if (pSockAddr->addr.sa_family == AF_INET6)
    {
        res.family = LibCpp::enAddressFamily_IPv6;
        for (int i=0; i<8; i++)
            res.address[i] = htons(pSockAddr->in6.sin6_addr.u.Word[i]);
        res.port = htons(pSockAddr->in6.sin6_port);
        if (ifId)
            res.ifId = ifId;
        else
            res.ifId = pSockAddr->in6.sin6_scope_id;
    }
    return res;
}

/**
 * @brief Converts the hardware independent ethernet address to the system like 'struct sockaddr' representation.
 * @param ipAddress
 * @param pIfId If set, output for the inderface ID. Unseful for IPv4.
 * @return
 */
unSockAddr cWinIp::toSockAddr(stIpAddress* pIpAddress, int* pIfId)
{
    unSockAddr res;
    memset(&res, 0, sizeof(unSockAddr));
    if (pIfId) *pIfId = 0;

    if (pIpAddress->family == enAddressFamily_IPv4)
    {
        res.addr.sa_family = AF_INET;
        res.in.sin_addr.S_un.S_un_b.s_b1 = pIpAddress->address[0];
        res.in.sin_addr.S_un.S_un_b.s_b2 = pIpAddress->address[1];
        res.in.sin_addr.S_un.S_un_b.s_b3 = pIpAddress->address[2];
        res.in.sin_addr.S_un.S_un_b.s_b4 = pIpAddress->address[3];
        res.in.sin_port = htons(pIpAddress->port);
        if (pIfId)
            *pIfId = pIpAddress->ifId;
    }
    if (pIpAddress->family == enAddressFamily_IPv6)
    {
        res.addr.sa_family = AF_INET6;
        for (int i=0; i<8; i++)
            res.in6.sin6_addr.u.Word[i] = htons(pIpAddress->address[i]);
        res.in.sin_port = htons(pIpAddress->port);
        res.in6.sin6_scope_id = pIpAddress->ifId;
        if (pIfId)
            *pIfId = pIpAddress->ifId;
    }
    return res;
}

/** @} */
