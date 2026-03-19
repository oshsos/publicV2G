// UTF8 (ü) //
/**
\file   cModbusSlave.h

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2025-10-04

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_Modbus

@{
\class cModbusSlave

This class represents a Modbus slave device and handles the required communication protocoll.
The communication line can be a serial bus or a TCP Ethernet network.\n
The standard Modbus slave links to the application through a memory array for coils, parameters
and file transfer. The application also might implement a new Modbus slave class inheriting
from cModbusSlave and overriding the virtual handler methods for incomming requests.\n
The cModbusSlave class is a wrapping class around the 'nanomodbus' c-implemented Modbus
slave. A hardware independent class cTcpServer interfaces to the hardware. For each hardware
a TCP server implementation derived from cTcpServer is required. Implementations for Windows
and Linux are provided.\n
\n
The actual minimal code required to implement a Modbus client is pretty small.

\code
    cWinTcpServer       tcpServer;
    cModbusSlave        slave(&tcpServer, "127.0.0.1");

    uint16_t            slave_registers[10] = {0};

    void main()
    {
        slave.setRegisterMemory(slave_registers, 10);
        slave_registers[1] = 0x4567;

        while(true)
        {
            slave.operate();
        }
    }
\endcode

A complete running project requires the following files and libraries.

\code
    SOURCES += \
        ../LibCpp/HAL/Windows/cWinConsoleInput.cpp \
        ../LibCpp/HAL/Windows/cWinIp.cpp \
        ../LibCpp/HAL/Windows/cWinTcpServer.cpp \
        ../LibCpp/HAL/Windows/HW_Tools.cpp \
        ../LibCpp/HAL/Tools.cpp \
        ../LibCpp/HAL/cDebug.cpp \
        ../LibCpp/HAL/cFramePort.cpp \
        ../LibCpp/HAL/cIp.cpp \
        ../LibCpp/HAL/cTcpServer.cpp \
        ../LibCpp/Modbus/cModbusSlave.cpp \
        ../LibCpp/Modbus/nanomodbus.c \
        ../LibCpp/Modbus/platform.cpp \
        ../LibCpp/cTimer.cpp \
        main.cpp

    HEADERS += \
        ../LibCpp/HAL/Windows/cWinConsoleInput.h \
        ../LibCpp/HAL/Windows/cWinIp.h \
        ../LibCpp/HAL/Windows/cWinTcpServer.h \
        ../LibCpp/HAL/HW_Tools.h \
        ../LibCpp/HAL/Tools.h \
        ../LibCpp/HAL/cDebug.h \
        ../LibCpp/HAL/cFramePort.h \
        ../LibCpp/HAL/cIp.h \
        ../LibCpp/HAL/cTcpServer.h \
        ../LibCpp/Modbus/cModbusMaster.h \
        ../LibCpp/Modbus/nanomodbus.h \
        ../LibCpp/Modbus/platform.h \
        ../LibCpp/cTimer.h

    LIBS += -pthread
    LIBS += $$PWD/../LibCpp/HAL/Windows/iphlpapi.lib
    LIBS += $$PWD/../LibCpp/HAL/Windows/ws2_32.lib
\endcode

The example application code

\code
    #include <iostream>
    #include <unistd.h>

    #include "../LibCpp/HAL/cDebug.h"
    #include "../LibCpp/HAL/Tools.h"
    #include "../LibCpp/HAL/Windows/cWinTcpServer.h"
    #include "../LibCpp/HAL/Windows/cWinConsoleInput.h"

    #include "../LibCpp/Modbus/cModbusSlave.h"

    using namespace std;
    using namespace LibCpp;
    using namespace Modbus;

    #define COILS_ADDR_MAX 100
    #define REGS_ADDR_MAX  32
    #define FILE_SIZE_MAX  32

    cDebug              dbg("main", enDebugLevel_Debug);
    cWinConsoleInput    console;

    cWinTcpServer       tcpServer;
    cModbusSlave        slave(&tcpServer, "127.0.0.1");

    nmbs_bitfield       slave_coils = {0};
    uint16_t            slave_registers[REGS_ADDR_MAX] = {0};
    uint16_t            slave_file[FILE_SIZE_MAX] = {0};

    void printHelp()
    {
        printf_flush("'q' to finish\n");
        printf_flush("'h' to help\n");
    }

    int main()
    {
        printf_flush("Modbus TCP slave (server) test program\n");
        printHelp();

        dbg.setInstanceName("main");
        dbg.setDebugLevel(enDebugLevel_Debug);

        tcpServer.dbg.setInstanceName("tcpServer", &dbg);
        slave.dbg.setInstanceName("slave", &dbg);

        slave.setCoilMemory(&slave_coils, COILS_ADDR_MAX);
        slave.setRegisterMemory(slave_registers, REGS_ADDR_MAX);
        slave.setFileMemory(slave_file, FILE_SIZE_MAX);

        slave_registers[1] = 0x4567;

        while (1)
        {
            slave.operate();

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
            }
            usleep(10000);
        }

        tcpServer.close();

        return 0;
    }
\endcode

\n
*/

