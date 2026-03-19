// UTF8 (ü) //
/**
\file cCanPort.cpp

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2023-12-29

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL
@{

\class LibCpp::cCanPort
\brief Virtuelles Objekt zur Kapselung einer CAN-Bus Anbindung

\section cCanPort_Bib Bibliothekszuordnung

Das Objekt obCanPort ist ein virtuelles Objekt des \ref G_LibCpp_HAL Hardware Abstraction Layers
und ist gemaess der grundsaetzlichen objektorientierten Kommunikationsstruktur des Abschnittes
Kommunikation aufgebaut.\n
\n
Implementierungen liegen fuer folgenden Hardwareplattformen vor:

Windows:	LibCpp::cWCanPort
Linux:      LibCpp::cLxCanPort

\section cCanPort_B Beschreibung

Mit diesem virtuellen Objekt wird ein CAN Treiber gekapselt, der Nachrichten in Form einer Que (Nachrichtenpuffer)
verarbeitet.\n
Die grundsätzliche Funktion wird durch die folgenden Beispiele beschrieben. Das dabei verwendete
Objekt des Typs LibCpp::cCanPort ist in einer Anwendung durch das hardwarespezifische Objekt, also z.B.
im Windows durch LibCpp::cWCanPort zu ersetzen.

\subsection cCanPort_B1 Implementierung im Pollingverfahren

Die einfachste Form der Anwendung sieht wie folgt aus:

\code
	#include "cCanPort.h"
    using namespace LibCpp;

	int Send=1;

	stCanMsg	Msg;
	cCanPort	Can;

	main()
	{
		while(1)
		{
			if (Send)
			{
                Msg.id		  = 0x100;
                Msg.len		  = 2;
                Msg.data.b[0] = 1;
                Msg.data.b[1] = 2;

				Can.pM->Send(&Can,&Msg,1);

				Send = 0;
			}
			if ( Can.Receive(&Msg) )
			{
                int ID = Msg.id;
			}
		}
	}
\endcode

\subsection cCanPort_B2 Implementierung im Pollingverfahren mit Empfangs-CallBack-Instanz

Der Call Back Mechanismus der Empfangsfunkton kann wie folgt benutzt werden:

\code
	#include "cCanPort.h"
    using namespace LibCpp;

	int Send=1;

	class cCanReceive : public iCanPort
	{
		void OnCanReceive(cCanPort* pCanPort)
		{
			stCanMsg Msg;
			pCanPort->Receive(&Msg);
		}
	} CanReceive;

	cWCanPort Can(&CanReceive, false);

	main()
	{
		while(1)
		{
			if (Send)
			{
				stCanMsg Msg;
                Msg.id		  = 0x100;
                Msg.len		  = 2;
                Msg.data.b[0] = 1;
                Msg.data.b[1] = 2;

				Can.Send(&Msg);

				Send = 0;
			}
			Can.Operate();
		}
	}
\endcode

\subsection cCanPort_B3 Implementierung im Interruptempfangsverfahren

In diesem Fall wird die Empfangs Call-Back-Funktion aufgrund eines Hardwareinterruptes aufgerufen,
muss also nicht mehr regelmässig abgefragt (gepollt) werden. Es ist dabei aber zu beachten, dass
die Empfangsfunktion in einer anderen Task abgearbeitet wird als die Task in der gesendet wird.
\n
Nicht jede Hardwareimplementierung muss diese Methode implementiert haben, da diese durch Aufruf
der Operate-Methode in einem Hintergrundprozess der Hardware (z.B. periodischer Timeraufruf) leicht
nachgebildet werden kann.

\code
	#include "cCanPort.h"
	using namespace BibCpp;

	int Send=1;

	class cCanReceive : public iCanPort
	{
		void OnCanReceive(cCanPort* pCanPort)
		{
			stCanMsg Msg;
			pCanPort->Receive(&Msg);
		}
	} CanReceive;

	cWCanPort Can(&CanReceive, true);

	main()
	{
		while(1)
		{
			if (Send)
			{
				stCanMsg Msg;
                Msg.id		  = 0x100;
                Msg.len		  = 2;
                Msg.data.b[0] = 1;
                Msg.data.b[1] = 2;

				Can.Send(&Msg);

				Send = 0;
			}
		}
	}
\endcode

\subsection cCanPort_B4 Verwendung von RTR-Nachrichten

Für das Versenden von RTR-Nachrichten (Request Transmit message) ist Msg.RTR auf 'true' zu setzen.\n
Der Empfang einer RTR-Nachricht kann über das Strukturelement RTR erkannt werden.\n
\code
if (Msg.id==0x100 && Msg.rtr) ...
\endcode
*/

