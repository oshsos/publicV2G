// UTF8 (ü)
/**
\file   cLxPacketSocket.h

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2024-05-16
\brief  See cLxPacketSocket.cpp

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL_Linux
@{

\class LibCpp::cLxPacketSocket

**/

#ifndef cLxPacketSocket_H
#define cLxPacketSocket_H

#include <thread>
#include <atomic>

#include "../cPacketSocket.h"
#include "../cDebug.h"

namespace LibCpp
{

/**
 * @brief The windows implementation of the LibCpp::cUdpSocket hardware abstraction class.
 */
class cLxPacketSocket : public cPacketSocket
{
public:
    cLxPacketSocket(bool open = false, std::string localInterfaceNameAddress = "", bool async = false, iFramePort* pCallback = nullptr);   ///< Constructor.
    ~cLxPacketSocket();                            ///< Destructor.

    enIpResult      open(std::string localInterfaceNameAddress = "", bool async = false);          ///< Opens the instance.
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
    void            internalOperate();              ///< Calls registered callback objects on message reception
    void            threadReceiving();              ///< Background thread.

public:
    cDebug dbg;                                     ///< Logger instance

private:

    int                 localSocket;               ///< Local winpcap socket the cPacketSocket class instance is using.
    int                 ifIndex;                    ///< Interface index, the socket is bound to.
    unsigned char       localMacAddress[ETHER_ADDR_LEN];        ///< Local host interface ehternet (mac) address.
    char                message[LibCpp_cIp_PACKET_BUFFERSIZE * 10];  ///< Message buffer of received messages.
    std::atomic<int>    messageLen;                 ///< Length of the message stored in the message buffer (controls access between background process and main process.
    std::atomic<bool>   threadEnabled;              ///< Flag indicatin a started background process and to be used to friendly close the process.
    std::thread*        pThread;                    ///< Class instance representing the background task.
    bool                async;                      ///< Asynchronous call of callback objects
};

}

#endif

/** @} */
