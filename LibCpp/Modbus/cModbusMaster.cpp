// UTF8 (ü) //
/**
\file cModbusMaster.cpp

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2025-07-21

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_Modbus
@{

*/


#include "platform.h"
#include "cModbusMaster.h"

using namespace std;
using namespace LibCpp;
using namespace Modbus;

/**
 * @brief Constructor
 */
cModbusMaster::cModbusMaster() :
    dbg("cModbusMaster")
{
    pTcpClient = nullptr;
    pSerialPort = nullptr;
    modbusPort = 502;
}

/**
 * @brief Constructor for serial port communication
 */
cModbusMaster::cModbusMaster(LibCpp::cSerialPort* pSerialPort) :
    dbg("cModbusMaster")
{
    this->pTcpClient = nullptr;
    this->pSerialPort = pSerialPort;
    modbusPort = 502;
}

/**
 * @brief Constructor for Ethernet communication
 * The ethernet communicaton may be combined with a paralles serial communication at the same master.
 */
cModbusMaster::cModbusMaster(LibCpp::cTcpClient* pTcpClient, LibCpp::cSerialPort* pSerialPort) :
    dbg("cModbusMaster")
{
    this->pTcpClient = pTcpClient;
    this->pSerialPort = pSerialPort;
    modbusPort = MODBUSPORT;
}

/**
 * @brief Destructor
 */
cModbusMaster::~cModbusMaster()
{
    if (pSerialPort)
        pSerialPort->close();
    if (pTcpClient)
        pTcpClient->close();
}

void cModbusMaster::setCommunicationPort(LibCpp::cTcpClient* pTcpClient)
{
    this->pTcpClient = pTcpClient;
}

void cModbusMaster::setCommunicationPort(LibCpp::cSerialPort* pSerialPort)
{
    this->pSerialPort = pSerialPort;
}

void cModbusMaster::setEthernetPort(int portNumber)
{
    modbusPort = portNumber;
}

bool cModbusMaster::checkBufferSize(cDebug* pDbg, int resultBufferByteSize, int registerQuantity)
{
    if (resultBufferByteSize < registerQuantity * 2)
    {
        pDbg->printf(LibCpp::enDebugLevel_Error, "Size of result buffer byte size does not fit 2 times register quantity!");
        return false;
    }
    return true;
}

nmbs_error cModbusMaster::readCoils(nmbs_bitfield coils, std::string ipAddress, int rtuAddress, int registerAddress, int registerQuantity,LibCpp::cTcpClient* pExclusivePort)
{
    cDebug dbg("readCoils", &this->dbg);

    cModbusHalAdapter halAdapter(true, nullptr, pTcpClient, pSerialPort, ipAddress, rtuAddress, pExclusivePort, modbusPort);
    if (!halAdapter.success)
        return NMBS_ERROR_INVALID_ARGUMENT;

    nmbs_error err = nmbs_read_coils(&halAdapter.nmbs, registerAddress, registerQuantity, coils);
    if (err != NMBS_ERROR_NONE)
    {
        if (!nmbs_error_is_exception(err))
            dbg.printf(LibCpp::enDebugLevel_Error, "Error reading coils at address %i - %s", registerAddress, nmbs_strerror(err));
        else
            dbg.printf(LibCpp::enDebugLevel_Debug, "Modbus exception while readomg coils at address %i - %s", registerAddress, nmbs_strerror(err));
        return err;
    }
    return NMBS_ERROR_NONE;
}

