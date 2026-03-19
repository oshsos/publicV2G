// UTF8 (ü)
/**
\file   cWinPacketSocket.h

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2024-04-22
\brief  See cWinPacketSocket.cpp

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL_Windows
@{

\class LibCpp::cWinPacketSocket

The implementation is based on the WinPCap library. This library needs to be installed
to use this object.\n
https://www.winpcap.org/default.htm\n
It is required to set the header include path:\n
INCLUDEPATH += ../WpdPack/Include\n
Otherwise subsequend includes would fail.

**/

#ifndef cWinPacketSocket_H
#define cWinPacketSocket_H

//#undef UNICODE
//#define WIN32_LEAN_AND_MEAN

#include "cWinIp.h"

#include <thread>
//#include <atomic>
#ifdef __MINGW32__
#include "atomic_patch/atomic.h"     // There is an ambigious conversion in <atomic> in line 367. This is corrected in the local library file.
#else
#include <atomic>
#endif

#include "../cPacketSocket.h"
#include "../cDebug.h"

#ifndef WIN32
#define WIN32               // Required for pcap.h when using VisualStudio compiler
#endif

#include "pcap/pcap.h"      // The following two include paths are required to be defined for the compiler! ../LibCpp/HAL/Windows/WpdPack/Include ../LibCpp/HAL/Windows/WpdPack/Include/pcap

namespace LibCpp
{

void printPcapInterfaces();

/**
 * @brief The windows implementation of the LibCpp::cUdpSocket hardware abstraction class.
 */
class cWinPacketSocket : public cPacketSocket
{
public:
    cWinPacketSocket(bool open = false, std::string localInterfaceName = "", bool async = false, iFramePort* pCallback = nullptr);   ///< Constructor.
    ~cWinPacketSocket();                            ///< Destructor.

    enIpResult      open(std::string localInterfaceName = "", bool async = false);          ///< Opens the instance.
    void            prepareClose();                 ///< Prepares closing by ordering the background task to finish.
    void            close();                        ///< Closes the instance, frees resources and finishes background the task.

    int             send(const char* message, int messageLen, enBlocking blockingMode = enBlocking_BUFFER);                             ///> Sends a data frame.
    int             receive(char* buffer, int bufferSize, enBlocking blockingMode = enBlocking_NONE);                                   ///> Receives a data frame.
    int             receiveBuffer(char** pBuffer, enBlocking blockingMode = enBlocking_NONE);                                           ///> Receives a data frame without copying
    void            receiveAcknowledge();           ///> Acknowledges a reception of a message by 'receiveBuffer'
    void            operate();                      ///> Cyclic operation on callback polling mode usage.

    unsigned char*  getMacAddressLocal();           ///< Retrieves the local host mac address the cPacketSocket instance is sending and receiving on.
    void            setFilter();                    ///< Sets the socket messaage filter (e.g. receives just messages to own ethernet (mac) address)

private:
    int             sendToInternal(const char* message, int messageLen, unSockAddr destination, enBlocking blockingMode = enBlocking_BUFFER);   ///< Helping method used by 'send' and 'sendTo'.
    void            internalOperate();              ///< Calls registered callback objects on message reception
    void            threadReceiving();              ///< Background thread.

public:
    cDebug dbg;                                     ///< Logger instance

private:

    std::string         localInterfaceName;         ///< The windows interface name
    std::string         ifName;                     ///< The pcap interface name
    pcap_t*             pLocalSocket;               ///< Local winpcap socket the cPacketSocket class instance is using.
    unsigned char       localMacAddress[ETHER_ADDR_LEN];        ///< Local host interface ehternet (mac) address.
    char                message[LibCpp_cIp_PACKET_BUFFERSIZE * 10];  ///< Message buffer of received messages.
    std::atomic<int>    messageLen;                 ///< Length of the message stored in the message buffer (controls access between background process and main process.
    std::atomic<bool>   threadEnabled;              ///< Flag indicating a started background process and to be used to friendly close the process.
    std::atomic<bool>   threadRunning;              ///< Flag indicating an actually running background process.
    std::thread*        pThread;                    ///< Class instance representing the background task.
    bool                async;                      ///< Asynchronous call of callback objects

public:
    char                buffer[LibCpp_cIp_PACKET_BUFFERSIZE];  ///< Message buffer of received messages.
    int                 bufferLen;
};

}

#endif

/** @} */
