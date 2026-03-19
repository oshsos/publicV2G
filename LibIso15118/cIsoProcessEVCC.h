// UTF8 (ü) //
/**
\file   cIsoProcessEVCC.h

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2024-10-25

\copyright
Published under the provided \ref v2g_license.txt file!
(Free for private use and professional evaluation purpose, license contract required
for professional use e.g. in products.)

\addtogroup G_LibIso15118
@{

\class LibCpp::cIsoProcessEVCC

\tableofcontents

\section S_General General description

This class handles the message sequece the electric vehicle communication controller (EVCC)
sends to and receives from the supply equipment.\n
For simulation usage on message level, this object is the main object providing the functionality.
A corresponding sample project is provided within this Iso 15118 library package.\n
If used for simulation on signal level this class is accompanied with additional
classes for plc and control pilot operation.\n
Real operation with electric vehicle hardware is achieved by providing a hardware interface
class inheriting from the LibIso15118::cEvHardware interface class. A corresponding sample implementation
is also provided with this library.\n
\n
The class requires a link to a LibCpp::cUdpSocket and to a LibCpp::cTcpClient instance. As these
classes are virtual the necessary hardware depending implementations are required. Furthermore
an EXI codec has to be chosen, instanciated and to be attached to the class.\n
Call 'setupCommunication' and
set the member 'evccInput.enableCharging' to 'true' in order to start the message sequence required for charging.
The charging process stopps in case 'evccInput.enableCharging' is reset to 'false'.\n
To get the state machine running the 'operate' method needs to be called repeatedly in a relatively
fast cycle of one to ten Milliseconds. The calls may or may not be in equidistant time steps.\n
In order to start the state machince call operate without a parameter or with parameter 'run'
equal to 'true'. Once running the 'run' parameter has no function until the state machine
has reached the 'finished' or the 'failure' state. In order to restart the charging process
the 'operate' method has to be calles with the paremeter 'run' set to 'false'. This will be
the trigger to bring the state machine back to the 'idle' state. This procedure is only possible
in case the state machine is in either the state 'finished' or 'failure'.\n

\section S_Code Sample code

The following sample code demonstrates the usage of the class.

\code
#include "../LibCpp/HAL/Linux/cLxUdpSocket.h"
#include "../LibCpp/HAL/Linux/cLxTcpClient.h"

#include "../LibCpp/HAL/Tools.h"

#include "../LibIso15118/cExiIso15118_2013_openV2g.h"
#include "../LibIso15118/cIsoProcessEVCC.h"

cDebug dbg("main", enDebugLevel_Debug);
cLxConsoleInput console;

cLxUdpSocket udp;
cLxTcpClient client;
cIsoProcessEVCC isoProcessEVCC(&codecIso, &udp, &client);

void printHelpEVCC()
{
    printf_flush("'q' to finish\n");
    printf_flush("'h' to help\n");
    printf_flush("'s' to friendly stop charging\n");
    printf_flush("'e' enable charging\n");
    printf_flush("\n");
}

int mainEVCC()
{

    printf_flush("KEA publicV2G: Electric Vehicle Sample Program on message Level\n\n");
    printHelpEVCC();

    dbg.setInstanceName("mainEVCC");
    udp.dbg.setInstanceName("port", &dbg, enDebugLevel_Info);
    client.dbg.setInstanceName("client", &dbg, enDebugLevel_Info);
    codecIso.dbg.setInstanceName("codecIso", &dbg, enDebugLevel_Info);
    codecDin.dbg.setInstanceName("codecDin", &dbg, enDebugLevel_Info);
    isoProcessEVCC.dbg.setInstanceName("isoProcessEVCC", &dbg, enDebugLevel_Info);

    if (udp.open(55100, INTERFACE_NAME, LibCpp::enAddressFamily_IPv6) != LibCpp::enIpResult_Success)
    {
        cInterfaces interfaces(true);
        printf("Opening '%s' failed! Available interfaces are:\n%s\n", INTERFACE_NAME, interfaces.toString().c_str());
        interfaces.close();
    }

    udp.setDestination(stIpAddress("FF02::1"), 15118);
    isoProcessEVCC.setupCommunication();
    isoProcessEVCC.evccInput.enable = true;

    while (1)
    {
        // Message process operation
        udp.operate();
        client.operate();

        isoProcessEVCC.operate(isoProcessEVCC.isOperational() && isoProcessEVCC.evccInput.enable);

        if (console.hasKey())
        {
            char input = console.getKey();
            printf_flush("\n", input);
            if (input == 'q')
                break;
            if (input == 'h')
            {
                printHelpEVCC();
            }
            if (input == 's')
            {
                isoProcessEVCC.evccInput.enable = false;
            }
            if (input == 'e')
            {
                isoProcessEVCC.evccInput.enable = true;
            }
        }
    }

    udp.close();
    client.close();

    printf_flush("Ready\n");
    return 0;
}
\endcode

\section S_code Sample code

The implemented state machine basically follows the charging standard.\n
\n
ISO/DIS 15118-2 :2018 (ED2)
- Figure 189 — EVCC communication states for AC V2G messaging
- Figure 190 - EVCC communication states for DC V2G messaging
After entering a certain state the first operation will be sending a request message to the
supply equipment. After the reception of the response the next state will be entered.

**/

