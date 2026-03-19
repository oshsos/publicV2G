// UTF8 (ü) //
/**
\file cIp.cpp

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2023-01-02

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL
@{

\class LibCpp::cIp
\brief Virtual object providing (virtual static) hardware independent ethernet ip support methods

To call methods use e.g. 'cIp::instance().getInterfaces(...).\n
\n
This is a virtual interface object as the functional implementation is hardware dependent. In prinziple
this object provides global static functions. As it is not possible to override static methods, they
need to be provided through a global instance without any member variable. This way they are not
static but are treated to be.

To do:
- Automatic interface detection (which is active)
- Automatic interface detection through destination address, and setting destination in this case
- Addressing per dns (name server)
**/

#undef UNICODE
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS         // required for Visual Studio compiler

#define cWinIp_INTERFACEBUFFERSIZE 16384

#include "cIp.h"
#include "Tools.h"
#include "cDebug.h"

using namespace std;
using namespace LibCpp;

cIp* pGlobalIpInstance = nullptr;   ///< Pointer to the hardware dependent global instance providing the virtual methods implementations. The pointer is to be called by cIp::instance(). ... !

namespace LibCpp
{
void cIpCheckInstance();
}

/** @brief Converts an enIpResult to a string */
string LibCpp::enIpResult_toString(enIpResult result)
{
    #ifdef LANGUAGE_DE
    switch (result)
    {
        case enIpResult_None: return "Kein Ergebnis."; break;
        case enIpResult_Success: return "Erfolg."; break;
        case enIpResult_Failure: return "Allgemeiner Fehler!"; break;
        case enIpResult_InvalidIpAddress: return "Die IP-Adresse ist nicht vorhanden oder ungueltig!"; break;
        case enIpResult_SocketFailed: return "Es konnte kein 'socket' erzeugt werden!"; break;
        case enIpResult_SocketOptionFailed: return "Eine 'socket' option konnte nicht gesetzt werden!"; break;
        case enIpResult_InvalidInterfaceId: return "Der Index des Internet Interfaces konnte nicht gefunden werden!"; break;
        case enIpResult_InterfaceListFailed: return "Die Liste der Interfaces konnte nicht gelesen werden!"; break;
        case enIpResult_AddrInfoFailed: return "Der Adressinformationen konnten nicht gelesen werden!"; break;
        case enIpResult_BindFailed: return "Der 'socket' konnte nicht gebunden werden!"; break;
        case enIpResult_ConnectFailed: return "Der 'socket' konnte nicht verbunden werden!"; break;
        case enIpResult_ListenFailed: return "Der 'socket' konnte nicht als Server konfigurert werden ('listen')!"; break;
        case enIpResult_InvalidFamily: return "Die IP Familie ist ungültig!"; break;
        case enIpResult_InvalidInterface: return "Das Interface ist nicht gültig"; break;
        case enIpResult_NoInterface: return "Das System besitzt kein Ethernet Interface"; break;
        default: return "Unbekannter Fehler!";
    }
    #else
    switch (result)
    {
        case enIpResult_None: return "No result."; break;
        case enIpResult_Success: return "Success."; break;
        case enIpResult_Failure: return "General failure!"; break;
        case enIpResult_InvalidIpAddress: return "The IP-address is not present or invalid!"; break;
        case enIpResult_SocketFailed: return "Could not open a socket!"; break;
        case enIpResult_SocketOptionFailed: return "A 'socket' option could not be set!"; break;
        case enIpResult_InvalidInterfaceId: return "Unable to find the interface index!"; break;
        case enIpResult_InterfaceListFailed: return "Unable to read the interaface list!"; break;
        case enIpResult_AddrInfoFailed: return "The address information could not be read!"; break;
        case enIpResult_BindFailed: return "Unable to bind the socket!"; break;
        case enIpResult_ConnectFailed: return "Unable to connect the socket!"; break;
        case enIpResult_ListenFailed: return "Configuring the socket to listen as server failed!"; break;
        case enIpResult_InvalidFamily: return "Address family is not valid!"; break;
        case enIpResult_InvalidInterface: return "The interface is not valid!"; break;
        case enIpResult_NoInterface: return "There is no ethernet interface present on the system!"; break;
        default: return "Unknown Error!";
    }
    #endif
}

/** @brief Converts an enAddressFamily to a string */
string LibCpp::enAddressFamily_toString(enAddressFamily family)
{
    if (family == enAddressFamily_IPv6)
        return "IPv6";
    else if (family == enAddressFamily_IPv4)
        return "IPv4";
    else
        return "IP_unknown";
}