#include "cCanPort.h"

#include "Tools.h"

using namespace std;
using namespace LibCpp;

/** @brief Initializer */
stCanMsg::stCanMsg(uint32_t id, uint8_t len, uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7, uint8_t rtr, uint8_t ext)
{
    set(id, len, b0, b1, b2, b3, b4, b5, b6, b7, rtr, ext);
}

/** @brief (Re-)sets the struct values */
void stCanMsg::set(uint32_t id, uint8_t len, uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7, uint8_t rtr, uint8_t ext)
{
    this->id = id;
    this->len = len;
    this->data.b[0] = b0;
    this->data.b[1] = b1;
    this->data.b[2] = b2;
    this->data.b[3] = b3;
    this->data.b[4] = b4;
    this->data.b[5] = b5;
    this->data.b[6] = b6;
    this->data.b[7] = b7;
    this->rtr = rtr;
    this->ext = ext;
    this->timeStamp = 0;
}

/** @brief Converts the can message contet to a string */
string stCanMsg::toString()
{
    string out;
    const char* rtrStr = rtr ? "true" : "false";
    const char* extStr = ext ? "true" : "false";
    if (ext==1)
        out += stringFormat("Ext");
    out += stringFormat("Id=0x%X Len=%i Data=%s", id, len, ByteArrayToString(&data.b[0], (unsigned int)len).c_str(), rtrStr, extStr);
    if (rtr==1)
        out += stringFormat(" RTR=true");
    return out;
}

/** @brief Converts LibCpp::enCanBitRate to a string */
string enCanBitRate_toString(enCanBitRate bitRate)
{
    switch (bitRate)
    {
    case enCanBitRate_1000k: return u8"1000k";
    case enCanBitRate_800k:  return u8"800k";
    case enCanBitRate_500k:  return u8"500k";
    case enCanBitRate_250k:  return u8"250k";
    case enCanBitRate_125k:  return u8"125k";
    case enCanBitRate_Auto:  return u8"Auto";
    case enCanBitRate_Std:  return u8"Standard(typ. 500k)";
    default: return u8"<unspecified>";
    }
}

/** @brief Converts LibCpp::enCanState to a string */
string enCanState_toString(enCanState state)
{
    switch (state)
    {
    case enCanState_UNDEFINED: return u8"UNDEFINED";
    case enCanState_ACTIVE:  return u8"ACTIVE";
    case enCanState_WARNING: return u8"WARNING";
    case enCanState_PASSIVE: return u8"PASSIVE";
    case enCanState_BUSOFF:  return u8"BUSOFF";
    case enCanState_OVERRUN: return u8"OVERRUN";
    default: return u8"<undefined>";
    }
}

#pragma GCC diagnostic ignored "-Wunused-parameter"

/**
 * @brief Constructor for the CAN port interface object required for receiving messages by the callback mechanism
 * Implementations of this virtual object usually do not override constructor or destructor.\n
 * Instead Implementations require the Implementation of the method LibCpp::iCanPort::OnCanReceive.\n
 * Application specific objects inherit from this virtual interface class if they require callback notifications.
 * Mostly they inherit from an applicaton specific base object and secondly from iCanPort.
 */
iCanPort::iCanPort() {}

/** @brief Destructor for the CAN port interface object. */
iCanPort::~iCanPort() {}

/**
 * @brief Constructor of the virtual CAN bus interface
 * It is intended for each interface object within the \ref G_LibCpp_HAL to be not operational after standard object construction,
 * as otherwise creating arrays of interface objects would not be possible.
 * On the other hand the creation of operational objects should as easy as possible.
 * For that reason the first parameter should be a boolean indicating to open the object
 * within the constructor.
 * Therefore, implementations should automatically call the 'open' method, unless the callback functions are called from a seperate
 * task (or interrupt service routine) handling data reception. In such cases the 'open' method shall be required to be
 * called explicitly to enable callback calls. In case a flag is used to enable functionality within the seperate task but being set
 * from the main task, the flag has to be at least declared as 'virtual' as otherwise compilers or multi-processor architectures
 * might eliminate this flag during optimization or the flag might not be working due to processor memory buffers.
 * In order to avoid racing conditons, all registrations of callback objects are required to be finished before the seperate
 * task accesses the callback object list.
 */
