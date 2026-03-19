// UTF8 (ü) //
/**
\file cWinCanPort.cpp

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2023-12-30

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL_Windows
@{

\class LibCpp::cWinCanPort

\brief Linux implementation of the virtual HAL class LibCpp::cCanPort

For information on usage see LibCpp::cCanPort.\n
\n
For documentation of the can socket see:\n
https://www.kernel.org/doc/html/latest/networking/can.html.\n
https://github.com/linux-can/can-utils/issues/451\n

*/

#ifdef PROJECTDEFS
    #include "ProjectDefs.h"
#endif

//#ifndef PLATFORM
//    #define PLATFORM X64    // options are X64, ARM
//#endif

#include "Pcan/PCANBasic.h"
#include "cWinCanPort.h"

using namespace LibCpp;
using namespace std;

//string canInterfaceId_toString(int canId)
//{
//    if (canId>7) canId=0;
//    if (canId<0) canId=0;
//    #if PLATFORM == ARM
//        return string("hcan") + to_string(canId);
//    #else
//        return string("can") + to_string(canId);
//    #endif
//}

/**
 * @brief cWinCanPort::cWinCanPort
 * See LibCpp::cCanPort::cCanPort for explanations.
 * @param open Set true to open the CAN port within the constructor (default: false)
 * @param canInterface Id of the CAN hardware to open (0 = hcan0, 1 = hcan1, ..)
 * @param bitRate Bitrate the CAN is operating on
 * @param async
 */
cWinCanPort::cWinCanPort(bool open, enCanBitRate bitRate, bool async, iCanPort* pCallbackInstance) :
    dbg("cWinCanPort")
{
    canSocket = 0;
    cWinCanPort::setCallback(pCallbackInstance);
    cWinCanPort::set(open, bitRate, async);
}

/**
 * @brief cWinCanPort::set
 * See LibCpp::cCanPort::cCanPort for explanations.
 * @param open
 * @param canInterface
 * @param bitRate
 * @param async
 */
void cWinCanPort::set(bool open, enCanBitRate bitRate, bool async)
{
    cDebug dbg("set", &this->dbg);

    if (canSocket)
        cWinCanPort::close();

    this->bitRate = bitRate;
    receiveValid = false;
    this->async = async;
    threadEnabled = false;
    pThread = nullptr;
    notOpenWarning = false;
    state = enCanState_UNDEFINED;
    stateChanged = false;

    const TPCANBaudrate Bitrate = PCAN_BAUD_500K;

    TPCANStatus stsResult = CAN_Initialize(PcanHandle, Bitrate);

    if (stsResult != PCAN_ERROR_OK)
    {
        dbg.printf(enDebugLevel_Fatal, "Can could not be initialized!");
        return;
    }

    canSocket = 1;

    if (open)
        cWinCanPort::open();
}

/**
 * @brief Opening
 * This method reserves the hardware recources and starts the background receiving task.
 * @return Successful opening
 */
bool cWinCanPort::open()
{
    cDebug dbg("open", &this->dbg);

    if (threadEnabled) return true;

    threadEnabled = true;     // To be fast enough in case a direct 'receive' follows after 'open'
    pThread = new std::thread(&cWinCanPort::threadReceiving, this);
    return true;
}

/**
 * @brief closePreparation Initiates the closing procedure of background tasks
 * As the close method in communikation objects might be a time consuming operation, due to friendly closing of
 * background tasks, sequential closing of several such objects leads to unacceptable long delay times. For that
 * reason this none blocking method initiates the closing procedure. By previously calling this method on several
 * objects using a background task, all tasks can close in parallel.
 */
void cWinCanPort::closePreparation()
{
    threadEnabled = false;
}

/**
 * @brief Closes hardware recources and tasks
 * The method keeps the configuration parameters alive such a following 'open' can resume operation.
 */
void cWinCanPort::close()
{
    threadEnabled = false;
    if (pThread)
    {
        pThread->join();
        delete pThread;
        pThread = nullptr;
    }
}