nmbs_error cModbusMaster::writeCoils(const nmbs_bitfield coils, std::string ipAddress, int rtuAddress, int registerAddress, int registerQuantity,LibCpp::cTcpClient* pExclusivePort)
{
    cDebug dbg("writeCoils", &this->dbg);

    cModbusHalAdapter halAdapter(true, nullptr, pTcpClient, pSerialPort, ipAddress, rtuAddress, pExclusivePort, modbusPort);
    if (!halAdapter.success)
        return NMBS_ERROR_INVALID_ARGUMENT;

    nmbs_error err = nmbs_write_multiple_coils(&halAdapter.nmbs, registerAddress, registerQuantity, coils);
    if (err != NMBS_ERROR_NONE)
    {
        if (!nmbs_error_is_exception(err))
            dbg.printf(LibCpp::enDebugLevel_Error, "Error writing coils at address %i - %s", registerAddress, nmbs_strerror(err));
        else
            dbg.printf(LibCpp::enDebugLevel_Debug, "Modbus exception while writing coils at address %i - %s", registerAddress, nmbs_strerror(err));
        return err;
    }
    return NMBS_ERROR_NONE;
}

/**
 * @brief Reads the addressed remote device input registers
 * @param resultBuffer Memory containing the reading result
 * @param resultBufferByteSize Memory size in bytes (a minimum of two times of the register quantity is required).
 * @param ipAddress IP Address in case an ethernet device is called.
 * @param rtuAddress Serial bus address in case not IP Address is given, unit id otherwise.
 * @param registerAddress Address of the register to be read from.
 * @param registerQuantity Number of subsequent registers to read from.
 * @param pExclusivePort In case of null pointer, the master internal TCP client recouce is used. Otherwise, another hardware dependet TCP client can be provide to avoid time consuming connection processes.
 * @return
 */
nmbs_error cModbusMaster::readInputRegisters(uint16_t* resultBuffer, int resultBufferByteSize, std::string ipAddress, int rtuAddress, int registerAddress, int registerQuantity, LibCpp::cTcpClient* pExclusivePort)
{
    cDebug dbg("readInputRegisters", &this->dbg);

    checkBufferSize(&dbg, resultBufferByteSize, registerQuantity);
    cModbusHalAdapter halAdapter(true, nullptr, pTcpClient, pSerialPort, ipAddress, rtuAddress, pExclusivePort, modbusPort);
    if (!halAdapter.success)
        return NMBS_ERROR_INVALID_ARGUMENT;

    nmbs_error err = nmbs_read_input_registers(&halAdapter.nmbs, registerAddress, registerQuantity, resultBuffer);
    if (err != NMBS_ERROR_NONE)
    {
        if (!nmbs_error_is_exception(err))
            dbg.printf(LibCpp::enDebugLevel_Error, "Error reading input register address %i - %s", registerAddress, nmbs_strerror(err));
        else
            dbg.printf(LibCpp::enDebugLevel_Debug, "Modbus exception while reading input register address %i - %s", registerAddress, nmbs_strerror(err));
        return err;
    }
    return NMBS_ERROR_NONE;
}

/**
 * @brief Reads the addressed remote device holding (output) registers
 * See cModbusMaster::readInputRegisters for parameter explanations.
 * @param resultBuffer
 * @param resultBufferByteSize
 * @param ipAddress
 * @param rtuAddress
 * @param registerAddress
 * @param registerQuantity
 * @param pExclusivePort
 * @return
 */
nmbs_error cModbusMaster::readHoldingRegisters(uint16_t* resultBuffer, int resultBufferByteSize, string ipAddress, int rtuAddress, int registerAddress, int registerQuantity, cTcpClient* pExclusivePort)
{
    cDebug dbg("readHoldingRegisters", &this->dbg);

    checkBufferSize(&dbg, resultBufferByteSize, registerQuantity);
    cModbusHalAdapter halAdapter(true, nullptr, pTcpClient, pSerialPort, ipAddress, rtuAddress, pExclusivePort, modbusPort);
    if (!halAdapter.success)
        return NMBS_ERROR_INVALID_ARGUMENT;

    nmbs_error err = nmbs_read_holding_registers(&halAdapter.nmbs, registerAddress, registerQuantity, resultBuffer);
    if (err != NMBS_ERROR_NONE)
    {
        if (!nmbs_error_is_exception(err))
            dbg.printf(LibCpp::enDebugLevel_Error, "Error reading holding register address %i - %s", registerAddress, nmbs_strerror(err));
        else
            dbg.printf(LibCpp::enDebugLevel_Debug, "Modbus exception while reading holding register address %i - %s", registerAddress, nmbs_strerror(err));
        return err;
    }
    return NMBS_ERROR_NONE;
}

