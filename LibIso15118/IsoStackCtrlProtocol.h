// UTF8 (ü) //
/**
\file   cIsoStackCtrlProtocol.h

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2025-04-10

Published under MIT license (see provided license.txt file)

\page Page_IsoStackCtrlProtocol Message protocol controling a supply equipment or an ISO 15118 stack control device

The file cIsoStackCtrlProtocol.h provides predefined messages to be used to control the publicV2G stack together with corresponding enumerations.
The intention for these messages ist to control a complete supply equipment from outside or to control the stack running as an own
executable or even on a different hardware control device. A typical case is to provide these messages through an
ethernet interface but is not limited to that solution.

\image html Iso15118_ExternalControl.svg

- Complete supply equipment control

For this case the following two message structs are defined, one to be sent as a command to the stack includes within the supply equipment,
the other to be received from the stack providing state information.

<table>
<caption id="control_messages">Control messages</caption>
<tr><th>Command: <td>Iso15118::stIsoStackCmd
<tr><th>State:   <td>Iso15118::stIsoStackState
</table>

- ISO 15118 stack control device as external executable or hardware

In this cas the stack requires information about the supply equipment hardware such as the power electronics and the stack
prvides command values for the hardware such as closing the main switch of the required output current.\n
This information is added to the data required for the overlaying control as it is the case for the complete supply equipment control.

<table>
<caption id="control_messages">Control messages</caption>
<tr><th>Command: <td>Iso15118::stSeIsoStackCmd
<tr><th>State:   <td>Iso15118::stSeIsoStackState
</table>

\addtogroup G_LibIso15118
@{

**/

#ifndef ISOSTACKCTRLPROTOCOL_H
#define ISOSTACKCTRLPROTOCOL_H

#include <cstdint>
#include <string>

namespace Iso15118
{

/** \brief Enumeration for message type identification. */
enum class enIsoStackMsgType : uint8_t
{
    CtrlCmd     = 0,        ///< Process data commanding the stack.
    CtrlState   = 1,        ///< Process data delivering the state and baic identification information of the stack.
    ParamRead   = 2,        ///< Sets all configuration parameters of the stack.
    ParamWrite  = 3,        ///< Reads all operational parameters of the stack.
    SeCtrlCmd   = 4,        ///< Process data command including SE hardware state.
    SeCtrlState = 5         ///< Process data state including the SE hardware command.
};

/** \brief Enumeration for Iso 15118 stack control states
 * This enumeration is closely related to states that are required to be visualized
 * to the user of the charging supply equipment (HMI).
 * A charging process can be treated to be active during the states 'ready' or 'charging' until 'finished' or 'error'.\n
 * \n
 * The state functionalities and state transitions are explained within the following graph.
 *
 * \image html IsoChargingStates.svg
 */
enum class enIsoChargingState : uint8_t
{
    off             = 0,    ///< The charging station has no input power supply or an severe error (communication may have seperate supply).
    idle            = 1,    ///< No plug is connected to the charging socket
    connected       = 2,    ///< The Iso message exchange is being executed to collect charging session relevant data.
    authorization   = 3,    ///< The process of checking the validity of the certificate is ongoing.
    identification  = 4,    ///< The stack awaits an external identification confirmation.
    preparation     = 6,    ///< The charging process is (still) getting technically prepared (e.g. cable check).
    ready           = 7,    ///< The charging session is set up but the vehicle not yet requested power transfer by the pilot pin.
    charging        = 8,    ///< Power is being active (but may also be zero).
    stop            = 9,    ///< Power transfer is interrupted and the charging session is being to be finisched.
    finished        = 10,   ///< Charging has been shut down, but the plug is still connected.
    error           = 11    ///< An probably resettable error has been occurred. The plug has to be disconnected to reset the error.
};

std::string enIsoChargingState_toString(enIsoChargingState value);  ///< \copybrief enIsoChargingState_toString

/** \brief Enumeration to specify the number of phases
 * In order to calculate the actual power from current and voltage, it is required to know e.g. about a 3 phase supply.
 */
enum class enSupplyPhases : uint8_t
{
    dc  = 0,                ///< Direct current supply mode
    ac1 = 1,                ///< Alternating current one phase  mode (current and voltage are effective values)
    ac3 = 3,                ///< Alternating current three phase  mode (current and voltage are effective values)
};

#pragma pack (push,1)

/** \brief Command message protocol struct for Iso15118 stack control. Version: 0, Type CtrlCmd

<b>stIsoStackCmd:</b>

byte    | 0          | 1                           | 2      | 3        | 4         | 5 | 6-7
:------:|:----------:|:---------------------------:|:------:|:--------:|:---------:|:-:|:-------------:
content | msgVersion | msgType                     | enable | verified | preferred | - | currentDemand
unit    | 0          | Iso15118::enIsoStackMsgType | bool   | bool     | bool      | - | A / 10

<b>Byte order: big endian</b>\n

Back to \ref Page_IsoStackCtrlProtocol

*/
struct stIsoStackCmd
{
    uint8_t             msgVersion;     ///< Message format version.
    enIsoStackMsgType   msgType;        ///< Indicator of message type.
    uint8_t             enable;         ///< Enables a connection process. A disable will interrupt and stop the charging session.
    uint8_t             verified;       ///< Confirmation of a successful identification to continue the session set up.
    uint8_t             preferred;      ///< Signal for preferred charging times. A vehicle with no need for immediate charging may choose to charge during preferred times, only.
    uint8_t             padding0;       ///< padding to short
    uint16_t            currentDemand;  ///< Current demand (/ A/10, big endian) the supply equipment orderes the vehicle to receive

