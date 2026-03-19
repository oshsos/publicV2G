// UTF8 (ü) //
/**
\file cLxCanPort.cpp

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2023-12-30

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL
@{

\class LibCpp::cLxCanPort

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

#if !(defined(PLATFORM_X64) || defined(PLATFORM_ARM))
    #define PLATFORM_X64
#endif

//#ifndef PF_CAN
//#define PF_CAN 29
//#endif

//#ifndef AF_CAN
//#define AF_CAN PF_CAN
//#endif

#include <cstring>
#include <stdio.h>
//#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/error.h>
#include <linux/can/raw.h>
#include <linux/sockios.h>

#include "cLxCanPort.h"

using namespace LibCpp;
using namespace std;

string canInterfaceId_toString(int canId)
{
    if (canId>7) canId=0;
    if (canId<0) canId=0;
    #ifdef PLATFORM_ARM
        return string("hcan") + to_string(canId);
    #else
        return string("can") + to_string(canId);
    #endif
}

/**
 * @brief cLxCanPort::cLxCanPort
 * See LibCpp::cCanPort::cCanPort for explanations.
 * @param open Set true to open the CAN port within the constructor (default: false)
 * @param canInterface Id of the CAN hardware to open (0 = hcan0, 1 = hcan1, ..)
 * @param bitRate Bitrate the CAN is operating on
 * @param async
 */
cLxCanPort::cLxCanPort(bool open, int canInterfaceId, enCanBitRate bitRate, bool async, iCanPort* pCallbackInstance) :
    dbg("cLxCanPort")
{
    cLxCanPort::setCallback(pCallbackInstance);
    cLxCanPort::set(open, canInterfaceId, bitRate, async);
}

/**
 * @brief cLxCanPort::set
 * See LibCpp::cCanPort::cCanPort for explanations.
 * @param open
 * @param canInterface
 * @param bitRate
 * @param async
 */
void cLxCanPort::set(bool open, int canInterfaceId, enCanBitRate bitRate, bool async)
{
    if (canInterfaceId < 0 || canInterfaceId >7) canInterfaceId = 0;

    if (canSocket)
        cLxCanPort::close();

    canSocket = 0;
    this->canInterfaceId = canInterfaceId;
    this->bitRate = bitRate;
    receiveValid = false;
    this->async = async;
    threadEnabled = false;
    pThread = nullptr;
    notOpenWarning = false;
    receiveCount = 0;
    state = enCanState_UNDEFINED;
    stateChanged = false;

    if (open)
        cLxCanPort::open();
}

/**
 * @brief Opening
 * This method reserves the hardware recources and starts the background receiving task.
 * @return Successful opening
 */
