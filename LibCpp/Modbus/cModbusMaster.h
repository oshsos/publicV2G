// UTF8 (ü) //
/**
\file   cModbusMaster.h

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2025-07-21

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_Modbus
@{

\class cModbusMaster

This class functions as a Modbus master to initiate all requensts to slaves
connected to the communication line. This might be RTU-slaves on
the serial bus or TCP-slaves on the Ethernet. All slave requests are initiated
by the master handling the communication sequence and occurring timeouts.\n
The master holds and contolls the corresponding communication interface
through the LibCpp::HAL hardware abstraction layer.\n
A convenient way to address remote Modbus slaves connected to the communication
line is to instanziate a cModusRemoteClient class and linking this class to
the corresponding master. This class holds the address information thus
simplifying register access.\n
The folloing example shows the minimal code.

\code
    cWinTcpClient       tcpClient;
    cModbusMaster       master(&tcpClient);
    cModbusRemoteClient client(&master,"127.0.0.1");

    uint16_t            resultBuffer[1] = {};

    while (1)
    {
        tcpClient.operate();

        nmbs_error err = client.readInputRegisters(resultBuffer, sizeof(resultBuffer), 1, 1);

        if (err == NMBS_ERROR_NONE)
            uint16_t value = BigEndian(resultBuffer[0]);
    }
\endcode

An application requires the following modules to be included.

\code
SOURCES += \
        ../LibCpp/HAL/Windows/cWinConsoleInput.cpp \
        ../LibCpp/HAL/Windows/cWinIp.cpp \
        ../LibCpp/HAL/Windows/cWinTcpClient.cpp \
        ../LibCpp/HAL/Windows/HW_Tools.cpp \
        ../LibCpp/HAL/Tools.cpp \
        ../LibCpp/HAL/cDebug.cpp \
        ../LibCpp/HAL/cFramePort.cpp \
        ../LibCpp/HAL/cIp.cpp \
        ../LibCpp/HAL/cTcpClient.cpp \
        ../LibCpp/Modbus/cModbusMaster.cpp \
        ../LibCpp/Modbus/nanomodbus.c \
        ../LibCpp/Modbus/platform.cpp \
        ../LibCpp/cTimer.cpp \
        main.cpp

HEADERS += \
    ../LibCpp/HAL/Windows/cWinConsoleInput.h \
    ../LibCpp/HAL/Windows/cWinIp.h \
    ../LibCpp/HAL/Windows/cWinTcpClient.h \
    ../LibCpp/HAL/HW_Tools.h \
    ../LibCpp/HAL/Tools.h \
    ../LibCpp/HAL/cDebug.h \
    ../LibCpp/HAL/cFramePort.h \
    ../LibCpp/HAL/cIp.h \
    ../LibCpp/HAL/cTcpClient.h \
    ../LibCpp/Modbus/cModbusMaster.h \
    ../LibCpp/Modbus/nanomodbus.h \
    ../LibCpp/Modbus/platform.h \
    ../LibCpp/cTimer.h

LIBS += -pthread
LIBS += $$PWD/../LibCpp/HAL/Windows/iphlpapi.lib
LIBS += $$PWD/../LibCpp/HAL/Windows/ws2_32.lib
\endcode

An example application is given below.

\code
#include <iostream>
#include <unistd.h>

#include "../LibCpp/HAL/Windows/cWinTcpClient.h"
#include "../LibCpp/HAL/Windows/cWinConsoleInput.h"
#include "../LibCpp/HAL/cDebug.h"
#include "../LibCpp/HAL/Tools.h"
#include "../LibCpp/HAL/HW_Tools.h"

#include "../LibCpp/Modbus/cModbusMaster.h"

using namespace std;
using namespace LibCpp;
using namespace Modbus;

cDebug              dbg("main", enDebugLevel_Debug);
cWinConsoleInput    console;

cWinTcpClient       tcpClient;
cModbusMaster       master(&tcpClient);
cModbusRemoteClient client(&master,"127.0.0.1");

void printHelp()
{
    printf_flush("'q' to finish\n");
    printf_flush("'h' to help\n");
    printf_flush("'r' to read\n");
}

int main()
{
    dbg.setInstanceName("main");
    dbg.setDebugLevel(enDebugLevel_Debug);

    tcpClient.dbg.setInstanceName("tcpClient", &dbg);
    master.dbg.setInstanceName("master", &dbg);

    printf_flush("Modbus master TCP test program\n");
    printHelp();

    while (1)
    {
        tcpClient.operate();

        if (console.hasKey())
        {
            char input = console.getKey();
            printf_flush("\n");
            if (input == 'q')
                break;
            if (input == 'h')
            {
                printHelp();
            }
            if (input == 'r')
            {
                int registerAddress = 1;
                int registerQuantity = 1;
                uint16_t resultBuffer[1] = {};

                nmbs_error err = client.readInputRegisters(resultBuffer, sizeof(resultBuffer), registerAddress, registerQuantity);

                if (err == NMBS_ERROR_NONE)
                {
                    BigEndian(&resultBuffer[0]);
                    printf_flush("Reading result of address %i: %s\n", registerAddress, ByteArrayToString((char*)resultBuffer, sizeof(resultBuffer)).c_str());
                }
            }
        }
        LibCpp::sleep(10);
    }
    tcpClient.close();
    return 0;
}
\endcode

*/
#ifndef CMODBUSMASTER_H
#define CMODBUSMASTER_H

