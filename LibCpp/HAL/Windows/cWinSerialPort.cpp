// https://learn.microsoft.com/en-us/windows/win32/api/winbase/ns-winbase-commtimeouts
// https://stackoverflow.com/questions/15794422/serial-port-rs-232-connection-in-c

#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "../HW_Tools.h"
#include "../Tools.h"

#include "cWinSerialPort.h"

using namespace std;
using namespace LibCpp;

cWinSerialPort::cWinSerialPort(bool open, string serialPortName, enBaud baud, enBytes bytes, enParity parity, enStopBits stopBits, int frameTimeout_us, bool async, iFramePort* callback) :
    dbg("cWinSerialPort")
{
    hSerial = INVALID_HANDLE_VALUE;
    messageLen = 0;
    sendMessageLen = 0;
    threadEnabled = false;
    threadRunning = false;
    pThread = nullptr;
    if (open)
        this->open(serialPortName, baud, bytes, parity, stopBits, frameTimeout_us, async, callback);
}

cWinSerialPort::~cWinSerialPort()
{
    cWinSerialPort::close();
}

void cWinSerialPort::close()
{
    cDebug dbg("close", &this->dbg);

    threadEnabled = false;
    if (pThread)
    {
        if (pThread->joinable())
            pThread->join();
        delete pThread;
        pThread = nullptr;
    }
    if (hSerial != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hSerial);
        hSerial = INVALID_HANDLE_VALUE;
        dbg.printf(enDebugLevel_Info, "Closed serial port.");
    }
    messageLen = 0;
    sendMessageLen = 0;
    responseTime = 0;
    receiveTime = 0;
}

/**
 * @brief Prepares for closing by friendly closing the background task.
 * This way several objects requiring to finish a background task can do so in parallel.
 * Just calling 'close' requires much time as each 'close' needs to wait for the backgrond task actually
 * having finished.
 */
bool cWinSerialPort::prepareClose()
{
    threadEnabled = false;
    return !threadRunning;
}

