// UTF8 (ü) //
/**
\file cUdpSocket.cpp

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2023-01-02

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL
@{

\class LibCpp::cUdpSocket
\brief Virtual class for data transmission using UDP ethernet frames

This object is part of the virtual hardware abstraction layer \ref G_LibCpp_HAL.
It inherits from LibCpp::cFramePort and specializes for UDP ethernet communication.
See there for general usage information.\n
\n
Objects inheriting from the cUdpSocket class are expected to request hardware recources.
The standard constructor must not request hardware recources (see LibCpp::cFramePort).
To do so call the 'open' method. In case asynchronous operation is configured in the constructor
any callback instances registrations must be done before calling 'open'.\n
Calling the constructor with just the first parameter 'open' set to true will immideately
open the UDP port setting the instance into operation.\n
In order to open the instance within the constructor even in asynchronous operation one
receiving callback method can be set within the constructor.\n
Implementations should follow these rules in order to achieve uniquely handling of communication
objects.\n
To get access to general ethernet related support methods a corresponding hardware dependent class
inheriting from LibCpp::cIp usally is required to be included to the user project.
*/

#include "cUdpSocket.h"

using namespace std;
using namespace LibCpp;

#pragma GCC diagnostic ignored "-Wunused-parameter"

/**
 *  @brief Constructor
 */
cUdpSocket::cUdpSocket()
{
    pCheckCallback = nullptr;
}

/**
 *  @brief Destructor
 */
cUdpSocket::~cUdpSocket() {}

/**
 * @brief Opens the Udp socket Instance to be operational
 * @param localPort
 * @param localInterfaceNameAddress
 * @param family
 * @param linkLocal
 * @param async
 * @return
 */
enIpResult cUdpSocket::open(int localPort, std::string localInterfaceNameAddress, enAddressFamily family, bool linkLocal, bool async) {return enIpResult_Failure;}


/**
 * @brief Prepares for closing by friendly closing the background task.
 * This way several objects requiring to finish a background task can do so in parallel.
 * Just calling 'close' requires much time as each 'close' needs to wait for the backgrond task actually
 * having finished.
 */
void cUdpSocket::prepareClose() {}

/**
 * @brief Frees the resources and finishes the background task
 * This method  waits until the background task is actually finished which may lead
 * to long execution times. Call LibCpp::cUdpSocket::prepareClose to avoid multiple
 * long execution times of 'close'.
 */
void cUdpSocket::close() {}

/**
 * @brief Sends a message to all network clients.
 * @param pMsg
 * @param messageLen
 * @param wait
 * @return
 */
int cUdpSocket::broadcast(const char* pMsg, int messageLen, enBlocking blockingMode) {return 0;}

/**
 * @brief Sets the destination address for 'send' calls.
 * @param destination
 * @return
 */
enIpResult  cUdpSocket::setDestination(stIpAddress* pDestination, int port) {return enIpResult_Success;}

/**
 * @brief Sets the destination address for 'send' calls.
 * @param destination
 * @return
 */
enIpResult  cUdpSocket::setDestination(stIpAddress destination, int port) {return setDestination(&destination);}

/**
 * @brief Reads the configured destionation ip address.
 * @return
 */
stIpAddress cUdpSocket::getIpAddressDestination() {stIpAddress addr; return addr;}

/**
 * @brief Deliveres the interface ip address the UDP socket is bound to.
 * @return
 */
stIpAddress cUdpSocket::getIpAddressLocal() {stIpAddress addr; return addr;}

/**
 * @brief ///< Deliveres the ip Address of the sender of the last received message
 * @return
 */
stIpAddress cUdpSocket::getIpAddressSource() {stIpAddress addr; return addr;}

/**
 * @brief Sends a message to all network clients.
 * @param pMsg
 * @param messageLen
 * @param wait
 * @return
 */
int cUdpSocket::broadcast(const uint8_t* pMsg, int messageLen, enBlocking blockingMode)
{
    return broadcast((const char*)pMsg, messageLen, blockingMode);
}

/**
 * @brief Sets a group address the host is listening on.
 * @param groupAddress
 */
void cUdpSocket::joinGroup(stIpAddress groupAddress) {}

/**
 * @brief Deliveres the port the object is listening on.
 * @return
 */
int cUdpSocket::getLocalPort()
{
    return 0;
}

/**
 * @brief Sends a message to a given destination
  Identical to 'sendTo', but using uint8_t as buffer.
 * @param message
 * @param messageLen
 * @param destination
 * @param blockingMode
 * @return
 */
int cUdpSocket::sendTo(const char* message, int messageLen, stIpAddress destination, enBlocking blockingMode)
{
    return send(message, messageLen, blockingMode);
}

/**
 * @brief Sends a message to a given destination
  Identical to 'sendTo', but using uint8_t as buffer.
 * @param message
 * @param messageLen
 * @param destination
 * @param blockingMode
 * @return
 */
int cUdpSocket::sendTo(const uint8_t* message, int messageLen, stIpAddress destination, enBlocking blockingMode)
{
    return sendTo((const char*)message, messageLen, destination, blockingMode);
}


/**
 * @brief Receives a message and its source address
 * Receives a message and if successful sets the parameter 'sourceAddress' to the internal memorized source Address.
 * The function is identical to LibCpp::cUdpPort::receive.
 * @param buffer
 * @param bufferSize
 * @param destination
 * @param blockingMode
 * @return
 */
int cUdpSocket::receiveFrom(char* buffer, int bufferSize, stIpAddress& source, enBlocking blockingMode)
{
    int len = receive(buffer, bufferSize, blockingMode);
    if (len)
        source = getIpAddressSource();
    return len;
}

/**
 * @brief Receives a message and its source address
 * Identical to 'receiveFrom', but using other buffer type instead.
 * @param buffer
 * @param bufferSize
 * @param destination
 * @param blockingMode
 * @return
 */
int cUdpSocket::receiveFrom(uint8_t* buffer, int bufferSize, stIpAddress& source, enBlocking blockingMode)
{
    return receiveFrom((char*)buffer, bufferSize, source, blockingMode);
}
#pragma GCC diagnostic warning "-Wunused-parameter"

/** @} */