#include "../HAL/cDebug.h"
#include "../HAL/cSerialPort.h"
#include "../HAL/cTcpClient.h"
#include "nanomodbus.h"

namespace Modbus
{

/**
 * @brief Class representing a Modbus master
 */
class cModbusMaster
{
public:
    cModbusMaster();                                                                                    ///< Standard constructor.
    cModbusMaster(LibCpp::cSerialPort* pSerialPort);                                                    ///< Constructor for serial communication lines.
    cModbusMaster(LibCpp::cTcpClient* pTcpClient, LibCpp::cSerialPort* pSerialPort = nullptr);          ///< Constructor for TCP communication lines.
    virtual ~cModbusMaster();                                                                           ///< Destructor

    void setCommunicationPort(LibCpp::cTcpClient* pTcpClient);                                          ///< @copybrief cModbusMaster::setCommunicationPort(LibCpp::cTcpClient*)
    void setCommunicationPort(LibCpp::cSerialPort* pSerialPort);                                        ///< @copybrief cModbusMaster::setCommunicationPort(LibCpp::cSerialPort*)
    void setEthernetPort(int portNumber);                                                               ///< @copybrief cModbusMaster::setEthernetPort

    nmbs_error readCoils(nmbs_bitfield coils, std::string ipAddress, int rtuAddress, int registerAddress, int registerQuantity = 1, LibCpp::cTcpClient* pExclusivePort = nullptr);                                 ///< @copybrief cModbusMaster::readCoils
    nmbs_error writeCoils(const nmbs_bitfield coils, std::string ipAddress, int rtuAddress, int registerAddress, int registerQuantity = 1, LibCpp::cTcpClient* pExclusivePort = nullptr);                          ///< @copybrief cModbusMaster::writeCoils

    nmbs_error readInputRegisters(uint16_t* resultBuffer, int resultBufferByteSize, std::string ipAddress, int rtuAddress, int registerAddress, int registerQuantity = 1, LibCpp::cTcpClient* pExclusivePort = nullptr);         ///< @copybrief cModbusMaster::readInputRegisters
    nmbs_error readHoldingRegisters(uint16_t* resultBuffer, int resultBufferByteSize, std::string ipAddress, int rtuAddress, int registerAddress, int registerQuantity = 1, LibCpp::cTcpClient* pExclusivePort = nullptr);       ///< @copybrief cModbusMaster::readHodingRegisters
    nmbs_error writeHoldingRegisters(const uint16_t* dataBuffer, int dataBufferByteSize, std::string ipAddress, int rtuAddress, int registerAddress, int registerQuantity = 1, LibCpp::cTcpClient* pExclusivePort = nullptr);    ///< @copybrief cModbusMaster::readHodingRegisters

    nmbs_error readFileRecord(uint16_t* fileBuffer, uint16_t fileBufferByteSize, std::string ipAddress, int rtuAddress, uint16_t file_number, uint16_t record_number, LibCpp::cTcpClient* pExclusivePort = nullptr);                                         ///< @copybrief cModbusMaster::readFileRecord
    nmbs_error writeFileRecord(const uint16_t* fileBuffer, uint16_t fileBufferByteSize, std::string ipAddress, int rtuAddress, uint16_t file_number, uint16_t record_number, LibCpp::cTcpClient* pExclusivePort = nullptr);                                  ///< @copybrief cModbusMaster::writeFileRecord

