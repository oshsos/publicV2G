// UTF8 (ü) //
/**
\file cLxIp.cpp

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2023-01-02

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL_Linux
@{

\class LibCpp::cLxIp
\brief Virtual object providing (virtual static) hardware independent ethernet ip support methods

To call methods use e.g. 'cLxIp::instance().getInterfaces(...).\n
\n
This is a virtual interface object as the functional implementation is hardware dependent. In prinziple
this object provides global static functions. As it is not possible to override static methods, they
need to be provided through a global instance without any member variable. This way they are not
static but are treated to be.

To do:
- Addressing per dns (name server)
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

#include "cLxIp.h"
#include "../cDebug.h"

using namespace std;
using namespace LibCpp;

extern cIp* pGlobalIpInstance;  ///< cIp type pointer to the unique instance winIp, to be called by cIp::instance(). ... !
cLxIp*      pLxIp = nullptr;    ///< Unique instance of cLxIp class. Instance is to be called by cLxIp::instance(). ... !

/** @brief Long/byte union */
union unLongByte
{
    unsigned long l;
    unsigned char b[4];
};

class cLxIpDeleter
{
public:
    ~cLxIpDeleter()
    {
        if (pLxIp)
        {
            cLxIp* pLocal = pLxIp;
            pLxIp = nullptr;
            delete pLocal;
        }
    }
};

cLxIpDeleter lxIpDeleter;

void LibCpp::cIpCheckInstance()
{
    if (!pLxIp)
    {
        pLxIp = (cLxIp*)1;
        pLxIp = new(cLxIp);
        pGlobalIpInstance = pLxIp;
    }
}


cLxIp::cLxIp() :
    dbg("cLxIp")
{
    this->dbg.setInstanceName("lxIp");

    pGlobalIpInstance = this;
}

cLxIp::~cLxIp()
{
    if (pLxIp)
    {
        cLxIp* pLocal = pLxIp;
        pLxIp = nullptr;
        delete pLocal;
    }
    pGlobalIpInstance = nullptr;
}

/**
 * @brief Retruns the global instance of windows related ethernet ip support functions
 * @return
 */
cLxIp& cLxIp::instance()
{
    cIpCheckInstance();
    return *pLxIp;
}
/**
 * @brief Windows implementation of LibCpp::cIp::getInterfaces.
 * @param result See LibCpp::cIp::getInterfaces.
 * @return Error code: enIpResult_Success, enIpResult_NoInterface, enIpResult_Failure
 */
enIpResult cLxIp::getInterfaces(cInterfaces* result)
{
    cDebug dbg("getInterfaces", &this->dbg);
    dbg.setInstanceName("winIp");

    struct ifaddrs *ifaddr, *ifa;

    if (getifaddrs(&ifaddr) == -1)
    {
        dbg.printf(enDebugLevel_Fatal, "Failed to identify adapters: Check for external adapters being plugged in!");
        return enIpResult_Failure;
    }

    ifa = ifaddr;
    do
    {
        stInterfaceInfo ifcInfo;

        ifcInfo.frientlyName = string(ifa->ifa_name);
        ifcInfo.ifId = if_nametoindex(ifcInfo.frientlyName.c_str());
        if (ifa->ifa_addr)
        {
            unSockAddr* pAddr = (unSockAddr*)ifa->ifa_addr;
            ifcInfo.ipAddress = instance().fromSockAddr(pAddr, ifcInfo.ifId);

            if (ifa->ifa_netmask && pAddr->addr.sa_family == AF_INET)
            {
                ifcInfo.up = pAddr->in.sin_addr.s_addr != 0;

                unSockAddr* pMask = (unSockAddr*)ifa->ifa_netmask;
                unsigned long mask = pMask->in.sin_addr.s_addr;
                unsigned long multAddr = (pAddr->in.sin_addr.s_addr & mask) | ~mask;
                unSockAddr multSockAddr;
                memset(&multSockAddr, 0, sizeof(unSockAddr));
                multSockAddr.in.sin_addr.s_addr = multAddr;
                multSockAddr.in.sin_family = AF_INET;
                ifcInfo.ipMulticast = instance().fromSockAddr(&multSockAddr);
            }
            else if (pAddr->addr.sa_family == AF_INET)
            {
                ifcInfo.up = pAddr->in.sin_addr.s_addr != 0;
            }
            else if (pAddr->addr.sa_family == AF_INET6)
            {
                bool isEmpty = true;
                for (int i=0; i<16; i++)
                    if (pAddr->in6.sin6_addr.s6_addr[i])
                        isEmpty = false;
                ifcInfo.up = !isEmpty;

                ifcInfo.ipMulticast = stIpAddress("ff02::1");
            }
            ifcInfo.up = ifcInfo.up && (ifa->ifa_flags & IFF_UP) && !(ifa->ifa_flags & IFF_LOOPBACK);
        }
        result->list.push_back(ifcInfo);
        ifa = ifa->ifa_next;
    } while(ifa != 0);

    freeifaddrs(ifaddr);

    if (result->list.size() == 0)
        return enIpResult_NoInterface;
    return enIpResult_Success;
}

