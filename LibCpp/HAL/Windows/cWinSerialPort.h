#ifndef CWINSERIALPORT_H
#define CWINSERIALPORT_H

#include <windows.h>
#include <atomic>
#include <thread>

#include "../cSerialPort.h"
#include "../cDebug.h"
#include "../../cTimer.h"

namespace LibCpp
{

class cWinSerialPort : public cSerialPort
{
public:
    cWinSerialPort(bool open = false, std::string serialPortName = "COM1", enBaud baud = enBaud::Bd9600, enBytes bytes = enBytes::b8, enParity parity = enParity::None, enStopBits stopBits = enStopBits::One, int frameTimeout_us = 1000, bool async = false, iFramePort* callback = nullptr);
    virtual ~cWinSerialPort();

    bool            open(std::string serialPortName = "COM1", enBaud baud = enBaud::Bd9600, enBytes bytes = enBytes::b8, enParity parity = enParity::None, enStopBits stopBits = enStopBits::One, int frameTimeout_us = 1000, bool async = false, iFramePort* callback = nullptr);
    void            close();
    bool            prepareClose();

    int             send(const char* pMsg = nullptr, int messageLen = 0, enBlocking blockingMode = enBlocking_BUFFER);             ///> Sends a data frame.
    int             receive(char* pMsg = nullptr, int bufferSize = 0, enBlocking blockingMode = enBlocking_NONE);                  ///> Receives a data frame.
    int             receiveBuffer(char** ppMsg = nullptr, enBlocking blockingMode = enBlocking_NONE);                              ///> Receives a data frame without copying
    void            receiveAcknowledge();   ///> Acknowledges a reception of a message by 'receiveBuffer'
    void            operate();              ///> Cyclic operation on callback polling mode usage.

private:
    void            internalOperate();      ///< Handles the callback instances
    void            threadReceiving();      ///< Background receiving thread
    static DWORD    WinBaud(enBaud baud);   ///< Retrieves the Windows baud rate register constant

public:
    cDebug              dbg;                 ///< Logging interface.
    std::string         portName;            ///< Name of the port being opened.
    int                 frameTimeout_us;     ///< Time needing to be elapsed after the reception of a byte to identify the end of the frame.
    uint64_t            responseTime;        ///< Time elapsed between a send operation to the start of the next message reception
    uint64_t            receiveTime;         ///< Time elapsed between a send operation until the end of the next message reception

private:
    HANDLE              hSerial;             ///< File descriptor of the serial port.
    char                sendMessage[LibCpp_cSerialPort_BUFFERSIZE]; ///< Message buffer of messages to be sent by the background process (file operations are not thread save!).
    std::atomic<int>    sendMessageLen;      ///< Length of the message stored in the message buffer for send operations(controls access between background process and main process).
    char                message[LibCpp_cSerialPort_BUFFERSIZE]; ///< Message buffer of received messages.
    std::atomic<int>    messageLen;          ///< Length of the message stored in the message buffer (controls access between background process and main process.
    std::atomic<bool>   threadEnabled;       ///< Flag indicating a started background process and to be used to friendly close the process.
    std::atomic<bool>   threadRunning;       ///< Flag indicating a started background process is still running or has finished.
    std::thread*        pThread;             ///< Class instance representing the background task.
    bool                async;               ///< Asynchronous call of callback objects
    cTimer              tim;

    COMMTIMEOUTS        noTimeout;           ///< Immediately returns with zero or number of bytes received.

};

}

#endif
