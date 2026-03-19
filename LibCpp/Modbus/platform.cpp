// UTF8 (ü) //
/**
\file platform.cpp

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2025-07-18

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_Modbus
@{

*/
#include "platform.h"

#include "../HAL/cDebug.h"
#include "../cTimer.h"
#include "../HAL/HW_Tools.h"

using namespace std;
using namespace LibCpp;
using namespace Modbus;

cDebug dbgModbus("nanoModbus");

/**
 * @brief Read function to be implemented for the nanoModbus stack as hardware interface
 * @param buf
 * @param count
 * @param timeout_ms
 * @param arg
 * @return
 */
int32_t  __attribute__((cdecl)) nanoModbusRead(uint8_t* buf, uint16_t count, int32_t timeout_ms, void* arg)
{
    cModbusHalAdapter* pHal = (cModbusHalAdapter*)arg;
    if (!pHal)
        return 0;
    cFramePort* pPort = pHal->getFramePort();
    if (!pPort)
        return 0;

    int readLen = 0;
    char* pBuffer = nullptr;

    if (pHal->pSerial)
    {
        if (timeout_ms == 0)
        {   // Flush the receive buffer
            while (pPort->receive((char*)buf, count)) {LibCpp::sleep(1);};
            return 0;
        }
    }

    readLen = pPort->receiveBuffer(&pBuffer);

//    if (readLen < 0)
//    {
//        dbgModbus.printf(LibCpp::enDebugLevel_Error, "Slave disconnected!\n");
//        return 0;
//    }

    if (timeout_ms > 0 && pHal->isMaster)
    {
        cTimer timeoutTimer(timeout_ms, true);
        while (!readLen && !timeoutTimer.elapsed())
        {
            readLen = pPort->receiveBuffer(&pBuffer);
            LibCpp::sleep(1);
        }
        if (!readLen)
        {
            dbgModbus.printf(LibCpp::enDebugLevel_Error, "No answer!\n");
            pHal->pTcpClient->close();
            return 0;
        }
    }
    else
    {
        if (readLen < 0 && pHal->pTcpConnection)
        {
            dbgModbus.printf(LibCpp::enDebugLevel_Info, "Disconnection request.\n");
            delete pHal->pTcpConnection;
            pHal->pTcpConnection = nullptr;
            return 0; // -1 für cNmbsPortAdapter
        }
    }

    // If count equals the maximum buffer size (260 is hard coded in nanomodbus.h), it means to receive all still available bytes.
    //    if (count == 260)
    //    {
    //        count = readLen;
    //    }

    //if ((pHal->bufferReadIndex + count) > readLen || (!count && !readLen))
    if ((pHal->bufferReadIndex + count) > readLen)
    {
        if (pHal->bufferReadIndex)
            dbgModbus.printf(LibCpp::enDebugLevel_Error, "No enough bytes!\n");
        return 0;
    }

    memcpy(buf, pBuffer + pHal->bufferReadIndex, count);

    pHal->bufferReadIndex += count;

    //printf_flush("Received %s\n", ByteArrayToString(buf, count).c_str());

    if (readLen == pHal->bufferReadIndex)
    {
        pPort->receiveAcknowledge();
        pHal->bufferReadIndex = 0;
    }

    return count;
}

/**
 * @brief Read function to be implemented for the nanoModbus stack as hardware interface
 * @param buf
 * @param count
 * @param timeout_ms
 * @param arg
 * @return
 */
int32_t __attribute__((cdecl)) nanoModbusWrite(const uint8_t* buf, uint16_t count, int32_t timeout_ms, void* arg)
{
    (void)timeout_ms;

    cModbusHalAdapter* pHal = (cModbusHalAdapter*)arg;
    if (!pHal)
        return 0;
    cFramePort* pPort = pHal->getFramePort();
    if (!pPort)
        return 0;
    (void)arg;

    //printf_flush("Sending: %s\n", ByteArrayToString(buf, count).c_str());
    pPort->send((char*)buf, count);
    return count;
}

cModbusHalAdapter::cModbusHalAdapter() :
    dbg("cModbusHalAdapter")
{
}

cModbusHalAdapter::cModbusHalAdapter(bool asMaster, cTcpServer* pTcpServer, cTcpClient* pTcpClient, cSerialPort* pSerial, string ipAddress, int rtuAddress, cTcpClient*  pExclusiveClient, int connectionTimeout, int modbusPort, nmbs_callbacks* callbacks) :
    dbg("cModbusHalAdapter")
{
    set(asMaster, pTcpServer, pTcpClient, pSerial, ipAddress, rtuAddress, pExclusiveClient, connectionTimeout, modbusPort);
}