cCanPort::cCanPort(bool open)
{
}

/**
 *  @brief Destructor
 *  The destructor shall call close automatically.
 */
cCanPort::~cCanPort() {}

/**
 * @brief Opens (or starts) callback operations on data reception
 * In case the constructor of an implementation inheriting from this object deliveres the option not to reserve hardware
 * recources immediately (see LibCpp::cCanPort::cCanPort) the 'open' method might also be used to
 * take access to hardware recources.\n
 * In case 'open' is called but the interface has already been opened the method shall return true. In such way the 'open'
 * method may be used without conflict if the constructor has opened the port already.
 * A call to 'open' shall restart the interface access after a 'close' call has been initiated in advance.
 */
bool cCanPort::open() {return true;}

/**
 * @brief cCanPort::Close
 * A call to 'Close' shall at least wait for callback methods being finished and stop further calls. It is strongly
 * recommended to release all hardware recources, as programmers of hardware independend code will expect such an
 * behavior.\n
 * It should be taken in account this function might be time consuming, as implementation might close a background
 * task in a friendly manner possibly with the need to wait for a certain timeout.
 * @return Successful hardware allocation
 */
void cCanPort::close() {}

/**
 * @brief Sends a message
 * Calling 'Send' without parameters (or parameter pMsg=0) will check the send
 * buffer to be free.\n
 * The implementation of the 'wait' parameter is to be treated as optional.\n
 * \n
 * It is best practice for hardware independent code to evaluate the return value of this method. Hardware independent
 * code relying on that feature needs to specify that only special implementations can be used with that code.\n
 * \n
 * Implementations inheriting from this class and not implementing the blocking parameter 'wait' should follow the rule
 * not to block at all or to block until the message could be placed within the send buffer within a reasonable time.
 * The second option corresponds to standard value '1' used for the wait parameter.\n
 * Double buffering is a recommended technique. This means the first call of 'Send' places the message in a usually free
 * buffer. The physic can take time until the next 'Send' call is initiated and blocks for a reasonable time until the
 * double buffer is free again. Most hardware provides double buffering. If this is not the case a software implementation
 * is relatively simple.\n
 * \n
 * The 'wait' parameter used to set blocking modes are defined in the following manner:\n
 * 0 = \n
 * not blocking\n
 * 1 = \n
 * blocks till the message could be placed within the send buffer within a usually acceptable and reachable short time.\n
 * 2 = \n
 * blocks until the message has physically been sent, before a usually configurable (but by hardware independent code not
 * variable) timeout (e.g. defined within the constructor) has been reached.\n
 * 3 = \n
 * blocks until a confirmation message from the destination has been received within the configured timeout. The definition
 * of such a response message is up to the application software. A previous 'Send' with wait=-1 might be a proposal to
 * define the expected response message.
 *
 * @param pMsg Pointer to the Message to be sent
 * @param wait Blocking mode according to the above description (standard: 1 = reasonable blocking)
 * @return Successful message placement without timeout according to the 'wait' parameter.
 */
bool cCanPort::send(stCanMsg* pMsg, int wait)
{return 1;}

/**
 * @brief Receives a message in a non blocking manner
 * Calling 'Receive' without parameters (or parameter pMsg=0) will check the receive
 * buffer for having a message available.\n
 * After calling 'Receive' the receive message buffer within the CAN port instance will be freed, in case no callback
 * instances are registered. In case callback instances are registered, the call to 'Receive' will not free the
 * receive buffer but will be freed automatically after each callback has been processed.\n
 * \n
 * The implementation of the 'wait' parameter is to be treated as optional (see LibCpp::cCanPort::Send).\n
 * \n
 * It is best practice for hardware independent code to expect no blocking behavior, according to the standard
 * value of the 'wait' parameter.\n
 * \n
 * Implementations inheriting from this class and not implementing the blocking parameter 'wait' should immediately return.\n
 * \n
 * The 'wait' parameter used to set blocking modes are defined in the following manner:\n
 * 0 = \n
 * non blocking immediate return\n
 * 1 = \n
 * blocks till a message has been received or before a usually configurable (but by hardware independent code not
 * variable) timeout (e.g. defined within the constructor) has been reached.\n
 * 2 = \n
 * blocks for ever until a message has been received.
 *
 * @param pMsg Pointer to a place the received message will be copied to.
 * @param wait Blocking mode according to the above description (standard: 0 = immediate return)
 * @return Successful message reception.
 */