bool cWinSerialPort::open(string serialPortName, enBaud baud, enBytes bytes, enParity parity, enStopBits stopBits, int frameTimeout_us, bool async, iFramePort* callback)
{
    cDebug dbg("open", &this->dbg);

    this->portName = serialPortName;
    this->frameTimeout_us = frameTimeout_us;

    cWinSerialPort::close();

    hSerial = CreateFileA(serialPortName.c_str(),
                         GENERIC_READ | GENERIC_WRITE,
                         0,
                         0,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL,
                         0);

    if(hSerial==INVALID_HANDLE_VALUE)
    {
        if(GetLastError()==ERROR_FILE_NOT_FOUND)
            dbg.printf(enDebugLevel_Fatal, "Serial interface %s is not available!", serialPortName.c_str());
        else
            dbg.printf(enDebugLevel_Fatal, "Error opening serial interface %s with code %i!", serialPortName.c_str(), GetLastError());
        return false;
            dbg.printf(enDebugLevel_Fatal, "Error opening serial interface %s: %s!", serialPortName.c_str(), GetErrorString(GetLastError()).c_str());
    }

    DCB serialParams = {};
    serialParams.DCBlength = sizeof(serialParams);
    GetCommState(hSerial, &serialParams);

    serialParams.BaudRate = WinBaud(baud);
    if (bytes == enBytes::b7)
        serialParams.ByteSize = 7;
    else
        serialParams.ByteSize = 8;

    if (parity == enParity::Even)
        serialParams.Parity = EVENPARITY;
    else if (parity == enParity::Odd)
        serialParams.Parity = ODDPARITY;
    else
        serialParams.Parity = NOPARITY;

    if (stopBits == enStopBits::Two)
        serialParams.StopBits = TWOSTOPBITS;
    else
        serialParams.StopBits = ONESTOPBIT;
    SetCommState(hSerial, &serialParams);

    // Set timeout for immediate return
    memset(&noTimeout, 0, sizeof(COMMTIMEOUTS));
    noTimeout.ReadIntervalTimeout = MAXDWORD ;
    noTimeout.ReadTotalTimeoutConstant = 0;
    noTimeout.ReadTotalTimeoutMultiplier = 0;
    noTimeout.WriteTotalTimeoutConstant = 0;
    noTimeout.WriteTotalTimeoutMultiplier = 0;

    SetCommTimeouts(hSerial, &noTimeout);

    PurgeComm(hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);

    dbg.printf(enDebugLevel_Info, "Opened serial port %s.", serialPortName.c_str());

    if (callback) setCallback(callback);
    this->async = async;
    threadEnabled = true;     // To be fast enough in case a direct 'receive' follows after 'open'
    threadRunning = true;
    pThread = new std::thread(&cWinSerialPort::threadReceiving, this);

    return true;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"

/**
 * @brief Sends a message to the serial port
 * @param message
 * @param messageLen
 * @param blockingMode
 * @return Len of actually sent bytes, -1 on failure (e.g. being unconnected).
 */
int cWinSerialPort::send(const char* message, int messageLen, enBlocking blockingMode)
{
    if (blockingMode != enBlocking_NONE)
    {
        while (sendMessageLen > 0 && threadEnabled)  // wait for send message buffer to become empty
            this_thread::sleep_for(chrono::milliseconds(1));
    }
    if (sendMessageLen<0 || !threadEnabled)
        return -1;
    if (sendMessageLen>0)
        return 0;

    if (threadEnabled)
    {
        if (messageLen > LibCpp_cSerialPort_BUFFERSIZE)
            messageLen = LibCpp_cSerialPort_BUFFERSIZE;
        memcpy(sendMessage, message, messageLen);
        sendMessageLen = messageLen;
        return messageLen;
    }
    return -1;
}

/**
 * @brief Receives a message
 * @param buffer
 * @param bufferSize
 * @param blockingMode Not implemented, method is not blocking
 * @return Number of bytes received.
 */
int cWinSerialPort::receive(char* buffer, int bufferSize, enBlocking blockingMode)
{
    cDebug dbg("receive", &this->dbg);

    int len = messageLen;
    if (bufferSize < len) len = bufferSize;
    if (len>0)
    {
        if (len>bufferSize) len=bufferSize;
        memcpy(buffer, message, len);
        if (len < bufferSize-1) buffer[len]=0;
        if (!callbacks.size())
            messageLen = 0;
    }
//    int len = messageLen;
//    if (len>0)
//    {
//        if (bufferSize < len)
//        {
//            memcpy(buffer, message, bufferSize);
//            memcpy(message, message + bufferSize, len + 1 - bufferSize);
//            messageLen = len - bufferSize;
//            len = bufferSize;
//        }
//        else
//        {
//            memcpy(buffer, message, len);
//            if (len < bufferSize-1) buffer[len]=0;
//            if (!callbacks.size())
//                messageLen = 0;
//        }
//    }

    return len;
}

/**
 * @brief Receives a message without copying from the receive buffer
 * In case the message is processed, call LibCpp::cLxUdpSocket::receiveAcknowledge to free the receive buffer.
 * @param ppBuffer
 * @param blockingMode blockingMode Not implemented, method is not blocking
 * @return Number of bytes available in the receive buffer.
 */
int cWinSerialPort::receiveBuffer(char** ppBuffer, enBlocking blockingMode)
{
    *ppBuffer = message;
    return messageLen;
}

/**
 * @brief Frees the receive Buffer
 * Method is to be called after calling LibCpp::cLxUdpSocket::receiveBuffer to enable the reception of further messages.
 */
void cWinSerialPort::receiveAcknowledge()
{
    if (messageLen>0 && !callbacks.size())              // do not access for writing, if the receiving task has control of the buffer
        messageLen = 0;
}

/**
 * @brief Method to be called cycelic to synchronously call registered callbacks
 * This type of operation mode is a polling method.
 */
void cWinSerialPort::operate()
{
    if (hSerial==INVALID_HANDLE_VALUE || async)
        return;
    internalOperate();
}

/**
 * @brief Method to call registered callback objects
 * The method is called either by 'operate' or the background thread
 * depending of 'async' is unset or set.
 */
void cWinSerialPort::internalOperate()
{
    if (messageLen && callbacks.size())
    {
        for (iFramePort* call : callbacks)
            call->onFrameReceive((cFramePort*)this);
        messageLen = 0;
    }
}

void cWinSerialPort::threadReceiving()
{
    cDebug dbg("threadReceiving", &this->dbg);

    threadEnabled = true;
    threadRunning = true;

    DWORD read_size = 0;

    while(threadEnabled)
    {
        // Check for messages to be sent
        if (sendMessageLen)
        {
            DWORD sentBytes = 0;
            //tcflow(fd, TCOOFF);
            WriteFile(hSerial, sendMessage, sendMessageLen, &sentBytes, 0);
            //tcflow(fd, TCOON);
            tim.start();
            sendMessageLen = 0;
        }
        // Receive the first byte of a message
        ReadFile(hSerial, message, LibCpp_cSerialPort_BUFFERSIZE, &read_size, 0);
        if (read_size>0)
        {   // Message frame reception has been started
            responseTime = tim.get();
            int message_size = read_size;
            while(read_size && message_size<LibCpp_cSerialPort_BUFFERSIZE)
            {
                usleep(frameTimeout_us);      // End of message time out
                ReadFile(hSerial, message + message_size, LibCpp_cSerialPort_BUFFERSIZE - message_size, &read_size, 0);
                message_size += read_size;
            }
            receiveTime = tim.get();
            // Ensure zero termination of message
            if (message_size < LibCpp_cSerialPort_BUFFERSIZE)
                message[message_size] = 0;
            messageLen = message_size;
            if (async && callbacks.size())
                internalOperate();
            else
                while (messageLen > 0 && threadEnabled)  // wait for message buffer to get read
                    usleep(frameTimeout_us);
        }
    }
    threadRunning = false;
    return;
}

/**
 * @brief Retrieves the Windows baud rate register constant
 * @param baud
 * @return
 */
DWORD cWinSerialPort::WinBaud(enBaud baud)
{
    switch (baud)
    {
        case enBaud::Bd0: return 0;		/* hang up */
        case enBaud::Bd50: return 50;
        case enBaud::Bd75: return 75;
        case enBaud::Bd110: return CBR_110;
        case enBaud::Bd134: return 134;
        case enBaud::Bd150: return 150;
        case enBaud::Bd200: return 200;
        case enBaud::Bd300: return CBR_300;
        case enBaud::Bd600: return CBR_600;
        case enBaud::Bd1200: return CBR_1200;
        case enBaud::Bd1800: return 1800;
        case enBaud::Bd2400: return CBR_2400;
        case enBaud::Bd4800: return CBR_4800;
        case enBaud::Bd9600: return CBR_9600;
        case enBaud::Bd19200: return CBR_19200;
        case enBaud::Bd38400: return CBR_38400;
        case enBaud::Bd57600: return CBR_57600;
        case enBaud::Bd115200: return CBR_115200;
        case enBaud::Bd230400: return 230400;
        case enBaud::Bd460800: return 460800;
        case enBaud::Bd500000: return 500000;
        case enBaud::Bd576000: return 576000;
        case enBaud::Bd921600: return 921600;
        case enBaud::Bd1000000: return 1000000;
        case enBaud::Bd1152000: return 1152000;
        case enBaud::Bd1500000: return 1500000;
        case enBaud::Bd2000000: return 2000000;
        case enBaud::Bd2500000: return 2500000;
        case enBaud::Bd3000000: return 3000000;
        case enBaud::Bd3500000: return 3500000;
        case enBaud::Bd4000000: return 4000000;
    };
    return 0;
}