void cModbusHalAdapter::set(bool asMaster, cTcpServer* pTcpServer, cTcpClient* pTcpClient, cSerialPort* pSerial, string ipAddress, int rtuAddress, cTcpClient*  pExclusiveClient, int connectionTimeout, int modbusPort, nmbs_callbacks* callbacks)
{
    cTcpClient* pUsedClient = pExclusiveClient;
    if (!pUsedClient)
        pUsedClient = pTcpClient;
    this->pSerial = pSerial;
    this->pTcpClient = pUsedClient;
    this->pTcpServer = pTcpServer;
    pTcpConnection = nullptr;
    bufferReadIndex = 0;
    this->ipAddress = ipAddress;
    this->rtuAddress = rtuAddress;
    this->isMaster = asMaster;
    this->pTcpConnection = nullptr;

    this->success = configureNmbs(callbacks);

    if (this->pTcpClient)
    {
        this->pTcpClient->open(modbusPort, ipAddress);

        cTimer timeout(connectionTimeout, true);
        while (!this->pTcpClient->isConnected() && !timeout.elapsed())
        {
            LibCpp::sleep(10);
        }
    }
    else if (this->pTcpServer)
    {
        pTcpServer->open(modbusPort, ipAddress);
    }
}

/**
 * @brief Sets the required structs for nano modbus library communication functions
 * @return Success
 */
bool cModbusHalAdapter::configureNmbs(nmbs_callbacks* callbacks)
{
    cDebug dbg("configureNmbs", &this->dbg);

    nmbs_platform_conf_create(&platform_conf);
    if (pTcpClient || pTcpServer)
        platform_conf.transport = NMBS_TRANSPORT_TCP;
    else if (pSerial)
        platform_conf.transport = NMBS_TRANSPORT_RTU;
    else
    {
        dbg.printf(LibCpp::enDebugLevel_Fatal, "Modbus communication prepared without any communication port.\n");
        return false;
    }
    platform_conf.read = nanoModbusRead;
    platform_conf.write = nanoModbusWrite;
    platform_conf.arg = this;

    if (isMaster)
    {
        // Create the modbus master (as TCP client)
        nmbs_error err = nmbs_client_create(&nmbs, &platform_conf);
        if (err != NMBS_ERROR_NONE)
        {
            if (!nmbs_error_is_exception(err))
                dbg.printf(LibCpp::enDebugLevel_Fatal, "Modbus exception occourred during creating modbus client.\n");
            else
                dbg.printf(LibCpp::enDebugLevel_Fatal, "Error creating modbus client.\n");
            return false;
        }
    }
    else
    {
//        nmbs_callbacks callbacks;
//        nmbs_callbacks_create(&callbacks);
//        callbacks.read_coils = handle_read_coils;
//        callbacks.write_multiple_coils = handle_write_multiple_coils;
//        callbacks.read_holding_registers = handle_read_holding_registers;
//        callbacks.write_multiple_registers = handle_write_multiple_registers;
//        callbacks.read_input_registers = handle_read_input_registers;
//        callbacks.read_file_record = handle_read_file_record;
//        callbacks.write_file_record = handle_write_file_record;
//        callbacks.read_device_identification_map = handle_read_device_identification_map;
//        callbacks.read_device_identification = handle_read_device_identification;

        // Create the modbus slave (as TCP server). It's ok to set address_rtu to 0 since we are on TCP
        nmbs_error err = nmbs_server_create(&nmbs, 0, &platform_conf, callbacks);
        if (err != NMBS_ERROR_NONE)
        {
            dbg.printf(LibCpp::enDebugLevel_Fatal, "Error creating modbus server.\n");
            return false;
        }
    }

    if (pTcpClient || pTcpServer)
        nmbs_set_read_timeout(&nmbs, 2000);
    else
        nmbs_set_read_timeout(&nmbs, 500);
    return true;
}

/**
 * @brief Deliveres the cFramePort instance to be used for sending or receiving messages.
 * @return
 */
cFramePort* cModbusHalAdapter::getFramePort()
{
    if (pSerial) return pSerial;
    else if (pTcpClient) return pTcpClient;
    else return pTcpConnection;
}
