// UTF8 (ü) //
/**
\file cModbusSlave.cpp

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2025-10-04

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_Modbus
@{

*/

#include "platform.h"
#include "cModbusSlave.h"

using namespace std;
using namespace LibCpp;
using namespace Modbus;

const char* Modbus::vendorName         = "Vendor name";        ///< Identification string
const char* Modbus::productCode        = "Product code";       ///< Identification string
const char* Modbus::majorMinorRevision = "1.0";                ///< Identification string
const char* Modbus::extended1          = "Extended 1";         ///< Identification string
const char* Modbus::extended2          = "Extended 2";         ///< Identification string

nmbs_error handle_read_coils(uint16_t address, uint16_t quantity, nmbs_bitfield coils_out, uint8_t unit_id, void* arg)
{
    cModbusHalAdapter* pHalAdapter = (cModbusHalAdapter*)arg;
    if (!pHalAdapter)
        return NMBS_EXCEPTION_SERVER_DEVICE_FAILURE;
    if (!pHalAdapter->pSlave)
        return NMBS_EXCEPTION_SERVER_DEVICE_FAILURE;
    if (unit_id != pHalAdapter->rtuAddress)
        return NMBS_ERROR_INVALID_UNIT_ID;

    return pHalAdapter->pSlave->onReadCoils(address, quantity, coils_out);
}

nmbs_error handle_write_multiple_coils(uint16_t address, uint16_t quantity, const nmbs_bitfield coils, uint8_t unit_id,
                                       void* arg) {
    cModbusHalAdapter* pHalAdapter = (cModbusHalAdapter*)arg;
    if (!pHalAdapter)
        return NMBS_EXCEPTION_SERVER_DEVICE_FAILURE;
    if (!pHalAdapter->pSlave)
        return NMBS_EXCEPTION_SERVER_DEVICE_FAILURE;
    if (unit_id != pHalAdapter->rtuAddress)
        return NMBS_ERROR_INVALID_UNIT_ID;

    return pHalAdapter->pSlave->onWriteMultipleCoils(address, quantity, coils);
}

nmbs_error handle_read_input_registers(uint16_t address, uint16_t quantity, uint16_t* registers_out, uint8_t unit_id,
                                       void* arg)
{
    cModbusHalAdapter* pHalAdapter = (cModbusHalAdapter*)arg;
    if (!pHalAdapter)
        return NMBS_EXCEPTION_SERVER_DEVICE_FAILURE;
    if (!pHalAdapter->pSlave)
        return NMBS_EXCEPTION_SERVER_DEVICE_FAILURE;
    if (unit_id != pHalAdapter->rtuAddress)
        return NMBS_ERROR_INVALID_UNIT_ID;

    return pHalAdapter->pSlave->onReadInputRegisters(address, quantity, registers_out);
}

nmbs_error handle_read_holding_registers(uint16_t address, uint16_t quantity, uint16_t* registers_out, uint8_t unit_id,
                                         void* arg) {
    cModbusHalAdapter* pHalAdapter = (cModbusHalAdapter*)arg;
    if (!pHalAdapter)
        return NMBS_EXCEPTION_SERVER_DEVICE_FAILURE;
    if (!pHalAdapter->pSlave)
        return NMBS_EXCEPTION_SERVER_DEVICE_FAILURE;
    if (unit_id != pHalAdapter->rtuAddress)
        return NMBS_ERROR_INVALID_UNIT_ID;

    return pHalAdapter->pSlave->onReadHoldingRegisters(address, quantity, registers_out);
}

