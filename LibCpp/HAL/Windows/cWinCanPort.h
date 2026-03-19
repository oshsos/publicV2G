// UTF8 (ü) //
/**
\file   cWinCanPort.h

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2023-12-30
\brief  See .cpp

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL_Windows
@{

\class LibCpp::cWinCanPort
**/

#ifndef h_cWinCanPort
#define h_cWinCanPort

#include <thread>
#include <atomic>

#include "../cCanPort.h"
#include "Pcan/PCANBasic.h"

#ifdef PROJECTDEFS
  #include <ProjectDefs.h>
#endif

#ifndef LibCpp_cWinCanPort_RCVREPEAT
  #define LibCpp_cWinCanPort_RCVREPEAT 10  ///< Number off messages processed by a single polling call to 'operate' or within one task cycle
#endif

#include "../cDebug.h"

namespace LibCpp
{

/**
*   @brief Linux implementation of LibCpp::cCanPort
*/
class cWinCanPort : public cCanPort
{

  public:
    cWinCanPort(bool open = false, enCanBitRate bitRate = enCanBitRate_500k, bool async = false, iCanPort* pCallbackInstance = nullptr);        ///> Constructor
    virtual ~cWinCanPort();                                      ///< DestructorpCallbackInstance

    virtual void    set(bool open, enCanBitRate bitRate, bool async = false);   ///> Sets the CAN parameters e.g. after standard object creation
    virtual bool	open();                                     ///< See LibCpp::cCanPort.
    virtual void    closePreparation();                         ///< Initiates closing background tasks
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
    const TPCANHandle PcanHandle = PCAN_USBBUS1; ///< Specifies the CAN hardware

    int                  canInterfaceId;         ///< Can port specification.
    enCanBitRate         bitRate;                ///< Can bus bitrate.

    stCanMsg             receiveMessage;         ///< Receive message buffer.
    std::atomic<bool>    receiveValid;           ///< Valid flag for receive message buffer.
    std::atomic<enCanState> state;               ///< Receive message buffer.
    std::atomic<bool>    stateChanged;           ///< Valid flag for receive message buffer.
    std::atomic<bool>    async;                  ///< Indication to call callbacks within the background receive task.
    std::atomic<bool>    threadEnabled;          ///< Indicates a running receive thread.
    std::thread*         pThread;                ///< Pointer to the thread object.

    int                  canSocket;              ///< Indicates the CAN hardware to be initialized
    bool                 notOpenWarning;         ///< If 'send' or 'receive' is called without the port is opened, this flag indicates an Error message has already given.
};

}
#endif

/** @} */