bool cCanPort::receive(stCanMsg* pMsg, int wait)
{return 0;}

/**
 * @brief Processes message reception in a synchronous (polling) manner
 * The call to this method has only effect in case callback instances are registered and no background task
 * handles message reception.\n
 * It is required to either call 'Receive' or to register callback instances. Doing both leads to an undefined behavior.
 * Calling 'Operate' and 'Receive' on the same CAN port instance within an application should not be done!
 */
void cCanPort::operate()
{}

/**
 * @brief Receives the state of the CAN port hardware
 * @return State
 */
enCanState cCanPort::getCanState()
{return enCanState_ACTIVE;}

/**
 * @brief Sets the bit rate of the CAN bus
 * A call to this will probaply close and reopen the CAN hardware.
 * @param bitRate
 * @return Success
 */
bool cCanPort::setBitRate(enCanBitRate bitRate)
{return false;}

/**
 * @brief setFilter
 * A received message matches on: receivedId & mask == id & mask.
 * @param id
 * @param mask
 * @return
 */
bool cCanPort::setFilter(uint32_t id, uint32_t mask)
{return false;}

/**
 * @brief cCanPort::getCount
 * @return
 */
int cCanPort::getCount()
{
    return 0;
}

/**
 * @brief Registeres a callback instance for receiving messages
 * This method and its corresponding member 'callbacks' is implemented in a nonthreadsave manner but overriding
 * is usually not necessary (if rules of 'open' are obeyed). Thread save implementations are mostly not intended
 * as message processing would slow down (despite you want to throw warnings on misusage).
 * @param pInstance
 * @return
 */
bool cCanPort::setCallback(iCanPort* pInstance)
{
    if (!pInstance) return true;
    deleteCallback(pInstance);          // Avoids the existence of the same instance twice in the list
    callbacks.push_back(pInstance);
    return true;
}

/**
 * @brief Registeres a callback instance for receiving messages
 * This method and its corresponding member 'callbacks' is implemented in a nonthreadsave manner but overriding
 * is usually not necessary (if rules of 'open' are obeyed). Thread save implementations are mostly not intended
 * as message processing would slow down (despite you want to throw warnings on misusage).\n
 * Calling this method parameterless (pInstance = nullptr) will clear all registered callback instances.
 * @param pInstance
 * @return Confirms, the instance is not part of the list (either not any more or not has been)
 */
bool cCanPort::deleteCallback(iCanPort* pInstance)
{
    if (!pInstance)
    {
        callbacks.clear();
    }
    for (int i = callbacks.size() - 1; i >= 0; i--)
        if( callbacks[i]==pInstance )
            callbacks.erase( callbacks.begin()+i );
    return true;
}

/**
 * @brief Registeres a callback instance to be notyfied in case of CAN bus state changes
 * Behavior is like LibCpp::cCanPort::setCallback.
 * @param pInstance
 * @return
 */
bool cCanPort::setStateCallback(iCanState* pInstance)
{
    if (!pInstance) return true;
    deleteStateCallback(pInstance);          // Avoids the existence of the same instance twice in the list
    stateCallbacks.push_back(pInstance);
    return true;
}

/**
 * @brief Registeres a callback instance for receiving messages
 * Behavior is like LibCpp::cCanPort::deleteCallback.
 * @param pInstance
 * @return Confirms, the instance is not part of the list (either not any more or not has been)
 */
bool cCanPort::deleteStateCallback(iCanState* pInstance)
{
    for (int i = callbacks.size() - 1; i >= 0; i--)
        if( stateCallbacks[i] == pInstance )
            stateCallbacks.erase( stateCallbacks.begin()+i );
    return true;
}

#pragma GCC diagnostic warning "-Wunused-parameter"

/** @} */