/** @brief Initializes a stIpAddress */
stIpAddress::stIpAddress()
{
    family = enAddressFamily_None;
    for (int i=0; i<8; i++)
        address[i] = 0;
    port = 0;
    ifId = 0;
}

/**
 * @brief Constructs a stIpAddress instance from a string type name
 * In case the string is an unvalid ip address, an empty instance is created.
 * You can check for an empty stIpAddress by evaluating the member 'family' to be 'enAddressFamily_None'.
 * This method calls hardware dependend methods and requires the existence of a LibCpp::cIp derived class instance (see: LibCpp::cIp::getIpAddress).
 * It is not recommended to use parameter 'ifId' as this might lead to data inconsistency
 * within the stIpAddress struct. If the value 'ifId' is required, use the method LibCpp::cInterfaces::completeInterfaceIpAddress
 * instead, although it requires an instance of LibCpp::cInterfaces.
 * @param ipAddressName
 * @param port
 * @param ifId
 */
stIpAddress::stIpAddress(std::string ipAddressName, int port, int ifId)
{
    family = enAddressFamily_None;
    memset(address, 0, 8*sizeof(int));
    this->port = 0;
    this->ifId = 0;

    stIpAddress out;
    if ( cIp::instance().getIpAddress(out, ipAddressName, port, ifId) == enIpResult_Success )
    {
        *this = out;
    }
}

stIpAddress::stIpAddress(const char* ipAddressName)
{
    family = enAddressFamily_None;
    memset(address, 0, 8*sizeof(int));
    this->port = 0;
    this->ifId = 0;

    stIpAddress out;
    if ( cIp::instance().getIpAddress(out, ipAddressName, port, ifId) == enIpResult_Success )
    {
        *this = out;
    }
}

/**
 * @brief Constructs a IPv6 stIpAddress instance from an array of eigth uint16_t values
 * It is not recommended to use parameter 'ifId' as this might lead to data inconsistency
 * within the stIpAddress struct. If the value 'ifId' is required, use the method LibCpp::cInterfaces::completeInterfaceIpAddress
 * instead, although it requires an instance of LibCpp::cInterfaces.
 * @param ipAddressName
 * @param port
 * @param ifId
 */
stIpAddress::stIpAddress(uint16_t ipAddressBytes[8], int port, int ifId)
{
    family = enAddressFamily_IPv6;
    for (int i=0; i<8; i++)
        address[i] = ipAddressBytes[i];
    this->port = port;
    this->ifId = ifId;
}


/** @brief Converts a stIpAddress to a string */
string stIpAddress::toString(bool printPort, bool printInterface)
{
    string out;
    int i;
    char txt[64];
    int cnt=0;
    if (family == enAddressFamily_IPv6)
    {
        out = string("[");
        for (i = 0; i < 7; i++)
        {
            if (address[i]!=0 || cnt>2)
            {
                sprintf(txt, "%x", address[i]);
                out += string(txt) + ":";
                if (cnt<2) cnt=1;
                if (cnt==2) cnt++;
            }
            else
            {
                if (cnt<2)
                {
                    out += string(":");
                    cnt++;
                }
            }
        }
        if (address[i]!=0)
        {
            sprintf(txt, "%x", address[i]);
            out += string(txt);
        }
        if (printInterface)
            out += string("%") + to_string(ifId) + string("]");
        else
            out += string("]");
    }
    else // enAddressFamily_ipV4
    {
        for (i = 0; i < 3; i++)
        {
            out += to_string(address[i]) + string(".");
        }
        out += to_string(address[i]);
        if (printInterface)
            out += string("%") + to_string(ifId);
    }
    if (printPort)
    {
        if (port)
            out += string(":") + to_string(port);
        else
            out += ":no";
    }
    return out;
}

/**
 * @brief Compares two stIpAddress instances to have equal family and address
 * The comparison does not take into account the content of the members
 * 'port' and 'ifId'. In case one of both addresses family is undefined, the result is 'false'.
**/
bool stIpAddress::hasEqualAddress(const stIpAddress& ref) const
{
    if (family == enAddressFamily_None || ref.family == enAddressFamily_None) return false;
    bool equal = (family == ref.family);
    equal = equal && (memcmp(&address, &ref.address, 8*sizeof(unsigned int))==0);
    return equal;
}

/** @brief Equal operator on all struct members */
bool stIpAddress::operator==(const stIpAddress& ref) const
{
    bool equal = this->hasEqualAddress(ref);
    equal = equal && (port == ref.port);
    equal = equal && (ifId == ref.ifId);
    return equal;
}

/** @brief Initializes a stIpAddress */
stInterfaceInfo::stInterfaceInfo()
{
    ifId = 0;
    up = false;
}

