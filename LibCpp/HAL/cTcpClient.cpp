// UTF8 (ü)
/**
\file cTcpClient.cpp

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2023-01-11

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL
@{

\class LibCpp::cTcpClient
\brief Virtual class for data transmission using TCP protocol from the client side

This object is part of the virtual hardware abstraction layer \ref G_LibCpp_HAL.
It inherits from LibCpp::cFramePort and specializes for TCP client ethernet communication.
See there for general usage information.\n
\n
Objects inheriting from the cTcpClient class are expected to request hardware recources.
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

#include "cTcpClient.h"

using namespace LibCpp;
using namespace std;

#pragma GCC diagnostic ignored "-Wunused-parameter"

/**
 *  @brief Constructor
 */
cTcpClient::cTcpClient()
{
    pExternalSndRcv = nullptr;
}

/**
 *  @brief Destructor
 */
cTcpClient::~cTcpClient()
{
}

/**
 * @brief Opens the TCP client socket Instance to be operational
 * @param serverPort Port to connect to on the server side (local port will be arbitrary)
 * @param serverAddressName If given, the 'open' method will memorize the server and call 'connect' itself.
 * @param localInterfaceOrAddressName Name of the interface or an address of the interface the client will use
 * @param family Used in case the 'localInterfaceOrAddressName' specifies an interface.
 * @param linkLocal If set and family is IPv6, only link local addresses (fe80:...) are accepted. Otherwise any address is taken.
 * @param async If set, callback objects are called by the background task. Otherwise 'operate' needs to be called cyclically.
 * @return
 */
enIpResult cTcpClient::open(int serverPort, string serverAddressName, int localPort, std::string localInterfaceOrAddressName, bool async)
{
    return LibCpp::enIpResult_Success;
}

/**
 * @brief Prepares for closing by friendly closing the background task.
 * This way several objects requiring to finish a background task can do so in parallel.
 * Just calling 'close' requires much time as each 'close' needs to wait for the backgrond task actually
 * having finished.
 * @return Indicates, if the background process is already finished.
 */
bool cTcpClient::prepareClose() {return true;}

/**
 * @brief Frees the resources and finishes the background task
 * This method  waits until the background task is actually finished which may lead
 * to long execution times. Call LibCpp::cUdpSocket::prepareClose to avoid multiple
 * long execution times of 'close'.
 */
void cTcpClient::close() {}

/**
 * @brief Checks whether the client is connected to a server and no disconnection request process is in progress.
 * @return
 */
bool cTcpClient::isConnected()
{
    return false;
}

/**
 * @brief Checks for the client to be in active operation.
 * @return
 */
bool cTcpClient::isOpen()
{
    return false;
}

/**
 * @brief Checks whether the client is completely disconnected
 * Completely disconnected means, a possibly running disconnection process has finished.
 * @return
 */
bool cTcpClient::isClosed()
{
    return true;
}

/**
 * @brief Deliveres the interface ip address of the server the client is or will be connected to.
 * @return
 */
stIpAddress cTcpClient::getIpAddressServer() {stIpAddress addr; return addr;}

#pragma GCC diagnostic warning "-Wunused-parameter"

/**
 * @brief Returns the socket file descriptor
 * @return
 */
int cTcpClient::getSocketDescriptor()
{
    return 0;
}

/**
 * @brief Sets an external instance to execute send and receive operations.
 * @param pExternalSndRcv
 */
void cTcpClient::setExternalSndRcv(iExternalSndRcv* pExternalSndRcv)
{
    this->pExternalSndRcv = pExternalSndRcv;
}

/** @} */
