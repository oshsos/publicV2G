// UTF8 (ü) //
/**
\file cPacketSocket.cpp

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2023-04-22

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL
@{

\class LibCpp::cPacketSocket
\brief Virtual class for data transmission using layer 2 mac-address ethernet frames

This object is part of the virtual hardware abstraction layer \ref G_LibCpp_HAL.
It inherits from LibCpp::cFramePort and specializes for layer 2 mac-address ethernet communication.
See there for general usage information.\n
\n
Objects inheriting from the cPacketSocket class are expected to request hardware recources.
The standard constructor must not request hardware recources (see LibCpp::cFramePort).
To do so call the 'open' method. In case asynchronous operation is configured in the constructor
any callback instances registrations must be done before calling 'open'.\n
Calling the constructor with just the first parameter 'open' set to true will immideately
open the ethernet port setting the instance into operation.\n
In order to open the instance within the constructor even in asynchronous operation one
receiving callback method can be set within the constructor.\n
Implementations should follow these rules in order to achieve uniquely handling of communication
objects.\n
To get access to general ethernet related support methods a corresponding hardware dependent class
inheriting from LibCpp::cIp usally is required to be included to the user project.
*/

#include "cPacketSocket.h"
#include "Tools.h"

using namespace std;
using namespace LibCpp;

uint8_t LibCpp::MAC_BROADCAST[ETHER_ADDR_LEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t LibCpp::MAC_EMPTY[ETHER_ADDR_LEN] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#pragma GCC diagnostic ignored "-Wunused-parameter"

/**
 *  @brief Constructor
 */
cPacketSocket::cPacketSocket()
{
    pCheckCallback = nullptr;
}

/**
 *  @brief Destructor
 */
cPacketSocket::~cPacketSocket() {}

/**
 * @brief Opens the Udp socket Instance to be operational
 * @param localPort
 * @param localInterfaceNameAddress
 * @param family
 * @param linkLocal
 * @param async
 * @return
 */
enIpResult cPacketSocket::open(std::string localInterfaceNameAddress, bool async) {return enIpResult_Failure;}


/**
 * @brief Prepares for closing by friendly closing the background task.
 * This way several objects requiring to finish a background task can do so in parallel.
 * Just calling 'close' requires much time as each 'close' needs to wait for the backgrond task actually
 * having finished.
 */
void cPacketSocket::prepareClose() {}

/**
 * @brief Frees the resources and finishes the background task
 * This method  waits until the background task is actually finished which may lead
 * to long execution times. Call LibCpp::cPacketSocket::prepareClose to avoid multiple
 * long execution times of 'close'.
 */
void cPacketSocket::close() {}

/**
 * @brief Retrieves the local host mac address the cPacketSocket instance is sending and receiving on.
 * @return
 */
unsigned char* cPacketSocket::getMacAddressLocal()
{
    return 0;
}

/**
 * @brief Sets the socket messaage filter (e.g. receives just messages to own ethernet (mac) address)
 */
void cPacketSocket::setFilter()
{
}

/**
 * @brief Converts a MAC address to a readable string
 * @param macAddress
 * @return
 */
string cPacketSocket::mac_toString(char* macAddress)
{
    string out;
    for(int i=0; i<6; i++)
    {
        out += stringFormat("%02x", (unsigned char)macAddress[i]);
        if (i<5) out+=":";
    }
    return out;
}

/** @} */