/** @brief Converts a stInterfaceInfo to a string */
string stInterfaceInfo::toString()
{
    return stringFormat("%4i %30s %45s %2s %44s", ifId, frientlyName.c_str(), ipAddress.toString(false, false).c_str(), up ? "up" : "", name.c_str());
}

/**
 * @brief Constructor
 */
cIp::cIp() :
    dbg("cIp")
{
    this->dbg.setInstanceName("*pGlobalIpInstance");
    pGlobalIpInstance = &instance();
}

/**
 * @brief Destructor
 */
cIp::~cIp()
{
    // pGlobalIpInstance = nullptr;
}

/**
 * @brief Retrieves the global hardware specific cIp instance
 * Deliveres access to the hardware dependent global instance providing the "virtually static" methods.
 * @return
 */
cIp& cIp::instance()
{
    cIpCheckInstance();
    return *pGlobalIpInstance;
}
//cIp* pMethods()
//{
//    return pGlobalIpInstance;
//}

#pragma GCC diagnostic ignored "-Wunused-parameter"
/**
 * @brief Retrieves a list of ethernet interfaces present on the system
 * The list of interfaces is stored to the provided LibCpp::cInterfaces instance.
 * This method is hardware dependent and not available on self declared cIp instances.
 * @param result Pointer to the LibCpp::cInterfaces instance
 * @return Error Code: enIpResult_Success, enIpResult_NoInterface, enIpResult_Failure
 */
enIpResult cIp::getInterfaces(cInterfaces* result)
{
    cDebug dbg("getInterfaces", &this->dbg);
    dbg.printf(enDebugLevel_Fatal, "This method is only available on a hardware dependent implementation of this class! Add e.g. cWinIp.cpp to your project files.");
    return LibCpp::enIpResult_Failure;
}

/**
 * @brief Retrieves a hardware independent binary ip address representation from a string
 * This method is hardware dependent and not available on self declared cIp instances.
 * In principle this message could be implemented hardware independent bu so far system
 * calls are used for this transformation.\n
 * It is not recommended to use parameter 'ifId' as this might lead to data inconsistency
 * within the stIpAddress struct. If the value of 'ifId' is required, use the method
 * LibCpp::cInterfaces::completeInterfaceIpAddress instead, although it requires an
 * instance of LibCpp::cInterfaces.
 * @param result Instance where the transformation result is stored to.
 * @param addressName String of the address name e.g. "127.0.0.1"
 * @param port Optional Stores the port to the result.
 * @param ifId Optional and not recommended to use! Stores the port to the result.
 * @return
 */
enIpResult cIp::getIpAddress(stIpAddress& result, std::string addressName, int port, int ifId)
{
    cDebug dbg("getIpAddress", &this->dbg);
    dbg.printf(enDebugLevel_Fatal, "This method is only available on a hardware dependent implementation of this class! Add e.g. cWinIp.cpp to your project files.");
    return LibCpp::enIpResult_Failure;
}
#pragma GCC diagnostic warning "-Wunused-parameter"

/**
 * @brief Creates a string representation optimized for debugging: length=34 <newline> 0x 01 02
 * @param message
 * @param messageLen
 * @return
 */
string cIp::messageString(const char* message, unsigned int messageLen)
{
    string out = string("length = ") + to_string(messageLen) + string("\nmessage = ");
    out += ByteArrayToString(message, messageLen);
    return out;
}

/**
 * @brief Creates a string representation optimized for debugging: length=34 <newline> 0x 01 02
 * @param message
 * @param messageLen
 * @return
 */
string cIp::messageString(const uint8_t* message, unsigned int messageLen)
{
    return messageString((char*)message, messageLen);
}

/** @brief Constructor */
cInterfaces::cInterfaces(bool open) :
    dbg("cInterfaces")
{
    getInterfacesResult = enIpResult_None;
    cIp::instance();
    if (open)
        this->open();
}

/** @brief Destructor */
cInterfaces::~cInterfaces()
{
    close();
}

/** @brief Retreives the list of ethernet interfaces
 *  This method is automatically called by the constructor but may be used
 *  after a close call.
 */
void cInterfaces::open()
{
    getInterfacesResult = cIp::instance().getInterfaces(this);
}

/** @brief Clears the list of ethernet interfaces
 *  This method is automatically called by the destructor but may be used
 *  to clear the list in order to clear the memory resources.
 */
void cInterfaces::close()
{
    getInterfacesResult = enIpResult_None;
    list.clear();
}

/**
 * @brief Checks a valid status of the member 'list'.
 * @param dbg Debug logger to be used for failure messages
 * @return
 */