nmbs_error handle_write_multiple_registers(uint16_t address, uint16_t quantity, const uint16_t* registers,
                                           uint8_t unit_id, void* arg) {
    cModbusHalAdapter* pHalAdapter = (cModbusHalAdapter*)arg;
    if (!pHalAdapter)
        return NMBS_EXCEPTION_SERVER_DEVICE_FAILURE;
    if (!pHalAdapter->pSlave)
        return NMBS_EXCEPTION_SERVER_DEVICE_FAILURE;
    if (unit_id != pHalAdapter->rtuAddress)
        return NMBS_ERROR_INVALID_UNIT_ID;

    return pHalAdapter->pSlave->onWriteMultipleRegisters(address, quantity, registers);
}

nmbs_error handle_read_file_record(uint16_t file_number, uint16_t record_number, uint16_t* registers, uint16_t count,
                                   uint8_t unit_id, void* arg) {
    cModbusHalAdapter* pHalAdapter = (cModbusHalAdapter*)arg;
    if (!pHalAdapter)
        return NMBS_EXCEPTION_SERVER_DEVICE_FAILURE;
    if (!pHalAdapter->pSlave)
        return NMBS_EXCEPTION_SERVER_DEVICE_FAILURE;
    if (unit_id != pHalAdapter->rtuAddress)
        return NMBS_ERROR_INVALID_UNIT_ID;

    return pHalAdapter->pSlave->onReadFileRecord(file_number, record_number, registers, count);
}

nmbs_error handle_write_file_record(uint16_t file_number, uint16_t record_number, const uint16_t* registers,
                                    uint16_t count, uint8_t unit_id, void* arg) {
    cModbusHalAdapter* pHalAdapter = (cModbusHalAdapter*)arg;
    if (!pHalAdapter)
        return NMBS_EXCEPTION_SERVER_DEVICE_FAILURE;
    if (!pHalAdapter->pSlave)
        return NMBS_EXCEPTION_SERVER_DEVICE_FAILURE;
    if (unit_id != pHalAdapter->rtuAddress)
        return NMBS_ERROR_INVALID_UNIT_ID;

    return pHalAdapter->pSlave->onWriteFileRecord(file_number, record_number, registers, count);
}

nmbs_error handle_read_device_identification_map(nmbs_bitfield_256 map) {
    return cModbusSlave::onReadDeviceIdentificationMap(map);
}

nmbs_error handle_read_device_identification(uint8_t object_id, char buffer[NMBS_DEVICE_IDENTIFICATION_STRING_LENGTH])
{
    return cModbusSlave::onReadDeviceIdentification(object_id, buffer);
}

/**
 * @brief Constructor
 */
cModbusSlave::cModbusSlave() :
    dbg("cModbusSlave")
{
    setup();
}

/**
 * @brief Constructor for serial port communication
 */
cModbusSlave::cModbusSlave(LibCpp::cSerialPort* pSerialPort, int rtuAddress) :
    dbg("cModbusSlave")
{
    setup();
    halAdapter.set(false, nullptr, nullptr, pSerialPort, "", rtuAddress, nullptr, 500, MODBUSPORT, &callbacks);
    halAdapter.pSlave = this;
    nmbs_set_platform_arg(&halAdapter.nmbs, &halAdapter);
    nmbs_set_read_timeout(&halAdapter.nmbs, 1000); // ???
}

/**
 * @brief Constructor for Ethernet communication
 * The ethernet communicaton may be combined with a paralles serial communication at the same master.
 */
cModbusSlave::cModbusSlave(LibCpp::cTcpServer* pTcpServer, string interfaceOrAddress, int rtuAddress, int modbusPort) :
    dbg("cModbusSlave")
{
    setup();
    halAdapter.set(false, pTcpServer, nullptr, nullptr, interfaceOrAddress, rtuAddress, nullptr, 2000, modbusPort, &callbacks);
    halAdapter.pSlave = this;
    nmbs_set_platform_arg(&halAdapter.nmbs, &halAdapter);
    nmbs_set_read_timeout(&halAdapter.nmbs, 1000); // ???
}

/**
 * @brief Destructor
 */
cModbusSlave::~cModbusSlave()
{
    if (halAdapter.pSerial)
        halAdapter.pSerial->close();
    if (halAdapter.pTcpServer)
        halAdapter.pTcpServer->close();
}

