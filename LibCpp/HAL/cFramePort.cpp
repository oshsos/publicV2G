// UTF8 (ü) //
/**
\file cFramePort.cpp

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2023-01-01

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL
@{

*/

#include "cFramePort.h"

using namespace std;
using namespace LibCpp;

#pragma GCC diagnostic ignored "-Wunused-parameter"

/**
 * @brief Constructor for the frame port interface object required for receiving messages by the callback mechanism.
 * Implementations of this virtual object usually do not override constructor or destructor.\n
 * Instead Implementations require the Implementation of the method LibCpp::iFramePort::onFrameReceive.\n
 * Application specific objects inherit from this virtual interface class if they require callback notifications.
 * Mostly they inherit from an applicaton specific base object and secondly from iCanPort.
 */
iFramePort::iFramePort() {}

/** @brief Destructor for the frame port interface object. */
iFramePort::~iFramePort()
{
}

/**
 * @brief Constructor for a frame controlling object to evaluate stream or to encode frames.
 * Implementations of this virtual object usually do not override constructor or destructor.\n
 * Instead Implementations require the Implementation of the methods
 * LibCpp::iFramePort::onReceiveFrameCheck and LibCpp::iFramePort::onSendFrameCheck.\n
 * Application specific objects inherit from this virtual interface class to control
 * the identification of incoming data streams. In case a finished byte package is found
 * the method returns the len of the package or zero otherwise. Typically but not necessarily
 * the method is called on each incoming byte for such a purpose.\n
 * Another use case is the encoding of frames to be sent and decoding of received frames.
 * The parameter 'completedFrame' may be used in case the frame port object detects frames
 * itself and the checking object is just used for coding and encoding.
 */
iFrameCheck::iFrameCheck()
{
}

/** @brief Destructor for the frame port interface object. */
iFrameCheck::~iFrameCheck()
{
}

/**
 * @brief External method to check input stream data for completed frames and to manipulate received data
 * @param pFramePort
 * @param receiveBuffer
 * @param pLen
 * @param bufferSize
 * @return
 */
bool iFrameCheck::onReceiveFrameCheck(cFramePort* pFramePort, char* receiveBuffer, unsigned int* pLen, unsigned int bufferSize)
{
    return true;
}

/**
 * @brief External method to check input stream data for completed frames and to manipulate received data
 * @param pFramePort
 * @param sendBuffer
 * @param len
 * @param ppNewSendBuffer
 * @return
 */
int iFrameCheck::onSendFrameCheck(cFramePort* pFramePort, char* sendBuffer, unsigned int len, char** ppNewSendBuffer)
{
    return len;
}

/**
 * @brief Constructor of the virtual byte frame interface
 * In case a communication object using hardware recources inherit from cFramePort, it is intended
 * not to requst those recources with the parameterless standard constructor. Otherwise creating arrays
 * of interface objects would not be possible. On the other hand the creation of operational objects
 * should be as easy as possible. For that reason it is recommended to use the first parameter
 * to be a boolean indicating to open (requesting recources) the object instance within the constructor
 * and set 'false' as the standard value.
 * The same is required for objects with asynchronous calls of callback methods unless those calls are
 * already provided within the constructor. This is because registering callback instances while callbacks
 * are already used within a background task, racing conditions might occur.\n
 * In case a flag is used to enable functionality within a seperate background task (even to friendly finish
 * them) the flag has to be at least declared as 'virtual' as otherwise compilers or multi-processor architectures
 * might eliminate this flag during optimization or the flag might not be working due to processor memory buffers.
 */
cFramePort::cFramePort()
{
    pCheckCallback = nullptr;
}

/**
 *  @brief Destructor
 */
cFramePort::~cFramePort()
{
}

/**
 * @brief Sends a byte frame
 * Calling 'Send' without parameters (or parameter pMsg=0) will check the send
 * buffer to be free.\n
 * The implementation of the 'wait' parameter is to be treated as optional.\n
 * \n
 * Implementations inheriting from this class and not implementing the blocking parameter 'wait' should follow the rule
 * not to block q all or to block until the message could be placed within the send buffer within a reasonable time.
 * The second option corresponds to the standard value used for the wait parameter.\n
 * Double buffering is a recommended technique. This means the first call of 'Send' places the message in a usually free
 * buffer. The physic can take time to free the buffer until the next 'send' call is initiated. Most hardware provides double buffering.
 * If this is not the case a software implementation is relatively simple.
 * @param pMsg Pointer to the message (byte frame) to be send
 * @param messageLen Length of the message to be send.
 * @param blockingMode See LibCpp::enBlocking (Standard is enBlockinMode_BUFFER - reasonable short blocking)
 * @return Number of bytes being placed in the send buffer. 0 in case sending was not possible.
 */
int cFramePort::send(const char* pMsg, int messageLen, enBlocking blockingMode) {return 1;}

/**
 * @brief Receives a message (byte frame) typically in a non blocking manner
 * Calling 'receive' without parameters (or parameter pMsg=0) will check the receive
 * buffer for having a message available.\n
 * The method will copy the message from the internal receive buffer to the buffer provided with 'pMsg'.
 * After calling 'receive' the receive message buffer will be freed in case no callback
 * instances are registered. In case callback instances are registered, the call to 'receive' will not free the
 * receive buffer but will be freed automatically after each callback has been processed.\n
 * \n
 * The implementation of the 'blockingMode' parameter is to be treated as optional.
 * It is best practice for hardware independent code to expect no blocking behavior, according to the standard
 * value of the 'blockingMode' parameter.\n
 * Implementations inheriting from this class and not implementing the blocking parameter 'blockingMode' should return immediately.\n
 * Implementations shall add a zero to the provided buffer (unless the raw message size is greater or equal
 * to the buffer size. This added zero will not count as a received byte. Such way \n
 * @param pMsg Pointer to a buffer the received message will be copied to.
 * @param bufferSize Maximum size of the provided buffer.
 * @param blockingMode See LibCpp::enBlocking (Standard is enBlockinMode_NONE - no blocking)
 * @return Number of bytes being received.
 */
