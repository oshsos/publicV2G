// UTF8 (ü) //
/**
\file   cLxCanPort.h

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2023-12-30
\brief  See .cpp

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL_Linux
@{

\class LibCpp::cLxCanPort

Einrichtung:

$ ip link show

$ sudo nano /etc/profile

sudo ip link set can0 type can bitrate 500000
sudo ip link set up can0
sudo ip link set can0 type can restart-ms 100

$ sudo visudo

# User priviledge specification
MYUSER	ALL=(root) NOPASSWD :/usr/bin/ip *

$ sudo apt install can-utils
$ cansend can0 123#1122
$ candump can0

**/

#ifndef h_cLxCanPort
#define h_cLxCanPort

#include <thread>
#include <atomic>

#include "../cCanPort.h"

//#include "can/cSocketCAN.h"

#ifdef PROJECTDEFS
  #include <ProjectDefs.h>
#endif

#ifndef LibCpp_cLxCanPort_RCVREPEAT
  #define LibCpp_cLxCanPort_RCVREPEAT 10  ///< Number off messages processed by a single polling call to 'operate' or within one task cycle
#endif

#include "../cDebug.h"

namespace LibCpp
{

/**
*   @brief Linux implementation of LibCpp::cCanPort
*/
class cLxCanPort : public cCanPort
{

  public:
    cLxCanPort(bool open = false, int canInterfaceId = 0, enCanBitRate bitRate = enCanBitRate_500k, bool async = false, iCanPort* pCallbackInstance = nullptr);        ///> Constructor
    virtual ~cLxCanPort();                                      ///< DestructorpCallbackInstance

    virtual void    set(bool open, int canInterfaceId, enCanBitRate bitRate, bool async = false);   ///> Sets the CAN parameters e.g. after standard object creation
    virtual bool	open();                                     ///< See LibCpp::cCanPort.
    virtual void    prepareClose();                         ///< Initiates closing background tasks
    virtual void 	close();                                    ///< See LibCpp::cCanPort.

    virtual bool	send(stCanMsg* pMsg, int wait=1);           ///< See LibCpp::cCanPort.
    virtual bool    receive(stCanMsg* pMsg, int wait=0);        ///< Ssee LibCpp::cCanPort.
    virtual void	operate();                                  ///< See LibCpp::cCanPort.
    virtual bool    setBitRate(enCanBitRate bitRate);           ///< LibCpp::cCanPort::setBitRate
    virtual bool    setFilter(uint32_t id, uint32_t mask);      ///< Prefiltering of messages being accepted for receiving operations.
    virtual enCanState getCanState();                           ///< LibCpp::cCanPort::getCanState
    virtual int     getCount();                                 ///< LibCpp::cCanPort::getCount


    virtual bool	setCallback(iCanPort* pInstance);           ///< See LibCpp::cCanPort.
    virtual bool	deleteCallback(iCanPort* pInstance);        ///< See LibCpp::cCanPort.
    virtual bool	setStateCallback(iCanState* pInstance);     ///< See LibCpp::cCanPort.
    virtual bool	deleteStateCallback(iCanState* pInstance);  ///< See LibCpp::cCanPort.

private:
    bool            hardwareReceive(bool blocking = false);     ///< Receive operation with hardware access.
    void            internalOperate();                          ///< Will be called either from operate or the background task
    void            threadReceiving();                          ///< Background thread for receiving.

public:
    cDebug dbg;                                  ///< Debug logger instance.

private:
    int                  canInterfaceId;         ///< Can port specification.
    enCanBitRate         bitRate;                ///< Can bus bitrate.

    stCanMsg             receiveMessage;         ///< Receive message buffer.
    std::atomic<bool>    receiveValid;           ///< Valid flag for receive message buffer.
    std::atomic<enCanState> state;               ///< Receive message buffer.
    std::atomic<bool>    stateChanged;           ///< Valid flag for receive message buffer.
    std::atomic<bool>    async;                  ///< Indication to call callbacks within the background receive task.
    std::atomic<bool>    threadEnabled;          ///< Indicates a running receive thread.
    std::thread*         pThread;                ///< Pointer to the thread object.

    int                  canSocket;              ///< Socket being used for CAN hardware access
    bool                 notOpenWarning;         ///< If 'send' or 'receive' is called without the port is opened, this flag indicates an Error message has already given.
    int                  receiveCount;           ///< Counts the number of incoming messages
};

}
#endif

/** @} */