/**
 * @brief Sets the callback functions struct of the nano modbus interface
 */
void cModbusSlave::setup()
{
    coils = nullptr;
    coilSize = 0;
    registers = nullptr;
    registerSize = 0;
    file = nullptr;
    fileSize = 0;

    nmbs_callbacks_create(&callbacks);
    callbacks.read_coils = handle_read_coils;
    callbacks.write_multiple_coils = handle_write_multiple_coils;
    callbacks.read_holding_registers = handle_read_holding_registers;
    callbacks.write_multiple_registers = handle_write_multiple_registers;
    callbacks.read_input_registers = handle_read_input_registers;
    callbacks.read_file_record = handle_read_file_record;
    callbacks.write_file_record = handle_write_file_record;
    callbacks.read_device_identification_map = handle_read_device_identification_map;
    callbacks.read_device_identification = handle_read_device_identification;
    callbacks.arg = &halAdapter;
}

void cModbusSlave::setCommunicationPort(LibCpp::cTcpServer* pTcpServer, string interfaceOrAddress, int rtuAddress, int modbusPort)
{
    halAdapter.set(false, pTcpServer, nullptr, nullptr, interfaceOrAddress, rtuAddress, nullptr, modbusPort);
    halAdapter.pSlave = this;
    nmbs_set_platform_arg(&halAdapter.nmbs, &halAdapter);
    nmbs_set_read_timeout(&halAdapter.nmbs, 1000); // ???
}

void cModbusSlave::setCommunicationPort(LibCpp::cSerialPort* pSerialPort, int rtuAddress)
{
    halAdapter.set(false, nullptr, nullptr, pSerialPort, nullptr, rtuAddress, nullptr, MODBUSPORT);
    halAdapter.pSlave = this;
    nmbs_set_platform_arg(&halAdapter.nmbs, &halAdapter);
    nmbs_set_read_timeout(&halAdapter.nmbs, 1000); // ???
}

void cModbusSlave::setCoilMemory(nmbs_bitfield* coils, int size)
{
    this->coils = coils;
    this->coilSize = size;
}

void cModbusSlave::setRegisterMemory(uint16_t* registers, int size)
{
    this->registers = registers;
    this->registerSize = size;
}

void cModbusSlave::setFileMemory(uint16_t* fileBuffer, int size)
{
    this->file = fileBuffer;
    this->fileSize = size;
}

void cModbusSlave::operate()
{
    // Tcp server accept operation (wait for a client to connect)
    cConnectedClient* pNewClient = halAdapter.pTcpServer->newClient();
    if (pNewClient)
    {
        if (!halAdapter.pTcpConnection)
        {
            halAdapter.pTcpConnection = pNewClient;
            dbg.printf(enDebugLevel_Info, "Client connected to server from %s.", halAdapter.pTcpConnection->getIpAddressRemote().toString(true, true).c_str());
        }
        else
        {
            dbg.printf(enDebugLevel_Error, "Connection refused as a client is allready connected!");
            delete pNewClient;
        }
    }

    if (halAdapter.pTcpConnection)
    {
        nmbs_error err = nmbs_server_poll(&halAdapter.nmbs);
        if (err != NMBS_ERROR_NONE)
        {
            printf("Error on modbus connection - %s\n", nmbs_strerror(err));
            // In a more complete example, we would handle this error by checking its nmbs_error value
        }
        if (halAdapter.pTcpConnection) // The connected client might be destroyed by above called functions!
            halAdapter.pTcpConnection->operate();
    }
}

nmbs_error cModbusSlave::onReadCoils(uint16_t address, uint16_t quantity, nmbs_bitfield coils_out)
{
    if (coils)
    {
        if (address + quantity >= coilSize)
            return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;

        // Read our coils values into coils_out
        for (int i = 0; i < quantity; i++)
        {
            bool value = nmbs_bitfield_read(*coils, address + i);
            nmbs_bitfield_write(coils_out, i, value);
        }
        return NMBS_ERROR_NONE;
    }
    return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
}