    stIsoStackCmd();                    ///< Initializer
    void clear();                       ///< Sets the data content to initial values
};

/** \brief State message protocol struct for Iso15118 stack control. Version: 0, Type CtrlState

<b>stIsoStackState:</b>

byte    | 0          | 1                           | 2                            | 3                        | 4-5     | 6-7     | 8-14   | 15 | 16-23  | 24-29   | 30-31 | 32-39     | 40-41          | 42-43         | 44-51
:------:|:----------:|:---------------------------:|:----------------------------:|:------------------------:|:-------:|:-------:|:------:|:--:|:------:|:-------:|:-----:|:---------:|:--------------:|:-------------:|:------------:
content | msgVersion | msgType                     | state                        | supplyPhases             | current | voltage | seccId | -  | evccId | evccMac | -     | sessionId | energyCapacity | energyRequest | departureTime
unit    | 0          | Iso15118::enIsoStackMsgType | Iso15118::enIsoChargingState | Iso15118::enSupplyPhases | A / 10  | V / 10  | string | 0  | number | -       | -     | number    | kWh / 10       | kWh / 10      | unix time / s

<b>Byte order: big endian</b>\n

Back to: \ref Page_IsoStackCtrlProtocol

*/
struct stIsoStackState
{
    uint8_t             msgVersion;         ///< Message format version.
    enIsoStackMsgType   msgType;            ///< Indicator of message type.
    enIsoChargingState  state;              ///< State of the Iso 15118 stack while setting up and running a charging session.
    enSupplyPhases      supplyPhases;       ///< Number of supply phases or DC.
    uint16_t            current;            ///< Actual current (/ Arms/10, big endian) the supply equipment orderes the vehicle to receive.
    uint16_t            voltage;            ///< Actual supply voltage (/ Vrms/10).
    uint8_t             seccId[7];          ///< Seven byte character string identifying the supply equipment.
    uint8_t             charEnd;            ///< Zero as string termination
    uint8_t             evccId[8];          ///< Eight byte number identifying the vehicle.
    uint8_t             evccMac[6];         ///< MAC address the evcc plc modem is using.
    uint8_t             padding0[2];        ///< padding to long.
    uint8_t             sessionId[8];       ///< Eight byte number identifying the charging session.
    uint16_t            energyCapacity;     ///< Battery charge capacity (/ kWh/10).
    uint16_t            energyRequest;      ///< Requested energy for the current charging session.
    uint32_t            departureTime;      ///< Expected time the charging process may durate.


    stIsoStackState();                      ///< Initializer
    void clear();                           ///< @copybrief stIsoStackState::clear
    stIsoStackState bigEndian();            ///< @copybrief stIsoStackState::bigEndian
};

/** \brief Command and state message protocol struct for for supply equipment hardware. Version: 0

This message protocol is appended to the LibIso15118::stIsoStackCtrlCmd respectively the LibIso15118::stIsoStackCtrlState format.\n


<b>stSeHardwareCtrl:</b>

byte 8/40 + | 0             | 1      | 2            | 3                    | 4-5           | 6-7
:----------:|--------------:|:------:|:------------:|:--------------------:|:-------------:|:------------:
content     | mainContactor | imd    | sourceEnable | sourceCurrentControl | sourceVoltage | sourceCurrent
unit        | bool          | t.b.d. | bool         | bool                 | V / 10        | A / 10

<b>Byte order: big endian</b>\n

Back to: \ref Page_IsoStackCtrlProtocol

*/
struct stSeHardwareCtrl
{
    uint8_t             mainContactor;          ///< Control or state of the main contactor.
    uint8_t             imd;                    ///< Control command for repectively state of the IMD device. (undefined yet)
    uint8_t             sourceEnable;           ///< Activate the power source.
    uint8_t             sourceCurrentControl;   ///< Control mode of the power source.
    uint16_t            sourceVoltage;          ///< Voltage demand respectively limit (/ V/10, big endian) the supply equipment orderes the vehicle to receive
    uint16_t            sourceCurrent;          ///< Current demand respectively limit (/ A/10, big endian) the supply equipment orderes the vehicle to receive
    uint16_t            padding0;               ///< padding to long.

    stSeHardwareCtrl();                         ///< Initializer
    void clear();                               ///< Sets the data content to initial values
};

/** \brief  Command message protocol struct for external stack hardware. Version 0, Type SeCtrlCmd
 *
 * <b>stSeIsoStackCmd:</b>
 *
 * byte    | 0-7           | 8-15
 * :------:|:-------------:|:---------------:
 * content | stIsoStackCmd | stSeHardwareCtrl
 *
 * Back to: \ref Page_IsoStackCtrlProtocol
 */
struct stSeIsoStackCmd
{
    stIsoStackCmd           isoStackCmd;        ///< Command to the iso stack.
    stSeHardwareCtrl        seHardwareState;    ///< State of the SE hardware.

    stSeIsoStackCmd();                          ///< Initializer
    void clear();                               ///< Sets the data content to initial values
};

/** \brief  Command message protocol struct for external stack hardware. Version 0, Type SeCtrlState
 *
 * <b>stSeIsoStackState:</b>
 *
 * byte    | 0-39            | 40-47
 * :------:|:---------------:|:---------------:
 * content | stIsoStackState | stSeHardwareCtrl
 *
 * Back to: \ref Page_IsoStackCtrlProtocol
 */
struct stSeIsoStackState
{
    stIsoStackState         isoStackState;      ///< State from the iso stack.
    stSeHardwareCtrl        seHardwareCmd;      ///< Command of the SE hardware.

    stSeIsoStackState();                        ///< Initializer
    void clear();                               ///< Sets the data content to initial values
};

#pragma pack (pop)

}

#endif

/** @} */