bool cLxCanPort::open()
{
    cDebug dbg("open", &this->dbg);

    if (threadEnabled) return true;
    cLxCanPort::close();

    struct sockaddr_can canSockAddr;
    struct ifreq m_ifr;

    memset(&canSockAddr, 0, sizeof(struct sockaddr_can));
    memset(&m_ifr, 0, sizeof(struct ifreq));

    if ((canSocket = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
    {
        dbg.printf(enDebugLevel_Fatal, "Socket opening failed with: %s!", strerror(errno));
        canSocket = 0;
        return false;
    }

    string ifName = canInterfaceId_toString(canInterfaceId);
    strncpy(m_ifr.ifr_name, ifName.c_str(), IFNAMSIZ);
    if (ioctl(canSocket, SIOCGIFINDEX, &m_ifr) < 0)
    {
        dbg.printf(enDebugLevel_Fatal, "Setting requested interface id=%i with name %s failed with: %s!", canInterfaceId, ifName.c_str(), strerror(errno));
        cLxCanPort::close();
        return false;
    }

    const int timestamp_on = 1;
    if (setsockopt(canSocket, SOL_SOCKET, SO_TIMESTAMP, &timestamp_on, sizeof(timestamp_on)))
        dbg.printf(enDebugLevel_Error, "Setting socket option 'timestamp' failed with: %s!", strerror(errno));

    can_err_mask_t err_mask = CAN_ERR_MASK; // ( CAN_ERR_TX_TIMEOUT | CAN_ERR_BUSOFF ...);
    if (setsockopt(canSocket, SOL_CAN_RAW, CAN_RAW_ERR_FILTER, &err_mask, sizeof(err_mask)))
        dbg.printf(enDebugLevel_Error, "Enabling state information failed with: %s!", strerror(errno));

    //  loopback is on in standard configuration. Even if loopback is enabled the sending
    //  application will not receive its own sent messages. Only other applications accessing the
    //  same hardware will or will not receive sent messages on the local system.
    //
    // int loopback = 0;
    // if (setsockopt(canSocket, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &loopback, sizeof(loopback)))
    // {
    //     dbg.printf(enDebugLevel_Error, "Setting socket option no loopback failed with: %s!", strerror(errno));
    // }

    string command("sudo ip link set ");
    command += canInterfaceId_toString(canInterfaceId);
    command += " type can restart-ms 100";
    int res = system(command.c_str());
    if ( WEXITSTATUS(res) != 0 )
        dbg.printf(enDebugLevel_Error, "Enabling CAN auto restart failed!");

    // enhances safety when using select
    if (fcntl(canSocket, F_SETFL, O_NONBLOCK) < 0)
        dbg.printf(enDebugLevel_Error,"fcntl O_NONBLOCK failed!");

    canSockAddr.can_family = AF_CAN;
    canSockAddr.can_ifindex = m_ifr.ifr_ifindex;
    if (bind(canSocket, (struct sockaddr *)&canSockAddr, sizeof(struct sockaddr)))
    {
        dbg.printf(enDebugLevel_Error, "Binding failed with: %s!", strerror(errno));
        cLxCanPort::close();
        return false;
    }

    dbg.printf(enDebugLevel_Info, "Opened Can %s\n", canInterfaceId_toString(canInterfaceId).c_str());

    threadEnabled = true;     // To be fast enough in case a direct 'receive' follows after 'open'
    pThread = new std::thread(&cLxCanPort::threadReceiving, this);
    return true;
}

/**
 * @brief closePreparation Initiates the closing procedure of background tasks
 * As the close method in communikation objects might be a time consuming operation, due to friendly closing of
 * background tasks, sequential closing of several such objects leads to unacceptable long delay times. For that
 * reason this none blocking method initiates the closing procedure. By previously calling this method on several
 * objects using a background task, all tasks can close in parallel.
 */
void cLxCanPort::prepareClose()
{
    threadEnabled = false;
}

/**
 * @brief Closes hardware recources and tasks
 * The method keeps the configuration parameters alive such a following 'open' can resume operation.
 */
void cLxCanPort::close()
{
    threadEnabled = false;
    if (pThread)
    {
        pThread->join();
        pThread = nullptr;
    }

    if (canSocket)
    {
        ::close(canSocket);
        canSocket = 0;
    }
}

cLxCanPort::~cLxCanPort()
{
    cLxCanPort::close();
}

#pragma GCC diagnostic ignored "-Wunused-parameter"

/**
 * @brief Sending a CAN message
 * @param pMsg Pointer to the message to be sent
 * @param wait Unimplemented
 * @return 1 if successful, 0 otherwise
 */
bool cLxCanPort::send(stCanMsg* pMsg, int wait)
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

  struct can_frame cf;
  memset(&cf, 0, sizeof(struct can_frame));

  if (pMsg->ext == 1)
      cf.can_id |= CAN_EFF_FLAG;
  else
      cf.can_id = pMsg->id;
  if (pMsg->rtr)
      cf.can_id |= CAN_RTR_FLAG;

  cf.can_dlc = pMsg->len;
  //cf.can_len = pMsg->len;
  memcpy(cf.data, pMsg->data.b, 8);

  if (write(canSocket, &cf, sizeof(cf)) != sizeof(cf))
  {
     return false;
  }
  return true;
}

/**
 * @brief Receives a message from the hardware to put in the object buffer
 * @param blocking If 'true' blocks for 1 second
 * @return message is present in the objects receive buffer
 */
bool cLxCanPort::hardwareReceive(bool blocking)
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

    fd_set set;
    struct timeval tv = {0, 0};
    struct timeval tv_block = {1, 0};

    FD_ZERO(&set);
    FD_SET(canSocket, &set);

    if (blocking)
        tv = tv_block;

    int rval = select(canSocket+1, &set, NULL, NULL, &tv);     ///< First argument specifies the search range.
    if (rval <= 0)
    {
        if (rval == 0 || errno == EAGAIN)
        {
          return false;
        }
        cDebug dbg("hardwareReceive", &this->dbg);
        dbg.printf(enDebugLevel_Error, "Select method failed with %s!", strerror(errno));
        notOpenWarning = true;
        return false;
    }

    struct can_frame frame;
    //struct timeval received = {0, 0};
    memset(&frame, 0, sizeof(struct can_frame));

    int res = read(canSocket, &frame, sizeof(struct can_frame));
    if (res <= 0)
    {
        if (res == 0 || errno == EAGAIN)
        {
        }
        else
        {
            cDebug dbg("hardwareReceive", &this->dbg);
            dbg.printf(enDebugLevel_Error, "Select method failed with %s!", strerror(errno));
            notOpenWarning = true;
        }
        return false;
    }

    struct timeval tv_recv;
    ioctl(canSocket, SIOCGSTAMP, &tv_recv);
    // gettimeofday(&received, nullptr);

    if (frame.can_id & CAN_ERR_FLAG)
    {
        enCanState state = enCanState_UNDEFINED;
        if (frame.can_id & CAN_ERR_BUSOFF)
            state = enCanState_BUSOFF;
        else
        {
            switch (frame.data[1])
            {
            case CAN_ERR_CRTL_RX_OVERFLOW:
            case CAN_ERR_CRTL_TX_OVERFLOW:
            {
                state = enCanState_OVERRUN;
                break;
            }
            case CAN_ERR_CRTL_RX_WARNING:
            case CAN_ERR_CRTL_TX_WARNING:
            {
                state = enCanState_WARNING;
                break;
            }
            case CAN_ERR_CRTL_RX_PASSIVE:
            case CAN_ERR_CRTL_TX_PASSIVE:
            {
                state = enCanState_PASSIVE;
                break;
            }
            case CAN_ERR_CRTL_ACTIVE:
            {
                state = enCanState_ACTIVE;
                break;
            }
            }
        }
        if (state != enCanState_UNDEFINED)
        {
            this->state = state;
            stateChanged = true;
        }
        return false;
    }

    receiveMessage.set();
    if (frame.can_id | CAN_EFF_FLAG)  // 29bit 11bit
    {
        receiveMessage.id = frame.can_id & 0x1FFFFFFF;
        receiveMessage.ext = 1;
    }
    else
        receiveMessage.id = frame.can_id & 0x7FF;
    receiveMessage.len = frame.len;
    if (frame.can_id & CAN_RTR_FLAG)
        receiveMessage.rtr = 1;
    else
        memcpy(&receiveMessage.data, &frame.data, 8);
    receiveMessage.timeStamp = (tv_recv.tv_sec * 10000) + tv_recv.tv_usec * 100;
    receiveCount++;
    receiveValid = 1;
    return true;
}