/**
 * @brief Writes to the addressed remote device holding (output) registers
 * See cModbusMaster::readInputRegisters for parameter explanations.
 * @param dataBuffer
 * @param dataBufferByteSize
 * @param ipAddress
 * @param rtuAddress
 * @param registerAddress
 * @param registerQuantity
 * @param pExclusivePort
 * @return
 */
nmbs_error cModbusMaster::writeHoldingRegisters(const uint16_t* dataBuffer, int dataBufferByteSize, std::string ipAddress, int rtuAddress, int registerAddress, int registerQuantity, LibCpp::cTcpClient* pExclusivePort)
{
    cDebug dbg("writeHoldingRegisters", &this->dbg);

    checkBufferSize(&dbg, dataBufferByteSize, registerQuantity);
    cModbusHalAdapter halAdapter(true, nullptr, pTcpClient, pSerialPort, ipAddress, rtuAddress, pExclusivePort, modbusPort);
    if (!halAdapter.success)
        return NMBS_ERROR_INVALID_ARGUMENT;

    nmbs_error err = nmbs_write_multiple_registers(&halAdapter.nmbs, registerAddress, registerQuantity, dataBuffer);
    if (err != NMBS_ERROR_NONE) {
        if (!nmbs_error_is_exception(err))
            dbg.printf(LibCpp::enDebugLevel_Error, "Error writing holding register address %i - %s", registerAddress, nmbs_strerror(err));
        else
            dbg.printf(LibCpp::enDebugLevel_Debug, "Modbus exception while writing holding register address %i - %s", registerAddress, nmbs_strerror(err));
        return err;
    }
    return NMBS_ERROR_NONE;
}

nmbs_error cModbusMaster::readFileRecord(uint16_t* fileBuffer, uint16_t fileBufferByteSize, std::string ipAddress, int rtuAddress, uint16_t file_number, uint16_t record_number, LibCpp::cTcpClient* pExclusivePort)
{
    cDebug dbg("readFileRecord", &this->dbg);

    cModbusHalAdapter halAdapter(true, nullptr, pTcpClient, pSerialPort, ipAddress, rtuAddress, pExclusivePort, modbusPort);
    if (!halAdapter.success)
        return NMBS_ERROR_INVALID_ARGUMENT;

    nmbs_error err = nmbs_read_file_record(&halAdapter.nmbs, file_number, record_number, fileBuffer, fileBufferByteSize);
    if (err != NMBS_ERROR_NONE)
    {
        if (!nmbs_error_is_exception(err))
            dbg.printf(LibCpp::enDebugLevel_Error, "Error reading file record at filenumber %i, record number %i - %s", file_number, record_number, nmbs_strerror(err));
        else
            dbg.printf(LibCpp::enDebugLevel_Debug, "Modbus exception while reading file record at filenumber %i, record number %i - %s", file_number, record_number, nmbs_strerror(err));
        return err;
    }
    return NMBS_ERROR_NONE;
}