/**
 * @brief Converts an ip address string to a hardware independent binary description
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
enIpResult cLxIp::getIpAddress(stIpAddress& address, std::string addressName, int port, int ifId)
{
    unSockAddr sockAddr;
    bool isV4 = false;
    bool isV6 = false;
    sockAddr.addr.sa_family = AF_INET;
    if (addressName.front() == '[')
        addressName = addressName.substr(1, addressName.size()-2);
    isV4 = inet_pton(AF_INET, addressName.c_str(), &(sockAddr.in.sin_addr)) == 1;
    if (!isV4)
    {
        sockAddr.addr.sa_family = AF_INET6;
        isV6 = inet_pton(AF_INET6, addressName.c_str(), &(sockAddr.in6.sin6_addr)) == 1;
        if (isV6)
        {
            sockAddr.in6.sin6_port = htons(port);
            sockAddr.in6.sin6_scope_id = ifId;
        }
    }
    else
    {
        sockAddr.in.sin_port = htons(port);
    }
    if (isV4 || isV6)
    {
        address = fromSockAddr(&sockAddr);
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
stIpAddress cLxIp::fromSockAddr(unSockAddr* pSockAddr, int ifId)
{
    stIpAddress res;
    if (pSockAddr->addr.sa_family == AF_INET)
    {
        res.family = enAddressFamily_IPv4;
        unLongByte addrVal;
        addrVal.l = pSockAddr->in.sin_addr.s_addr;
        res.address[0] = addrVal.b[0];
        res.address[1] = addrVal.b[1];
        res.address[2] = addrVal.b[2];
        res.address[3] = addrVal.b[3];
        res.port = htons(pSockAddr->in.sin_port);
        res.ifId = ifId;
    }
    if (pSockAddr->addr.sa_family == AF_INET6)
    {
        res.family = LibCpp::enAddressFamily_IPv6;
        unsigned short* pAdr6 = (unsigned short*)&pSockAddr->in6.sin6_addr.s6_addr;
        for (int i=0; i<8; i++)
            res.address[i] = htons(pAdr6[i]);
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
unSockAddr cLxIp::toSockAddr(stIpAddress* pIpAddress, int* pIfId)
{
    unSockAddr res;
    memset(&res, 0, sizeof(unSockAddr));
    if (pIfId) *pIfId = 0;

    if (pIpAddress->family == enAddressFamily_IPv4)
    {
        res.addr.sa_family = AF_INET;
        unLongByte addrVal;
        addrVal.b[0] = pIpAddress->address[0];
        addrVal.b[1] = pIpAddress->address[1];
        addrVal.b[2] = pIpAddress->address[2];
        addrVal.b[3] = pIpAddress->address[3];
        res.in.sin_addr.s_addr = addrVal.l;
        res.in.sin_port = htons(pIpAddress->port);
        if (pIfId)
            *pIfId = pIpAddress->ifId;
    }
    if (pIpAddress->family == enAddressFamily_IPv6)
    {
        res.addr.sa_family = AF_INET6;
        unsigned short* pAddr6 = (unsigned short*)&res.in6.sin6_addr.s6_addr;
        for (int i=0; i<8; i++)
            pAddr6[i] = htons(pIpAddress->address[i]);
        res.in.sin_port = htons(pIpAddress->port);
        res.in6.sin6_scope_id = pIpAddress->ifId;
        if (pIfId)
            *pIfId = pIpAddress->ifId;
    }
    return res;
}

/** @} */