nmbs_error cModbusSlave::onWriteMultipleCoils(uint16_t address, uint16_t quantity, const nmbs_bitfield coils)
{
    if (coils)
    {
        if (address + quantity >= coilSize)
            return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
        // Write coils values to our server_coils
        for (int i = 0; i < quantity; i++)
        {
            nmbs_bitfield_write(*this->coils, address + i, nmbs_bitfield_read(*this->coils, i));
        }
        return NMBS_ERROR_NONE;
    }
    return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
}

nmbs_error cModbusSlave::onReadInputRegisters(uint16_t address, uint16_t quantity, uint16_t* registers_out)
{
    if (registers)
    {
        if (address + quantity >= registerSize)
            return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
        // Read our registers values into registers_out
        for (int i = 0; i < quantity; i++)
            registers_out[i] = registers[address + i];
        return NMBS_ERROR_NONE;
    }
    return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
}

nmbs_error cModbusSlave::onReadHoldingRegisters(uint16_t address, uint16_t quantity, uint16_t* registers_out)
{
    if (registers)
    {
        if (address + quantity >= registerSize)
            return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
        // Read our registers values into registers_out
        for (int i = 0; i < quantity; i++)
            registers_out[i] = registers[address + i];
        return NMBS_ERROR_NONE;
    }
    return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
}

nmbs_error cModbusSlave::onWriteMultipleRegisters(uint16_t address, uint16_t quantity, const uint16_t* registers)
{
    if (registers)
    {
        if (address + quantity >= registerSize)
            return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
        // Write registers values to our server_registers
        for (int i = 0; i < quantity; i++)
            this->registers[address + i] = registers[i];
        return NMBS_ERROR_NONE;
    }
    return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
}

nmbs_error cModbusSlave::onReadFileRecord(uint16_t file_number, uint16_t record_number, uint16_t* registers, uint16_t count)
{
    if (file)
    {
        if (file_number != 1)
            return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
        if ((record_number + count) > fileSize)
            return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
        memcpy(registers, file + record_number, count * sizeof(uint16_t));
        return NMBS_ERROR_NONE;
    }
    return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
}

nmbs_error cModbusSlave::onWriteFileRecord(uint16_t file_number, uint16_t record_number, const uint16_t* registers, uint16_t count)
{
    if (file)
    {
        if (file_number != 1)
            return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
        if ((record_number + count) > fileSize)
            return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
        memcpy(file + record_number, registers, count * sizeof(uint16_t));
        return NMBS_ERROR_NONE;
    }
    return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
}

nmbs_error cModbusSlave::onReadDeviceIdentificationMap(nmbs_bitfield_256 map)
{
    // We support basic object ID and a couple of extended ones
    nmbs_bitfield_set(map, 0x00);
    nmbs_bitfield_set(map, 0x01);
    nmbs_bitfield_set(map, 0x02);
    nmbs_bitfield_set(map, 0x90);
    nmbs_bitfield_set(map, 0xA0);
    return NMBS_ERROR_NONE;
}

nmbs_error cModbusSlave::onReadDeviceIdentification(uint8_t object_id, char buffer[NMBS_DEVICE_IDENTIFICATION_STRING_LENGTH])
{
    switch (object_id) {
    case 0x00:
        strcpy(buffer, vendorName);
        break;
    case 0x01:
        strcpy(buffer, productCode);
        break;
    case 0x02:
        strcpy(buffer, majorMinorRevision);
        break;
    case 0x90:
        strcpy(buffer, extended1);
        break;
    case 0xA0:
        strcpy(buffer, extended2);
        break;
    default:
        return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
    }
    return NMBS_ERROR_NONE;
}

/** @} */
