// UTF8 (ü) //
/**
\file   cCanPort.h

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2023-12-29
\brief  See cCanPort.cpp

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL
@{

\class LibCpp::cCanPort
**/

#ifndef h_cCanPort
#define h_cCanPort

#include <cstdint>
#include <string>
#include <vector>

namespace LibCpp
{

#pragma pack(push, 8)
/**
 * @brief Struct representing a CAN message
 * The struct is strictly aligned to be usable within streams.
 */
struct stCanMsg
{
    uint32_t id;            ///< Message identifyer
    uint8_t  len;           ///< Data payload length
    uint8_t  rtr;           ///< 1 for request transmitt, 0 otherwise
    union unData            ///< Data payload union
    {
        uint64_t ll;        ///< Data content as long long
        uint32_t l[2];      ///< Data content as long
        uint16_t s[4];      ///< Data content as short
        uint8_t  b[8];      ///< Data content as bytes
    }        data;          ///< Data payload element
    uint32_t timeStamp;     ///< TimeStamp of the CAN message
    uint8_t  ext;           ///< 1 for extended identifyers, 0 otherwise
    uint8_t  padding;       ///< Padding to 3*64bit

    stCanMsg(uint32_t id = 0, uint8_t len = 0, uint8_t b0 = 0, uint8_t b1 = 0, uint8_t b2 = 0, uint8_t b3 = 0, uint8_t b4 = 0, uint8_t b5 = 0, uint8_t b6 = 0, uint8_t b7 = 0, uint8_t rtr = 0, uint8_t ext = 0);  ///< Initializer for CAN message struct
    void set(uint32_t id = 0, uint8_t len = 0, uint8_t b0 = 0, uint8_t b1 = 0, uint8_t b2 = 0, uint8_t b3 = 0, uint8_t b4 = 0, uint8_t b5 = 0, uint8_t b6 = 0, uint8_t b7 = 0, uint8_t rtr = 0, uint8_t ext = 0);  ///< Initializer for CAN message struct
    std::string toString(); ///< Converts the CAN message to a string
};
#pragma pack(pop)

/** @brief Bit rates of a CAN interface */
enum enCanBitRate : uint16_t
{
  enCanBitRate_1000k = 1000,        ///< 1 MBit
  enCanBitRate_800k  = 800,			///< 800 kBit
  enCanBitRate_500k  = 500,			///< 500 kBit
  enCanBitRate_250k  = 250,			///< 250 kBit
  enCanBitRate_125k  = 125,			///< 125 kBit
  enCanBitRate_Auto  = 1,			///< Automatic bit rate detection
  enCanBitRate_Std   = 0            ///< Standard bit rate (500k)
} ;
std::string enCanBitRate_toString(enCanBitRate bitRate);    ///< Converts a CAN bit rate to a string

/** @brief Staus modes of a CAN interface */
enum enCanState
{
    enCanState_UNDEFINED,   ///< Mode on usual operation
    enCanState_ACTIVE,      ///< Mode on usual operation
    enCanState_WARNING,     ///< Low losses of data or accnowledges
    enCanState_PASSIVE,     ///< High losses of data
    enCanState_BUSOFF,      ///< Too many losses of data to continue bus operation
    enCanState_OVERRUN      ///< Message loss due to receive buffer overflow
};
std::string enCanState_toString(enCanState state);  ///< Converts a CAN status to a string

class cCanPort;             ///< uncommented

/**
 * @brief Interface class for objects requiring receive callbacks
 */
class iCanPort
{
public:
    iCanPort();                                                 ///< Constructor
    virtual ~iCanPort();                                        ///< Destructor
    virtual void onCanReceive(stCanMsg* Msg, cCanPort* pCanPort) = 0;          ///< Receive callback method. Pure virtual method forces the user to implement this method
};

/**
 * @brief Interface class for objects requiring CAN state callbacks
 */
class iCanState
{
public:
    iCanState();                                                ///< Constructor
    virtual ~iCanState();                                       ///< Destructor
    virtual void onCanState(enCanState state, cCanPort* pCanPort) = 0;              ///< CAN state change callback method. Pure virtual method forces the user to implement this method
};

/**
*   @brief Abstract interface for a CAN-Port
*/ 
class cCanPort
{
public:
    cCanPort(bool open = false);                                ///> LibCpp::cCanPort::cCanPort
    virtual ~cCanPort();                                        ///> LibCpp::cCanPort::~cCanPort

    virtual bool open();                                        ///> LibCpp::cCanPort::open
    virtual void close();                                       ///> LibCpp::cCanPort::close

    virtual bool send(stCanMsg* pMsg = nullptr, int wait=1);    ///> LibCpp::cCanPort::send (the int return type is used to be similar to stream ports)
    virtual bool receive(stCanMsg* pMsg = nullptr, int wait=0); ///> LibCpp::cCanPort::receive
    virtual void operate();                                     ///> LibCpp::cCanPort::operate

    virtual bool setBitRate(enCanBitRate bitRate);              ///> LibCpp::cCanPort::setBitRate
    virtual bool setFilter(uint32_t id, uint32_t mask);         ///< Prefiltering of messages being accepted for receiving operations.
    virtual enCanState getCanState();                           ///> LibCpp::cCanPort::getCanState
    virtual int  getCount();                                    ///> LibCpp::cCanPort::getCount

    virtual bool setCallback(iCanPort* pInstance);              ///> LibCpp::cCanPort::setCallback
    virtual bool deleteCallback (iCanPort* pInstance = 0);      ///> LibCpp::cCanPort::deleteCallback
    virtual bool setStateCallback (iCanState* pInstance);       ///> LibCpp::cCanPort::setStateCallback
    virtual bool deleteStateCallback (iCanState* pInstance);    ///> LibCpp::cCanPort::deleteStateCallback

protected:
    std::vector<iCanPort*>  callbacks;                           ///> List of registered callback instances
    std::vector<iCanState*> stateCallbacks;                      ///> Callback instances for CAN state changes
};

}
#endif

/** @} */