bool cInterfaces::checkList(cDebug& dbgInst)
{
    cDebug dbg("checkList", &dbgInst);
    if (getInterfacesResult == enIpResult_Success)
    {
        if (list.size()==0)
            dbg.printf(enDebugLevel_Error, "%s", enIpResult_toString(enIpResult_NoInterface).c_str());
        else return true;
        return false;
    }
    else if (getInterfacesResult == enIpResult_None)
    {
        dbg.printf(enDebugLevel_Fatal, "Instance is not opened! Open instance using 'open' after a previous call of 'close'.");
    }
    else if (getInterfacesResult == enIpResult_NoInterface)
    {
        dbg.printf(enDebugLevel_Error, "%s", enIpResult_toString(enIpResult_NoInterface).c_str());
    }
    return false;
}

/**
 * @brief Retrieves the ethernet address from either an address string or an interface string.
 * The result is the hardware independent ethernet address representation from
 * either an address string like "127.0.0.1" (parameter 'family' is ignored)
 * or an interface friendly name like "eth0" (Linux) or "LAN-Verbindung* 2" (Windows).
 * For interface names the 'family' parameter is required and set to IPv4 as standard value.
 * In case 'interfaceOrAddressName' is empty or not parameters are set (except 'result'),
 * the first found active (and not loop back) interface is chosen.
 * @param result Struct the address is stored to.
 * @param interfaceOrAddressName Address or interface name
 * @param family
 * @param linkLocal If set and IPv6, only link local addresses (fe80:...) are accepted. Otherwise any address is taken.
 * @param port
 * @return Error code: enIpResult_Success, enIpResult_Failure
 */
enIpResult cInterfaces::getInterfaceIpAddress(stIpAddress& result, string interfaceOrAddressName, enAddressFamily family, bool linkLocal, int port)
{
    cDebug dbg("getInterfaceIpAddress", &this->dbg);

    if (!checkList(dbg))
        return getInterfacesResult;

    stIpAddress reqAddr;
    cIp::instance().getIpAddress(reqAddr, interfaceOrAddressName);

    // Check für all interfaces request
    unsigned int end = 4;
    unsigned int i;
    if (reqAddr.family == enAddressFamily_IPv6)
        end = 8;
    for (i=0; i<end; i++)
        if (reqAddr.address[i])
            break;
    if (i<end)
    {
        result = reqAddr;
        return enIpResult_Success;
    }

    for (stInterfaceInfo& ifc : list)
    {
        if (ifc.ipAddress.family == family)
        {
            if (ifc.frientlyName == interfaceOrAddressName ||  (interfaceOrAddressName.empty() && ifc.up))
            {
                if (family == enAddressFamily_IPv6 && linkLocal)
                {
                    if (ifc.ipAddress.address[0] == 0xfe80)
                    {
                        result = ifc.ipAddress;
                        if (port) result.port = port;
                        return enIpResult_Success;
                    }
                }
                else
                {
                    result = ifc.ipAddress;
                    if (port) result.port = port;
                    return enIpResult_Success;
                }
            }
        }
        else if (ifc.ipAddress.hasEqualAddress(reqAddr))
        {
            result = ifc.ipAddress;
            if (port) result.port = port;
            return enIpResult_Success;
        }
    }
    dbg.printf(enDebugLevel_Fatal, "'%s' using %s is not valid for any interface!", interfaceOrAddressName.c_str(), enAddressFamily_toString(family).c_str());
    return enIpResult_InvalidInterface;
}

/**
 * @brief Retrieves the IpAddress of a given Interface Id
 * @param result
 * @param interfaceId
 * @param family
 * @param port
 * @return Error code
 */
enIpResult cInterfaces::getInterfaceIpAddress(stIpAddress& result, unsigned int interfaceId, enAddressFamily family, int port)
{
    cDebug dbg("getInterfaceIpAddress", &this->dbg);

    if (!checkList(dbg))
        return enIpResult_Failure;

    for (stInterfaceInfo& ifc : list)
    {
        if (ifc.ifId == interfaceId && ifc.ipAddress.family == family)
        {
            result = ifc.ipAddress;
            if (port) result.port = port;
            return enIpResult_Success;
        }
    }
    dbg.printf(enDebugLevel_Error, "Interface ID %i is not valid for any interface!", interfaceId);
    return enIpResult_InvalidInterface;
}