nmbs_error cModbusMaster::writeFileRecord(const uint16_t* fileBuffer, uint16_t fileBufferByteSize, std::string ipAddress, int rtuAddress, uint16_t file_number, uint16_t record_number, LibCpp::cTcpClient* pExclusivePort)
{
    cDebug dbg("writeFileRecord", &this->dbg);

    cModbusHalAdapter halAdapter(true, nullptr, pTcpClient, pSerialPort, ipAddress, rtuAddress, pExclusivePort, modbusPort);
    if (!halAdapter.success)
        return NMBS_ERROR_INVALID_ARGUMENT;

    nmbs_error err = nmbs_write_file_record(&halAdapter.nmbs, file_number, record_number, fileBuffer, fileBufferByteSize);
    if (err != NMBS_ERROR_NONE)
    {
        if (!nmbs_error_is_exception(err))
            dbg.printf(LibCpp::enDebugLevel_Error, "Error writing file record at filenumber %i, record number %i - %s", file_number, record_number, nmbs_strerror(err));
        else
            dbg.printf(LibCpp::enDebugLevel_Debug, "Modbus exception while writing file record at filenumber %i, record number %i - %s", file_number, record_number, nmbs_strerror(err));
        return err;
    }
    return NMBS_ERROR_NONE;
}

nmbs_error cModbusMaster::readDeviceIdentification(char vendor_name[128], char product_code[128], char major_minor_revision[128], std::string ipAddress, int rtuAddress, LibCpp::cTcpClient* pExclusivePort)
{
    cDebug dbg("writeFileRecord", &this->dbg);

    cModbusHalAdapter halAdapter(true, nullptr, pTcpClient, pSerialPort, ipAddress, rtuAddress, pExclusivePort, modbusPort);
    if (!halAdapter.success)
        return NMBS_ERROR_INVALID_ARGUMENT;

    nmbs_error err = nmbs_read_device_identification_basic(&halAdapter.nmbs, vendor_name, product_code, major_minor_revision, 128);
    if (err != NMBS_ERROR_NONE)
    {
        if (!nmbs_error_is_exception(err))
            dbg.printf(LibCpp::enDebugLevel_Error, "Error writing reading device identification - %s\n", nmbs_strerror(err));
        else
            dbg.printf(LibCpp::enDebugLevel_Debug, "Modbus exception while reading device identification - %s", nmbs_strerror(err));
        return err;
    }
    return NMBS_ERROR_NONE;
}

/**
 * @brief Standard constructor.
 */
cModbusRemoteClient::cModbusRemoteClient()
{
}

/**
 * @brief Constructor for serial communication lines.
 * @param pMaster
 * @param rtuAddress
 */
cModbusRemoteClient::cModbusRemoteClient(cModbusMaster* pMaster, int rtuAddress, LibCpp::cTcpClient* pExclusiveClient) :
    dbg("cModbusRemoteClient")
{
    setAddress(pMaster, ipAddress, rtuAddress, pExclusiveClient);
}

/**
 * @brief Constructor for TCP communication lines.
 * @param pMaster
 * @param ipAddress
 * @param rtuAddress
 */
cModbusRemoteClient::cModbusRemoteClient(cModbusMaster* pMaster, string ipAddress, int rtuAddress, LibCpp::cTcpClient* pExclusiveClient)
{
    setAddress(pMaster, ipAddress, rtuAddress, pExclusiveClient);
}

/**
 * @brief Destructor
 */
cModbusRemoteClient::~cModbusRemoteClient()
{
}

/**
 * @brief Instance initialization
 * This method is to be used after the instance is created by the standard constructor.
 * @param pMaster
 * @param ipAddress
 * @param rtuAddress
 */
void cModbusRemoteClient::setAddress(cModbusMaster* pMaster, std::string ipAddress, int rtuAddress, LibCpp::cTcpClient* pExclusiveClient)
{
    this->pMaster = pMaster;
    this->ipAddress = ipAddress;
    this->rtuAddress = rtuAddress;
    this->pExclusiveClient = pExclusiveClient;
}

/**
 * @brief Instance initialization
 * This method is to be used after the instance is created by the standard constructor.
 * @param pMaster
 * @param rtuAddress
 */
void cModbusRemoteClient::setAddress(cModbusMaster* pMaster, int rtuAddress, LibCpp::cTcpClient* pExclusiveClient)
{
    this->pMaster = pMaster;
    this->rtuAddress = rtuAddress;
    this->pExclusiveClient = pExclusiveClient;
}