/**
 * @brief Receiving a CAN message in an unblocking manner
 * @param pMsg Pointer to the message to be copied to.
 * @param wait Unimplemented
 * @return 'true' if a message has been received, 'false' if no message was present in the receive buffer
 */
bool cLxCanPort::receive(stCanMsg* pMsg, int wait)
{
    if (!pMsg) return receiveValid;

    if (!receiveValid) return false;

    if (!callbacks.size())
        hardwareReceive();

    *pMsg = receiveMessage;

    if (!callbacks.size())
        receiveValid = false;

    return true;
}

#pragma GCC diagnostic warning "-Wunused-parameter"

/**
 * @brief Method to be called periodically to synchonously handle receive callbacks.
 */
void cLxCanPort::operate()
{
    if (!canSocket || async)
        return;
    int cnt = LibCpp_cLxCanPort_RCVREPEAT;
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
void cLxCanPort::internalOperate()
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
bool cLxCanPort::setFilter(uint32_t id, uint32_t mask)
{
    cDebug dbg("setFilter", &this->dbg);

    struct can_filter rfilter;
    rfilter.can_id = id;
    rfilter.can_mask = mask;
    if (setsockopt(canSocket, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter)))
    {
        dbg.printf(enDebugLevel_Error, "Setting filter id=0x%Xi mask=0x%Xi faild", (int)id, (int)mask);
        return false;
    }
    return true;
}

/**
 * @brief Sets the bit rate of the CAN bus
 * This method closes an reopens the CAN hardware and migth be time consuming in asychonous mode.
 * @param bitRate
 * @return
 */
bool cLxCanPort::setBitRate(enCanBitRate bitRate)
{
    set((canSocket!=0), canInterfaceId, bitRate, async);
    return true;
}

bool cLxCanPort::setCallback(iCanPort* pInstance)
{
    cDebug dbg("setCallback", &this->dbg);
    if (async && canSocket)
        dbg.printf(enDebugLevel_Error, "Callback instances must not be registered, if the port uses the task and is open! Racing conditions might occur.");
    // return cCanPort::setCallback(pInstance);
    return ((cCanPort*)this)->cCanPort::setCallback(pInstance);
}

enCanState cLxCanPort::getCanState()
{
    return state;
}

int cLxCanPort::getCount()
{
    return receiveCount;
}

bool cLxCanPort::deleteCallback(iCanPort* pInstance)
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
bool cLxCanPort::setStateCallback(iCanState* pInstance)
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
bool cLxCanPort::deleteStateCallback(iCanState* pInstance)
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
void cLxCanPort::threadReceiving()
{
    threadEnabled = true;

    while(threadEnabled)
    {
        if (hardwareReceive(true))
            internalOperate();
    }
    return;
}