/**
 * @brief Adds interface information to a given address or vice versa
 * To decide the direction either 'family' or 'ifId' have to been set.
 * In case both are set, 'family' overrules and 'ifId' is ignored.
 * If INADDR_ANY for IPv4 or IPv6 is given (all 'address' entries set to zero),
 * the method will set 'ifId' to zero (invalid) but returns with sucess.
 * In case parameter 'port' is set, the value given as member of the parameter
 * 'address' will be overwritten.
 * @param address Either address and family or ifId set for input and used as result output.
 * @param port
 * @return Error code enIpResult_Success, enIpResult_Failure, enIpResult_InvalidInterface
 */
enIpResult cInterfaces::completeInterfaceIpAddress(stIpAddress& address, int port)
{
    cDebug dbg("completeInterfaceIpAddress", &this->dbg);

    if (!checkList(dbg))
        return enIpResult_Failure;

    if (address.family)
    {
        // Check for inaddr_any
        bool valid = false;
        int cnt = 4;
        if (address.family == enAddressFamily_IPv6) cnt=8;
        for (int i=0; i<cnt; i++)
            valid = valid && address.address[i];
        if (!valid)
        {
            address.ifId = 0;
            if (port) address.port = port;
            return enIpResult_Success;
        }
        // Search ifId from giben address
        for (stInterfaceInfo& ifc : list)
        {
            if (ifc.ipAddress.hasEqualAddress(address))
            {
                address.ifId = ifc.ifId;
                if (port) address.port = port;
                return enIpResult_Success;
            }
        }
        dbg.printf(enDebugLevel_Error, "Address %s is not assigned to any!", address.toString(false, false).c_str());
        return enIpResult_InvalidInterface;
    }
    else
    {
        for (stInterfaceInfo& ifc : list)
        {
            if (ifc.ifId == address.ifId && ifc.ipAddress.family == address.family)
            {
                int prt = address.port;
                address = ifc.ipAddress;
                if (port)
                    address.port = port;
                else
                    address.port = prt;
                return enIpResult_Success;
            }
        }
        dbg.printf(enDebugLevel_Error, "Interface ID %i is not valid for any interface!", address.ifId);
        return enIpResult_InvalidInterface;
    }
}

/**
 * @brief Retrieves the LibCpp::stInterfaceInfo of a given name.
 * In case the 'interfaceName' is an empty string, the inferface information of the first available active interface
 * is retrieved.
 * @param interfaceName
 * @param family
 * @param linkLocal If set and an IPv6 interface searched the link local address is returned.
 * @return
 */
stInterfaceInfo cInterfaces::getInterfaceInfo(std::string interfaceName, enAddressFamily family, bool linkLocal)
{
    if (interfaceName.empty())
    {
        for (stInterfaceInfo& ifc : list)
        {
            if (ifc.up && ifc.ipAddress.family == family)
            {
                if (family == enAddressFamily_IPv6 && linkLocal)
                {
                    if (ifc.ipAddress.address[0] == 0xfe80)
                        return ifc;
                }
                else
                    return ifc;
            }
        }
    }
    else
    {
        for (stInterfaceInfo& ifc : list)
        {
            if (ifc.frientlyName == interfaceName  && ifc.ipAddress.family == family)
            {
                if (family == enAddressFamily_IPv6 && linkLocal)
                {
                    if (ifc.ipAddress.address[0] == 0xfe80)
                        return ifc;
                }
                else
                    return ifc;
            }
        }
    }
    return stInterfaceInfo();
}

/**
 * @brief Retrieves the LibCpp::stInterfaceInfo of a given interface id
 * In case 'interfaceId' is zero a pseudo interface info with an 'friendlyName' "<All>" is retrieved.
 * @param interfaceId
 * @param family
 * @return
 */
stInterfaceInfo cInterfaces::getInterfaceInfo(unsigned int interfaceId, enAddressFamily family, bool linkLocal)
{
    if (interfaceId)
    {
        for (stInterfaceInfo& ifc : list)
        {
            if (ifc.ifId == interfaceId  && ifc.ipAddress.family == family)
            {
                if (family == enAddressFamily_IPv6 && linkLocal)
                {
                    if (ifc.ipAddress.address[0] == 0xfe80)
                        return ifc;
                }
                else
                    return ifc;
            }
        }
        return stInterfaceInfo();
    }
    else
    {
        stInterfaceInfo res;
        res.frientlyName = "<All>";
        return res;
    }
}

/**
 * @brief Creates a multi line string of the cInterfaces instance content
 * @return
 */
string cInterfaces::toString()
{
    string res = stringFormat("List of system ethernet interface addresses:\n  Id %30s %45s\n", "Name", "Address");
    for (stInterfaceInfo ifc : list)
    {
        res += ifc.toString() + "\n";
    }
    return res;
}

/** @} */