#ifndef CMODBUSSLAVE_H
#define CMODBUSSLAVE_H

#include "../HAL/cDebug.h"
#include "../HAL/cSerialPort.h"
#include "../HAL/cTcpServer.h"

#include "platform.h"

namespace Modbus
{

extern const char* vendorName;
extern const char* productCode;
extern const char* majorMinorRevision;
extern const char* extended1;
extern const char* extended2;

/**
 * @brief Class representing the Modbus slave
 * This class functions as a Modbus slave to answer all requensts requests to
 * this slaves. The slave might be connected to the communication line. This
 * might be RTU-slaves on the serial bus or TCP-slaves on Ethernet.
 * The communication lines are accessed through the LibCpp::HAL hardware abstraction layer.
 */
class cModbusSlave
{
public:    
    cModbusSlave();                                                                                    ///< Standard constructor.
    cModbusSlave(LibCpp::cSerialPort* pSerialPort, int rtuAddress);                                    ///< Constructor for serial communication lines.
    cModbusSlave(LibCpp::cTcpServer* pTcpServer, std::string interfaceOrAddress, int rtuAddress = 0, int modbusPort = MODBUSPORT);          ///< Constructor for TCP communication lines.
    virtual ~cModbusSlave();                                                                           ///< Destructor

    void setCommunicationPort(LibCpp::cTcpServer* pTcpServer, std::string interfaceOrAddress, int rtuAddress, int modbusPort = MODBUSPORT);                                          ///< @copybrief cModbusSlave::setCommunicationPort(LibCpp::cTcpClient*)
    void setCommunicationPort(LibCpp::cSerialPort* pSerialPort, int rtuAddress);                       ///< @copybrief cModbusSlave::setCommunicationPort(LibCpp::cSerialPort*)

    void setCoilMemory(nmbs_bitfield* coils, int size);
    void setRegisterMemory(uint16_t* registers, int size);
    void setFileMemory(uint16_t* fileBuffer, int size);

    void operate();                                                                                     ///< @copybrief cModbusSlave::operate

    virtual nmbs_error onReadCoils(uint16_t address, uint16_t quantity, nmbs_bitfield coils_out);                                   ///< Handler method for Modbus requests
    virtual nmbs_error onWriteMultipleCoils(uint16_t address, uint16_t quantity, const nmbs_bitfield coils);                        ///< Handler method for Modbus requests
    virtual nmbs_error onReadInputRegisters(uint16_t address, uint16_t quantity, uint16_t* registers_out);                          ///< Handler method for Modbus requests
    virtual nmbs_error onReadHoldingRegisters(uint16_t address, uint16_t quantity, uint16_t* registers_out);                        ///< Handler method for Modbus requests
    virtual nmbs_error onWriteMultipleRegisters(uint16_t address, uint16_t quantity, const uint16_t* registers);                    ///< Handler method for Modbus requests
    virtual nmbs_error onReadFileRecord(uint16_t file_number, uint16_t record_number, uint16_t* registers, uint16_t count);         ///< Handler method for Modbus requests
    virtual nmbs_error onWriteFileRecord(uint16_t file_number, uint16_t record_number, const uint16_t* registers, uint16_t count);  ///< Handler method for Modbus requests

    static  nmbs_error onReadDeviceIdentificationMap(nmbs_bitfield_256 map);                                                        ///< Handler method for Modbus requests
    static  nmbs_error onReadDeviceIdentification(uint8_t object_id, char buffer[NMBS_DEVICE_IDENTIFICATION_STRING_LENGTH]);        ///< Handler method for Modbus requests

protected:
    void setup();

public:
    LibCpp::cDebug              dbg;                ///< Instance for debug purposes

protected:
    cModbusHalAdapter           halAdapter;         ///< Interface class instance to nano modbus library
    nmbs_callbacks              callbacks;          ///< Struct of nano Modbus library callback functions

    nmbs_bitfield*              coils;              ///< Coil data memory block
    int                         coilSize;           ///< Coil data memory size
    uint16_t*                   registers;          ///< Register data memory
    int                         registerSize;       ///< Register data memory size
    uint16_t*                   file;               ///< File transfer data memory
    int                         fileSize;           ///< File data memory size
};

}
#endif
/** @} */