/******************************************************/
///**
// * @brief \b static Finds the adapter Id from a given name e.g. 'eth0'
// * @param adapters friendly name e.g.
// * @return adapter Id, 0 in case of failure
// */
//int cLxIp::getAdapterId(const char* adapter)
//{
//    cDebug dbg("cLxIp::getAdapterId");

//    int ifIndex = if_nametoindex(adapter);
//    if (ifIndex == 0)
//       dbg.printf(enDebugLevel_Error, "False adapter name '%s': %i %s!\n", adapter, errno, strerror(errno));
//    return ifIndex;

//    // ******* eventuell wichtig für Windows-Implementierung *************
//    //    struct ifreq ifr;

//    //    int sock = socket(AF_LOCAL, SOCK_RAW, 0);
//    //    if (sock <= INVALID_SOCKET)
//    //    {
//    //        //dbg.printf(enDebugLevel_Error, "Could not open a socket!");
//    //        return enIpResult_SocketFailed;
//    //    }

//    //    bzero(&ifr , sizeof(ifr));
//    //    strncpy((char *)ifr.ifr_name, adapter , IFNAMSIZ);
//    //    if((ioctl(sock, SIOCGIFINDEX, &ifr)) == -1)
//    //    {
//    //        //dbg.printf(enDebugLevel_Error, "Unable to find interface index!");
//    //        close(sock);
//    //        return enIpResult_InvalidInterfaceId;
//    //    }

//    //    close(sock);
//    //    return ifr.ifr_ifindex;
//}




//void cLxIp::printAdapters(adapters_t adapters)
//{
//    printf("Adapter name: Adapter id\n");

//    for(stAdapterInfo adapter : adapters)
//    {
//        printf("%s: %d\n", adapter.name, adapter.ifId);
//    }
//}

//int cLxIp::getAdapterId(struct sockaddr* address)
//{
//    cDebug dbg("cLxIp::getAdapterId");

//    int ifIndex = 0;
//    struct ifaddrs* ifAddrStruct = nullptr;
//    struct ifaddrs* ifa = nullptr;

//    if (address->sa_family != AF_INET6 && address->sa_family != AF_INET)
//    {
//        dbg.printf(enDebugLevel_Error, "Invalid socket address!");
//        return 0;
//    }
//    if (getifaddrs(&ifAddrStruct) == -1)
//    {
//        dbg.printf(enDebugLevel_Error, "Function 'getifaddrs' failed!");
//        return 0;
//    }

//    for (ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next)
//    {
//        if (ifa ->ifa_addr->sa_family==address->sa_family)
//        {
//            if (address->sa_family == AF_INET)
//            {
//                struct sockaddr_in* pAddr = (struct sockaddr_in*) ifa->ifa_addr;
//                struct sockaddr_in* pAddress = (struct sockaddr_in*) address;
//                if (memcmp(&pAddress->sin_addr, &pAddr->sin_addr, sizeof (struct in_addr)) == 0)
//                {
//                    ifIndex = lxIp.getAdapterId(ifa->ifa_name);
//                    break;
//                }
//            }
//            else // AF_INET6
//            {
//                struct sockaddr_in6* pAddr = (struct sockaddr_in6*) ifa->ifa_addr;
//                struct sockaddr_in6* pAddress = (struct sockaddr_in6*) address;
//                if (memcmp(&pAddress->sin6_addr, &pAddr->sin6_addr, sizeof (struct in6_addr)) == 0)
//                {
//                    ifIndex = lxIp.getAdapterId(ifa->ifa_name);
//                    break;
//                }
//            }
//        }
//    }
//    freeifaddrs(ifAddrStruct);
//    if (ifIndex == 0)
//        dbg.printf(enDebugLevel_Error, "Failed to find adapter ID from socket address!");
//    return ifIndex;
//}

