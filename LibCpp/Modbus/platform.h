// UTF8 (ü) //
/**
\file   platform.h

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2025-07-18

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_Modbus

The designer of the nanoModbus library (https://github.com/debevv/nanoMODBUS)intended
this file to contain the application specific hardware layer used by the nanoModbus
implementation.\n
\n
Therefore this file contains the adaption from the nanoModbus HAL to the LibCpp specific
HAL. The adaption is based on the class cModbusHalAdapter containing members holding
information about controlling the hardware through the independent interface.\n
\n
As the nanoModbus implementation is designed to operate with a hardware interface
designed in a 'object oriented' c-language style, a pointer to this HAL interfacing
class is passed through the 'arg'-Argument of the c-functions.

@{
*/

#ifndef PLATFORM_H
#define PLATFORM_H

//#define MODBUSPORT 502
#define MODBUSPORT 55100

#include "../HAL/cSerialPort.h"
#include "../HAL/cTcpClient.h"
#include "../HAL/cTcpServer.h"

#include "nanomodbus.h"

extern "C"
{
int32_t nanoModbusRead(uint8_t* buf, uint16_t count, int32_t timeout_ms, void* arg);                ///< HAL dependend read function of the nanoModbul library.
int32_t nanoModbusWrite(const uint8_t* buf, uint16_t count, int32_t timeout_ms, void* arg);         ///< HAL dependend write function of the nanoModbul library.
}

namespace Modbus
{
class cModbusSlave;

/**
 * @brief Class the nanoModbus library points to as hardware abstraction instance
 * This class is used to handle read and write operations by the corresponding
 * nanoModbus library c-functions. The 'arg' parameter points to an instance of
 * this class.
 */
class cModbusHalAdapter
{
public:
    cModbusHalAdapter();
    cModbusHalAdapter(bool asMaster, LibCpp::cTcpServer* pTcpServer, LibCpp::cTcpClient* pTcpClient, LibCpp::cSerialPort* pSerial, std::string ipAddress, int rtuAddress = 0, LibCpp::cTcpClient*  pExclusiveClient = nullptr, int connectionTimeout = 5000, int modbusPort = MODBUSPORT, nmbs_callbacks* callbacks = nullptr);  ///< @copybrief cModbusHalAdapter::cModbusHalAdapter

    void set(bool asMaster, LibCpp::cTcpServer* pTcpServer, LibCpp::cTcpClient* pTcpClient, LibCpp::cSerialPort* pSerial, std::string ipAddress, int rtuAddress = 0, LibCpp::cTcpClient*  pExclusiveClient = nullptr, int connectionTimeout = 5000, int modbusPort = MODBUSPORT, nmbs_callbacks* callbacks = nullptr);  ///< @copybrief cModbusHalAdapter::cModbusHalAdapter

    LibCpp::cFramePort*         getFramePort();                                                     ///< @copybrief cModbusHalAdapter::getFramePort
    bool                        configureNmbs(nmbs_callbacks* callbacks);                                                    ///< @copybrief cModbusHalAdapter::configureNmbs

public:
    LibCpp::cDebug              dbg;                ///< Debug output instance.
    LibCpp::cSerialPort*        pSerial;            ///< Pointer to the used hardware class instance.
    LibCpp::cTcpClient*         pTcpClient;         ///< Pointer to the used hardware class instance.
    LibCpp::cConnectedClient*   pTcpConnection;     ///< Pointer to the used hardware class instance.
    LibCpp::cTcpServer*         pTcpServer;         ///< Pointer to the used hardware class instance.
    int                         bufferReadIndex;    ///< Index of bytes already being read from the port buffer.
    std::string                 ipAddress;          ///< Slave IP Address
    int                         rtuAddress;         ///< Address of serial communication line

    nmbs_platform_conf          platform_conf;      ///< Platform description struct to be used for communication through the nano Modbus library.
    nmbs_t                      nmbs;               ///< Slave information struct to be used for communication through the nano Modbus library.
    bool                        isMaster;           ///< Indicates a master functionality (required for serial port operation)
    cModbusSlave*               pSlave;             ///< Pointer to the Modbus slave instance.

    bool                        success;            ///< Flag to indicate a successful instance creation.
};

}
#endif
/** @} */