cWinCanPort::~cWinCanPort()
{
    cWinCanPort::close();
    CAN_Uninitialize(PCAN_NONEBUS);
    canSocket = 0;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
/**
 * @brief Sending a CAN message
 * @param pMsg Pointer to the message to be sent
 * @param wait Unimplemented
 * @return 1 if successful, 0 otherwise
 */
bool cWinCanPort::send(stCanMsg* pMsg, int wait)
{
    if (!canSocket)
    {
        if (!notOpenWarning)
        {
            cDebug dbg("send", &this->dbg);
            dbg.printf(enDebugLevel_Error, "Port not opened!");
        }
        return false;
    }
    if (!pMsg) return false;

    TPCANMsg canMsg;
    canMsg.MSGTYPE = PCAN_MESSAGE_STANDARD;

    if (pMsg->ext)
        canMsg.MSGTYPE |= PCAN_MESSAGE_EXTENDED;

    if (pMsg->rtr)
        canMsg.MSGTYPE |= PCAN_MESSAGE_RTR;

    canMsg.ID  = pMsg->id;
    canMsg.LEN = pMsg->len;
    for (int i=0; i<canMsg.LEN; i++)
        canMsg.DATA[i] = pMsg->data.b[i];

    return CAN_Write(PcanHandle, &canMsg) == PCAN_ERROR_OK;

}

/**
 * @brief Receives a message from the hardware to put in the object buffer
 * @param blocking unimplemented
 * @return message is present in the objects receive buffer
 */
bool cWinCanPort::hardwareReceive(bool blocking)
{
    if (!canSocket)
    {
        if (!notOpenWarning)
        {
            cDebug dbg("hardwareReceive", &this->dbg);
            dbg.printf(enDebugLevel_Error, "Port not opened!");
            notOpenWarning = true;
         }
         return false;
    }
    if (receiveValid) return true;

    TPCANMsg CANMsg;
    TPCANTimestamp CANTimeStamp;

    TPCANStatus stsResult = CAN_Read(PcanHandle, &CANMsg, &CANTimeStamp);

    if (stsResult != PCAN_ERROR_QRCVEMPTY)
    {
        receiveMessage.set();

        if (CANMsg.MSGTYPE & PCAN_MESSAGE_STATUS)
        {
//            enCanState state = enCanState_UNDEFINED;
//            if (frame.can_id & CAN_ERR_BUSOFF)
//                state = enCanState_BUSOFF;
//            else
//            {
//                switch (frame.data[1])
//                {
//                case CAN_ERR_CRTL_RX_OVERFLOW:
//                case CAN_ERR_CRTL_TX_OVERFLOW:
//                {
//                    state = enCanState_OVERRUN;
//                    break;
//                }
//                case CAN_ERR_CRTL_RX_WARNING:
//                case CAN_ERR_CRTL_TX_WARNING:
//                {
//                    state = enCanState_WARNING;
//                    break;
//                }
//                case CAN_ERR_CRTL_RX_PASSIVE:
//                case CAN_ERR_CRTL_TX_PASSIVE:
//                {
//                    state = enCanState_PASSIVE;
//                    break;
//                }
//                case CAN_ERR_CRTL_ACTIVE:
//                {
//                    state = enCanState_ACTIVE;
//                    break;
//                }
//                }
//                if (state != enCanState_UNDEFINED)
//                {
//                    this->state = state;
//                    stateChanged = true;
//                }
//                return false;
//            }
            return false;
        }

        if (CANMsg.MSGTYPE & PCAN_MESSAGE_ERRFRAME)
        {
            return false;
        }

        receiveMessage.id = CANMsg.ID;
        receiveMessage.len = CANMsg.LEN;
        for (int i=0; i<CANMsg.LEN; i++)
            receiveMessage.data.b[i] = CANMsg.DATA[i];

        if (CANMsg.MSGTYPE & PCAN_MESSAGE_EXTENDED)
            receiveMessage.ext = true;

        if (CANMsg.MSGTYPE & PCAN_MESSAGE_RTR)
            receiveMessage.rtr = true;

        receiveValid = true;
        return true;
    }
    return false;
}

/**
 * @brief Receiving a CAN message in an unblocking manner
 * @param pMsg Pointer to the message to be copied to.
 * @param wait Unimplemented
 * @return 'true' if a message has been received, 'false' if no message was present in the receive buffer
 */
bool cWinCanPort::receive(stCanMsg* pMsg, int wait)
{
    if (!pMsg) return receiveValid;

    if (!callbacks.size())
        hardwareReceive();

    if (receiveValid)
    {
        *pMsg = receiveMessage;

        if (!callbacks.size())
            receiveValid = false;

        return true;
    }
    return false;
}

/**
 * @brief Method to be called periodically to synchonously handle receive callbacks.
 */
void cWinCanPort::operate()
{
    if (!canSocket || async)
        return;
    int cnt = LibCpp_cWinCanPort_RCVREPEAT;
    bool received = false;
    do
    {
        received = hardwareReceive();
        if (received)
            internalOperate();
    }
    while (received && cnt--);
}

/**
 * @brief Calls registered callback instances
 */
void cWinCanPort::internalOperate()
{
    if (stateChanged && stateCallbacks.size())
    {
        for (iCanState* call : stateCallbacks)
            call->onCanState(state, this);
        stateChanged = false;
    }
    if (receiveValid && callbacks.size())
    {
        for (iCanPort* call : callbacks)
            call->onCanReceive(&receiveMessage, (cCanPort*)this);
        stateChanged = false;
    }
}

/**
 * @brief Sets the filter to receive selected messages only.
 * A message will be reiceived in case: receivedId & mask == id & mask.
 * @param id
 * @param mask
 * @return
 */
bool cWinCanPort::setFilter(uint32_t id, uint32_t mask)
{
    cDebug dbg("setFilter", &this->dbg);

//    struct can_filter rfilter;
//    rfilter.can_id = id;
//    rfilter.can_mask = mask;
//    if (setsockopt(m_canSocket, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter)))
//    {
//        dbg.printf(enDebugLevel_Error, "Setting filter id=0x%Xi mask=0x%Xi faild", (int)id, (int)mask);
//        return false;
//    }
    return false;
}
#pragma GCC diagnostic warning "-Wunused-parameter"

/**
 * @brief Sets the bit rate of the CAN bus
 * This method closes an reopens the CAN hardware and migth be time consuming in asychonous mode.
 * @param bitRate
 * @return
 */
bool cWinCanPort::setBitRate(enCanBitRate bitRate)
{
    set((canSocket!=0), bitRate, async);
    return true;
}

/**
 * @copydoc LibCpp::cCanPort::getCanState
 */
enCanState cWinCanPort::getCanState()
{
    return LibCpp::enCanState_UNDEFINED;
}

/**
 * @copydoc LibCpp::cCanPort::getCount
 * @return
 */
int cWinCanPort::getCount()
{
    return 0;
}

bool cWinCanPort::setCallback(iCanPort* pInstance)
{
    cDebug dbg("setCallback", &this->dbg);
    if (async && canSocket)
        dbg.printf(enDebugLevel_Error, "Callback instances must not be registered, if the port uses the task and is open! Racing conditions might occur.");
    // return cCanPort::setCallback(pInstance);
    return ((cCanPort*)this)->cCanPort::setCallback(pInstance);
}

bool cWinCanPort::deleteCallback(iCanPort* pInstance)
{
    cDebug dbg("deleteCallback", &this->dbg);
    if (async && canSocket)
        dbg.printf(enDebugLevel_Error, "Callback instances must not be deleted, if the port uses the task and is open! Racing conditions might occur.");
    // return cCanPort::deleteCallback(pInstance);
    return ((cCanPort*)this)->cCanPort::deleteCallback(pInstance);
}

/**
 * @brief Registeres a callback instance to be notyfied in case of CAN bus state changes
 * Behavior is like LibCpp::cCanPort::setCallback.
 * @param pInstance
 * @return
 */
bool cWinCanPort::setStateCallback(iCanState* pInstance)
{
    cDebug dbg("setStateCallback", &this->dbg);
    if (async && canSocket)
        dbg.printf(enDebugLevel_Error, "Callback instances must not be deleted, if the port uses the task and is open! Racing conditions might occur.");
    // return cCanPort::setStateCallback(pInstance);
    return ((cCanPort*)this)->cCanPort::setStateCallback(pInstance);
}

/**
 * @brief Registeres a callback instance for receiving messages
 * Behavior is like LibCpp::cCanPort::deleteCallback.
 * @param pInstance
 * @return Confirms, the instance is not part of the list (either not any more or not has been)
 */
bool cWinCanPort::deleteStateCallback(iCanState* pInstance)
{
    cDebug dbg("deleteStateCallback", &this->dbg);
    if (async && canSocket)
        dbg.printf(enDebugLevel_Error, "Callback instances must not be deleted, if the port uses the task and is open! Racing conditions might occur.");
    // return cCanPort::deleteStateCallback(pInstance);
    return ((cCanPort*)this)->cCanPort::deleteStateCallback(pInstance);
}

/**
 * @brief Background task for asynchronous receiving
 */
void cWinCanPort::threadReceiving()
{
    threadEnabled = true;

    while(threadEnabled)
    {
        if (hardwareReceive(true))
            internalOperate();
    }
    return;
}