    nmbs_error readDeviceIdentification(char vendor_name[128], char product_code[128], char major_minor_revision[128], std::string ipAddress, int rtuAddress = 0, LibCpp::cTcpClient* pExclusivePort = nullptr);                                                                  ///< @copybrief cModbusMaster::readDeviceIdentification

protected:
    bool       checkBufferSize(LibCpp::cDebug* pDbg, int resultBufferByteSize, int registerQuantity);   ///< @copybrief cModbusMaster::checkBufferSize

public:
    LibCpp::cDebug          dbg;                ///< Instance for debug purposes

protected:
    LibCpp::cTcpClient*     pTcpClient;         ///< Pointer to the hardware dependent TCP client instance being generally used for TCP communication to slaves (TCP-servers).
    LibCpp::cSerialPort*    pSerialPort;        ///< Pointer to the hardware dependent serial port instance.
    int                     modbusPort;         ///< Port to be used for Ethernet Modubus communication.
};

/**
 * @brief Class representing a remote Client on master side being connected to the communication line
 * This class basically carries address information to simplify the call of communication
 * methods within the application software.
 */
class cModbusRemoteClient
{
public:
    cModbusRemoteClient();                                                              ///< Standard constructor.
    cModbusRemoteClient(cModbusMaster* pMaster, int rtuAddress, LibCpp::cTcpClient* pExclusiveClient = nullptr);                        ///< Constructor for serial communication lines.
    cModbusRemoteClient(cModbusMaster* pMaster, std::string ipAddress, int rtuAddress = 0, LibCpp::cTcpClient* pExclusiveClient = nullptr); ///< Constructor for TCP communication lines.
    virtual ~cModbusRemoteClient();                                                     ///< Destructor

    void setAddress(cModbusMaster* pMaster, int rtuAddress, LibCpp::cTcpClient* pExclusiveClient = nullptr);                            ///< @copybrief cModbusRemoteClient::setCommunicationPort(cModbusMaster* pMaster, int rtuAddress))
    void setAddress(cModbusMaster* pMaster, std::string ipAddress, int rtuAddress, LibCpp::cTcpClient* pExclusiveClient = nullptr);     ///< @copybrief cModbusRemoteClient::setCommunicationPort(cModbusMaster* pMaster, std::string ipAddress, int rtuAddress))

    nmbs_error readCoils(nmbs_bitfield coils, int registerAddress, int registerQuantity = 1);                                           ///< @copybrief cModbusRemoteClient::readCoils
    nmbs_error writeCoils(const nmbs_bitfield coils, int registerAddress, int registerQuantity = 1);                                    ///< @copybrief cModbusRemoteClient::writeCoils

    nmbs_error readInputRegisters(uint16_t* resultBuffer, int resultBufferByteSize, int registerAddress, int registerQuantity = 1);                                                ///< @copybrief cModbusRemoteClient::readInputRegisters
    nmbs_error readHoldingRegisters(uint16_t* resultBuffer, int resultBufferByteSize, std::string ipAddress, int rtuAddress, int registerAddress, int registerQuantity = 1);       ///< @copybrief cModbusRemoteClient::readHodingRegisters
    nmbs_error writeHoldingRegisters(const uint16_t* dataBuffer, int dataBufferByteSize, std::string ipAddress, int rtuAddress, int registerAddress, int registerQuantity = 1);    ///< @copybrief cModbusRemoteClient::readHodingRegisters

    nmbs_error readFileRecord(uint16_t* fileBuffer, uint16_t fileBufferByteSize, std::string ipAddress, int rtuAddress, uint16_t file_number, uint16_t record_number);             ///< @copybrief cModbusRemoteClient::readFileRecord
    nmbs_error writeFileRecord(const uint16_t* fileBuffer, uint16_t fileBufferByteSize, std::string ipAddress, int rtuAddress, uint16_t file_number, uint16_t record_number);      ///< @copybrief cModbusRemoteClient::writeFileRecord

    nmbs_error readDeviceIdentification(char vendor_name[128], char product_code[128], char major_minor_revision[128], std::string ipAddress, int rtuAddress);                     ///< @copybrief cModbusRemoteClient::readDeviceIdentification

public:
    LibCpp::cDebug      dbg;                ///< Instance for debug purposes

protected:
    cModbusMaster*      pMaster;            ///< Modbus master instance to communicate to the slave
    std::string         ipAddress;          ///< IP address of the represented client
    int                 rtuAddress;         ///< Serial communication address or unit ID otherwise.
    LibCpp::cTcpClient* pExclusiveClient;   ///< Hardware dependent client to be exclusively used for this client
};

}
#endif
/** @} */