//void cLxIp::getAdapterAddress(char* addressBuffer, int addressBufferSize, int ifId, enAddressFamily addressFamily)
//{
//    //cDebug _dbg(0, "cLxTcpServer");
//    //cDebug dbg(&_dbg, "getInterfaceAddress");
//    struct sockaddr address;
//    unsigned short af = AF_INET;

//    addressBuffer[0] = 0;
//    if (addressFamily == enAddressFamily_ipV6)
//        af = AF_INET6;
//    address.sa_family = af;
//    int res;
//    if ((res=getAdapterSockAddr(&address, ifId))!=0)
//    {
//        addressBuffer[0] = 0;
//        //dbg.printf(enDebugLevel_Debug, "Fetching IP address from interface id %d and %s protocol failed: Check if the interface has an connection in up state to the specified protocol (use 'ifconfig' in terminal)!", ifId, enAddressFamilyToString(addressFamily));
//        return;
//    }
//    if (af == AF_INET6)
//        inet_ntop(af, &(((struct sockaddr_in6*)&address)->sin6_addr), addressBuffer, addressBufferSize);
//    else
//        inet_ntop(af, &(((struct sockaddr_in*)&address)->sin_addr), addressBuffer, addressBufferSize);
//    return;
// }

//enIpResult cLxIp::getAdapterSockAddr(struct sockaddr* address, int ifId)
//{
//    //cDebug _dbg(0, "cLxTcpServer");
//    //cDebug dbg(&_dbg, "getInterfaceAddress");
//    struct ifaddrs* ifAddrStruct = nullptr;
//    struct ifaddrs* ifa = nullptr;
//    enIpResult res = enIpResult_InvalidIpAddress;

//    if (address->sa_family != AF_INET6)
//        address->sa_family = AF_INET;
//    if (getifaddrs(&ifAddrStruct) == -1)
//    {
//        //dbg.printf(enDebugLevel_Error, "Get ifaddrs failed!");
//        return enIpResult_AdapterListFailed;
//    }

//    for (ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next)
//    {
//        if (ifa ->ifa_addr->sa_family==address->sa_family)
//        {
//            if (ifId == lxIp.getAdapterId(ifa->ifa_name))
//            {
//                *address = *ifa->ifa_addr;
//                res = enIpResult_Success;
//                break;
//            }
//        }
//    }
//    freeifaddrs(ifAddrStruct);
//    return res;
//}

//enIpResult cLxIp::getSockAddr(struct sockaddr_in6* pSockAddr, const char* address, int port)
//{
//    if (!pSockAddr) return enIpResult_Failure;
//    struct sockaddr* pSock = (struct sockaddr*)pSockAddr;

//    bzero(pSockAddr, sizeof(struct sockaddr_in6));
//    pSock->sa_family = AF_INET;
//    struct sockaddr_in* pSockAddrIn = (struct sockaddr_in*)pSockAddr;
//    if (inet_pton(AF_INET, address, &(pSockAddrIn->sin_addr))==1)
//    {
//        pSockAddrIn->sin_port = htons(port);
//        return enIpResult_Success;
//    }

//    bzero(pSockAddr, sizeof(struct sockaddr_in6));
//    pSock->sa_family = AF_INET6;
//    struct sockaddr_in6* pSockAddrIn6 = (struct sockaddr_in6*)pSockAddr;
//    if (inet_pton(AF_INET6, address, &(pSockAddrIn6->sin6_addr))==1)
//    {
//        pSockAddrIn6->sin6_port = htons(port);
//        return enIpResult_Success;
//    }

//    return enIpResult_InvalidIpAddress;
//}