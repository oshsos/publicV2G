// UTF8 (ü) //
/**
\file   cFramePort.h

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2024-01-01

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL
@{

\class LibCpp::cFramePort

\section SEC_LibCpp_cFramePort_Des Description

The class 'cFramePort' is an abstract interface class encapsulating the general sending
and receiving of byte arrays. The basic objective is the abstraction of message exchange
through any kind of physical communication channel.

\section SEC_LibCpp_cFramePort_Bib Library context

The class is part of the \ref G_LibCpp_HAL and designed according to the general handling
of communication objects providing message reception by polling as well as by synchronous
and asynchronous callback functions mechanism.

\section SEC_LibCpp_cFramePort_Exam Usage examples

The following examples describe the class usage for the three different methods of message
reception.

\subsection SEC_LibCpp_cFramePort_Exam1 Using the polling mechanism

The most simple usage is receiving messages by polling.

\code
    #include "cFramePort.h"

    using namespace LibCpp;

    bool    send  = true;
    bool    async = false;

    char    receiveBuffer[256];
    int     receiveBufferLen = 0;

    cFramePort	port(async);

    main()
    {
        while(1)
        {
            if (send)
            {
                port.send("Hallo", 5);
                send = false;
            }

            if ( (receiveBufferLen = port.receive(receiveBuffer, 256) )
            {
                printf("%s", receiveBuffer);
            }
        }
    }
\endcode

\subsection SEC_LibCpp_cFramePort_Exam2 Using synchronous callback functions

To handle message reception by a callback mechanism it is required to define a class
inherited from 'iFramePort'. This class provides a virtual method 'onFrameReceive' which
is requrired to be implemented and served as the receiving callback function.\n
An instance of this receiving class can be registered at the 'cFramePort' instance 'port'
by the 'setCallback' method.\n
It is possible to register ther receiving instance at several port instances. In order to
distinguish which port actually received the message, a pointer to the port is provided
as argument of the 'onFrameReceive' method.\n
Messages are actually received and subsequently callbacks are actually called in a synchronous
way during the call to the 'operate' method of the 'cFramePort' instance.

\code
    #include "cFramePort.h"

    using namespace LibCpp;

    bool    send  = true;
    bool    async = false;

    char    receiveBuffer[256];
    int     receiveBufferLen = 0;

    class cFrameReceive : public iFramePort
    {
        void onFrameReceive(cFramePort* pPort)
        {
            receiveBufferLen = port.receive(receiveBuffer, 256);
            printf("%s", receiveBuffer);
        }
    } frameReceive;

    cFramePort	port(async);

    main()
    {
        port.setCallback(&frameReceive);

        while(1)
        {
            if (send)
            {
                port.send("Hallo", 5);
                send = false;
            }

            port.operate();
        }
    }
\endcode

\subsection SEC_LibCpp_cFramePort_B3 Implementierung im Interruptempfangsverfahren

The asynchronous message reception is pretty much the same as the synchronous method.
The only difference is no need to call the 'operate' method repeatedly as this is done
by the background process.\n
On the other hand the receiving method of the receiving instance is part of the background
process and not of the main process any more. Therefore, accessing commonly used variables
must be synchronized by the user application leading to quite some effort.\n
The easiest way is to implement an atomic interger value indicating the buffer usage.

\code
    #include <atomic>
    #include "cFramePort.h"

    using namespace LibCpp;

    bool    send  = true;
    bool    async = true;

    char        receiveBuffer[256];
    atomic<int> receiveBufferLen = 0;

    class cFrameReceive : public iFramePort
    {
        void onFrameReceive(cFramePort* pPort)
        {
            if (!receiveBufferLen)
                receiveBufferLen = port.receive(receiveBuffer, 256);
        }
    } frameReceive;

    cFramePort	port(async);

    main()
    {
        port.setCallback(&frameReceive);

        while(1)
        {
            if (send)
            {
                port.send("Hallo", 5);
                send = false;
            }

            if (receiveBufferLen)
            {
                printf("%s", receiveBuffer);
                receiveBufferLen = 0;
            }
        }
    }
\endcode

\section SEC_LibCpp_cFramePort_Dir Direct message reception

As the message transmitting instance (e.g. 'port') usually carries a memory buffer for
receiving messages, it is quite often unwanted to copy this buffer to another buffer
provided by the application (e.g. 'receiveBuffer'). Therefore, the class 'cFramePort'
provides the method 'receiveBuffer' just returning the number of received bytes and
setting a message pointer to the point to the internal message buffer. After evaluation
the message has to be indicated as received by calling the method 'receiveAcknowledge'.
It is required to do so in polling operation, in case of registered callbacks the
acknowledge is executed automatically after all callbacks have been called.\n
\n
Instead of coding

\code
    char    receiveBuffer[256];
    int     receiveBufferLen = 0;

    if ( (receiveBufferLen = port.receive(receiveBuffer, 256) )
    {
        printf("%s", receiveBuffer);
    }
\endcode

it is much faster and simpler to code

\code
    char* pBuf = 0;

    if ( port.receiveBuffer(&pBuf) )
    {
        printf("%s", pBuf);
        port.receiveAcknowledge();
    }
\endcode

Be aware, 'port.receiveBuffer(&pBuf)' returns the number of received bytes in case you need
to use this value later.

\section SEC_LibCpp_cFramePort_Imp Implementation requirements

- Implementations of cFramePort shall allow to call the 'send' method within the receiving
callback functions.
- After the reception of data the next byte within the class internal reception buffer shall
be appended by a zero byte. Nevertheless this zero byte is not counted for the message length.
The reason is to enshure safe printing of the message buffer as a character string. The zero
byte shall also be copied to the application message buffer within the 'send' method. No zero
is appended if the number of received bytes is equal to the lenght of the provided reception
buffer.
**/