nmbs_error cModbusRemoteClient::readCoils(nmbs_bitfield coils, int registerAddress, int registerQuantity)
{
    return pMaster->readCoils(coils, ipAddress, rtuAddress, registerAddress, registerQuantity, pExclusiveClient);
}

nmbs_error cModbusRemoteClient::writeCoils(const nmbs_bitfield coils, int registerAddress, int registerQuantity)
{
    return pMaster->writeCoils(coils, ipAddress, rtuAddress, registerAddress, registerQuantity, pExclusiveClient);
}

/**
 * @brief Reads the remote devices input registers
 * @param resultBuffer Memory containing the reading result
 * @param resultBufferByteSize Memory size in bytes (a minimum of two times of the register quantity is required).
 * @param registerAddress Address of the register to be read from.
 * @param registerQuantity Number of subsequent registers to read from.
 * @return
 */
nmbs_error cModbusRemoteClient::readInputRegisters(uint16_t* resultBuffer, int resultBufferByteSize, int registerAddress, int registerQuantity)
{
    return pMaster->readInputRegisters(resultBuffer, resultBufferByteSize, ipAddress, rtuAddress, registerAddress, registerQuantity, pExclusiveClient);
}

/**
 * @brief Reads the remote devices holding (output) registers
 * See cModbusRemoteClient::readInputRegisters for parameter explanations.
 * @param resultBuffer
 * @param resultBufferByteSize
 * @param ipAddress
 * @param rtuAddress
 * @param registerAddress
 * @param registerQuantity
 * @return
 */
nmbs_error cModbusRemoteClient::readHoldingRegisters(uint16_t* resultBuffer, int resultBufferByteSize, std::string ipAddress, int rtuAddress, int registerAddress, int registerQuantity)
{
    return pMaster->readHoldingRegisters(resultBuffer, resultBufferByteSize, ipAddress, rtuAddress, registerAddress, registerQuantity, pExclusiveClient);
}

/**
 * @brief Writes to the remote devices holding (output) registers
 * See cModbusRemoteClient::readInputRegisters for parameter explanations.
 * @param dataBuffer
 * @param dataBufferByteSize
 * @param registerAddress
 * @param registerQuantity
 * @return
 */
nmbs_error cModbusRemoteClient::writeHoldingRegisters(const uint16_t* dataBuffer, int dataBufferByteSize, std::string ipAddress, int rtuAddress, int registerAddress, int registerQuantity)
{
    return pMaster->writeHoldingRegisters(dataBuffer, dataBufferByteSize, ipAddress, rtuAddress, registerAddress, registerQuantity, pExclusiveClient);
}

nmbs_error cModbusRemoteClient::readFileRecord(uint16_t* fileBuffer, uint16_t fileBufferByteSize, std::string ipAddress, int rtuAddress, uint16_t file_number, uint16_t record_number)
{
    return pMaster->readFileRecord(fileBuffer, fileBufferByteSize, ipAddress, rtuAddress, file_number, record_number, pExclusiveClient);
}

nmbs_error cModbusRemoteClient::writeFileRecord(const uint16_t* fileBuffer, uint16_t fileBufferByteSize, std::string ipAddress, int rtuAddress, uint16_t file_number, uint16_t record_number)
{
    return pMaster->writeFileRecord(fileBuffer, fileBufferByteSize, ipAddress, rtuAddress, file_number, record_number, pExclusiveClient);
}

nmbs_error cModbusRemoteClient::readDeviceIdentification(char vendor_name[128], char product_code[128], char major_minor_revision[128], std::string ipAddress, int rtuAddress)
{
    return pMaster->readDeviceIdentification(vendor_name, product_code, major_minor_revision, ipAddress, rtuAddress, pExclusiveClient);
}


/** @} */