int cFramePort::receive(char* pMsg, int bufferSize, enBlocking blockingMode) {return 0;}

/**
 * Receives a message by pointer reference
 * The behavior is identical to LibCpp::cFramePort::receive, but the message is not copied. Instead a pointer
 * to the internal buffer is provided. In case no message is available the method returns zero and the 'char*'
 * value 'ppMsg' is pointed to shall be kept untouched.
 * Be careful when using this method as it is recommended (although possible) not to change the buffer. In case
 * other callback instances need to evaluate the messages they would not find the original message any more. The
 * need to do so, especially appears in case a number format transformation from big to little endian number
 * is necessary to evaluate the message content. In such cases it is necessary to reformat to network byte order.
 * @brief receiveBuffer
 * @param ppMsg Pointer to a variable to copy the internal receive buffer pointer to.
 * @param blockingMode See LibCpp::enBlocking (Standard is enBlockinMode_NONE - no blocking)
 * @return Number of bytes being received.
 */
int cFramePort::receiveBuffer(char** ppMsg, enBlocking blockingMode) {return 0;}

/**
 * @brief Acknowledges the reception of a message after a previous call of LibCpp::cFramePort::receiveBuffer.
 * Implementations need to take care, to free the receive buffer only in case no callback Instances are
 * registered. In cas callbacks are registered the method shall simply return without doing anything.
 * Otherwise subsequend called callback instance would not find a valid message any more. Clearing the
 * receive buffer will be done by the process handling the callback requests.
 */
void cFramePort::receiveAcknowledge() {}

/**
 * @brief Processes message reception in a synchronous (polling) manner
 * The call to this method has only effect in case callback instances are registered and no background task
 * handles message reception.\n
 * It is required to either call 'Receive' or to register callback instances. Doing both leads to an undefined behavior.
 * Calling 'Operate' and 'Receive' on the same port instance within an application should not be done!
 */
void cFramePort::operate() {}

/**
 * @brief Registeres a callback instance for receiving messages
 * This method and its corresponding member 'callbacks' is implemented in a nonthreadsave manner but overriding
 * is usually not necessary, despite you want to create warning messages in case of misuse. Thread save
 * implementations are mostly not intended
 * as message processing would slow down (despite you want to throw warnings on misusage).
 * @param pInstance
 * @return
 */
bool cFramePort::setCallback(iFramePort* pInstance)
{
    if (!pInstance) return true;
    deleteCallback(pInstance);          // Avoids the existence of the same instance twice in the list
    callbacks.push_back(pInstance);
    return true;
}

/**
 * @brief Unregisteres a callback instance for receiving messages
 * See LibCpp::cFramePort::setCallback for documentaion of method behavior.\n
 * Calling this method parameterless (pInstance = nullptr) shall clear all registered callback instances.
 * @param pInstance
 * @return Confirms, the instance is not part of the list (either not any more or never has been)
 */
bool cFramePort::deleteCallback(iFramePort* pInstance)
{
    if (!pInstance)
    {
        callbacks.clear();
    }
    for (int i = (int)callbacks.size() - 1; i >= 0; i--)
        if( callbacks[i]==pInstance )
            callbacks.erase( callbacks.begin()+i );
    return true;
}

/**
 * @brief Registeres a frame check callback instance
 * The method registeres an object handling either frame end detection or frame encoding.
 * See LibCpp::cFramePort::iFramePort for further documentation.\n
 * Behavior is like LibCpp::cFramePort::setCallback.\n
 * To remove the instance call this function with nullptr as parameter (or parameter less).
 * @param pInstance
 * @return
 */
bool cFramePort::setFrameCheckCallback(iFrameCheck* pInstance)
{
    pCheckCallback = pInstance;
    return true;
}

/**
 * @brief Same as LibCpp::cFramePort::send
 * Avoids type conversions for buffers declared as uint8_t (aka unsigned char*).
 * @param pMsg
 * @param messageLen
 * @param blockingMode
 * @return
 */
int cFramePort::send(const uint8_t* pMsg, int messageLen, enBlocking blockingMode)
{
    return send((const char*)pMsg, messageLen, blockingMode);
}

/**
 * @brief Same as LibCpp::cFramePort::receive
 * Avoids type conversions for buffers declared as uint8_t (aka unsigned char*).
 * @param pMsg
 * @param bufferSize
 * @param blockingMode
 * @return
 */
int cFramePort::receive(uint8_t* pMsg, int bufferSize, enBlocking blockingMode)
{
    return receive((char*)pMsg, bufferSize, blockingMode);
}

/**
 * @brief Same as LibCpp::cFramePort::receiveBuffer
 * Avoids type conversions for buffers declared as uint8_t (aka unsigned char*).
 * @param ppMsg
 * @param blockingMode
 * @return
 */
int cFramePort::receiveBuffer(uint8_t** ppMsg, enBlocking blockingMode)
{
    return receiveBuffer((char**)ppMsg, blockingMode);
}

#pragma GCC diagnostic warning "-Wunused-parameter"

/** @} */