#ifndef CFRAMEPORT_H
#define CFRAMEPORT_H

#ifdef PROJECTDEFS
    #include <ProjectDefs.h>
#endif

#ifndef LibCpp_cFramePort_BUFFERSIZE
    #define LibCpp_cFramePort_BUFFERSIZE 4096
#endif

#include <cstdint>
#include <vector>

namespace LibCpp
{

/**
 * @brief Defines the blocking mode of sending and receiving methods
 */
enum enBlocking : int
{
    enBlocking_NONE = 0,        ///< Method will not block and immediately return
    enBlocking_BUFFER = 1,      ///< Blocks until the message is placed in the send buffer within a usually acceptable and reachable short time. (equals NONE on receiving)
    enBlocking_SENT = 2,        ///< Blocks until the message has been physically sent on the medium. (equals NONE on receiving)
    enBlocking_BLOCKING= 3,     ///< Blocks until a message has actually been received - possibly with a time out condition. (equals SENT on sending)
    enBlocking_ACKNOWLEDGE= 4   ///< Blocks until a sent message has been acknowledged by the receiver - possibly with a time out condition. (equals BLOCKING on receiving)
};

class cFramePort;

/**
 * @brief Interface class for objects checking input stream data for finished frames
 */
class iFrameCheck
{
public:
    iFrameCheck();                                                  ///< Constructor
    virtual ~iFrameCheck();                                         ///< Destructor
    virtual bool onReceiveFrameCheck(cFramePort* pFramePort, char* receiveBuffer, unsigned int* pLen, unsigned int bufferSize = 0); ///< External method to check input stream data for completed frames and to manipulate received data
    virtual int onSendFrameCheck(cFramePort* pFramePort, char* sendBuffer, unsigned int len, char** ppNewSendBuffer);    ///< External method to check input stream data for completed frames and to manipulate received data
};

/**
 * @brief Interface class for objects requiring receive callbacks
 */
class iFramePort : public iFrameCheck
{
public:
    iFramePort();                                                   ///< Constructor
    virtual ~iFramePort();                                          ///< Destructor
    virtual void onFrameReceive(cFramePort* pFramePort) = 0;        ///< Receive callback method. Pure virtual method forces the user to implement this method.
};

/**
*   @brief Abstract interface class for transmission and reception of byte arrays
*/
class cFramePort
{
public:
    cFramePort();                                                   ///> Constructor
    virtual ~cFramePort();                                          ///> Destructor

    // Required implementations
    virtual int  send(const char* pMsg = nullptr, int messageLen = 0, enBlocking blockingMode = enBlocking_BUFFER);             ///> Sends a data frame.
    virtual int  receive(char* pMsg = nullptr, int bufferSize = 0, enBlocking blockingMode = enBlocking_NONE);                  ///> Receives a data frame.
    virtual int  receiveBuffer(char** ppMsg = nullptr, enBlocking blockingMode = enBlocking_NONE);                              ///> Receives a data frame without copying
    virtual void receiveAcknowledge();                              ///> Acknowledges a reception of a message by 'receiveBuffer'
    virtual void operate();                                         ///> Cyclic operation on callback polling mode usage.

    // Preimplemented but overridable methods.
    virtual bool setCallback(iFramePort* pInstance);                ///> Sets an instance to be called at message receptions.
    virtual bool deleteCallback (iFramePort* pInstance = nullptr);  ///> Removes an instance from the callback list.
    virtual bool setFrameCheckCallback (iFrameCheck* pInstance = nullptr);    ///> Sets an instance to control data framing and encoding.

    // Preimplemented methods
    int  send(const uint8_t* pMsg = nullptr, int messageLen = 0, enBlocking blockingMode = enBlocking_BUFFER);         ///> Sends a message (type uint8_t).
    int  receive(uint8_t* pMsg = nullptr, int bufferSize = 0, enBlocking blockingMode = enBlocking_NONE);              ///> Receives a message (type uint8_t).
    int  receiveBuffer(uint8_t** ppMsg = nullptr, enBlocking blockingMode = enBlocking_NONE);                          ///> Receives a message (type uint8_t) without copying.

    std::vector<iFramePort*> callbacks;                             ///> List of registered callback instances
    iFrameCheck*             pCheckCallback;                        ///> Callback for frame detection and message encoding
};

}
#endif

/** @} */