#ifndef CISOPROCESSEVCC_H
#define CISOPROCESSEVCC_H

#include "../LibCpp/HAL/cDebug.h"
#include "../LibCpp/HAL/cUdpSocket.h"
#include "../LibCpp/HAL/cTcpClient.h"
#include "../LibCpp/cTimer.h"
#include "isoTypes.h"
#include "cExiCodec.h"
#include "cEvHardware.h"
#include "cControlPilot.h"

namespace Iso15118
{

/**
 * @brief Implementation of the charging process state machine for electric vehicles.
 */
class cIsoProcessEVCC : LibCpp::iFramePort
{
public:
    cIsoProcessEVCC(cExiCodec* pCodec, LibCpp::cUdpSocket* pUdp, LibCpp::cTcpClient* pClient, std::string clientInterface, cControlPilot* pControlPilot = nullptr, cEvHardware* pEv = nullptr); ///< Constructor

    void setupCommunication();                  ///< \copybrief cIsoProcessEVCC::setupCommunication() Links the communication ports to the instance by setting it as callback instance.
    void clear();                               ///< \copybrief cIsoProcessEVCC::clear() Sets the entire instance to its initial values
    void evaluate();                            ///< \copybrief cIsoProcessEVCC::evaluate() Extracts valuable information content out of the 'receivedMessage' and stores it to member variables.
    bool operate(bool run = true);              ///< \copybrief cIsoProcessEVCC::operate() Processes the state machine of the class
    bool isInitialized();                       ///< \copybrief cIsoProcessEVCC::isInitialized() Checks for control pilot and SE hardware to be initialized.
    bool isOperational();                       ///< \copybrief cIsoProcessEVCC::isOperational() The 'operate' method requires to be called with 'false' argument in order to restart the state machine.
    cEvHardware* evHardware();                  ///< \copybrief cIsoProcessEVCC::evHardware()

protected:
    void send(stV2gMessage& sendMsg);                                   ///< \copybrief cIsoProcessEVCC::send()
    void onFrameReceive(LibCpp::cFramePort* pFramePort);                ///< \copybrief cIsoProcessEVCC::onFrameReceive()
    bool responseCodeIsValid(enV2gResponse& code);                      ///< \copybrief cIsoProcessEVCC::responseCodeIsValid()
    bool responseIsValid(enV2gResponse& code, stV2gHeader& v2gHeader);  ///< \copybrief cIsoProcessEVCC::responseIsValid()

public:
    LibCpp::cDebug          dbg;                ///< Instance for debug outputs.

protected:
    cExiCodec*              pCodec;             ///< EXI codec instance to be used.
    LibCpp::cUdpSocket*     pUdp;               ///< UDP communication instance.
    LibCpp::cTcpClient*     pClient;            ///< TCP client instance.
    cControlPilot*          pControlPilot;      ///< Control pilot instance.
    cEvHardware             stdEv;              ///< Initialization instance for EV hardware simulation, in case no hardware is provided with the constructor.
    cEvHardware*            pEv;                ///< EV hardware instance.

public:
    //bool enableCharging;                        ///< Command value to activate or stopping the charging process.
    stEvccProcessDataInput  evccInput;          ///< Stack control data inupt of application program.
    stEvccProcessDataOutput evccOutput;         ///< Main output data inupt to the application program.
    stEvccInfo              evccInfo;           ///< Configuration and time varying control data of the electric vehicle.
    stSeccInfo              seccInfo;           ///< Received configuration and time varying data.
    //unsigned int            schemaId;

    LibCpp::cTimer          responseTimer;      ///< Timer to measure response times.
    uint64_t                responseTimes[(int)enV2gEvState::finished+1];  ///< Array containing the tesponse times of each state.

    //Iso15118::enV2gEvState  v2gState;           ///< The actual state of the state machine.
    bool                    enteredV2gState;    ///< Indicates the first operating cycle for the actual state.

    bool                    connecting;         ///< Indicates a received SDP response in order to await finished TCP connection without sending further SDP requests.

    LibCpp::cTimer          sdpRepeatTimer;     ///< Timer used to generate repetitive messages.
    LibCpp::cTimer          powerRepeatTimer;   ///< Timer used to generate fast repetitive messages.

    std::string             clientInterface;    ///< Ethernet Interface beeing used by the TCP client.
protected:
    char                    sendMessage[LibCpp_cIp_TCP_BUFFERSIZE];    ///< Buffer used to write the EXI coded message to, which will be sent aterwards.
    stV2gMessage            receivedMessage;    ///< Latest received message not being processed by the state machine.
};
}
#endif
/** @} */
