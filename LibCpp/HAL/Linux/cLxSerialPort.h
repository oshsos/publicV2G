#ifndef CLXSERIALPORT_H
#define CLXSERIALPORT_H

#include <atomic>
#include <thread>
#include <termios.h>

#include "../cSerialPort.h"
#include "../cDebug.h"

namespace LibCpp
{

class cLxSerialPort : public cSerialPort
{
public:
    cLxSerialPort(bool open = false, std::string serialPortName = "/dev/ttyS0", enBaud baud = enBaud::Bd4800, enBytes bytes = enBytes::b8, enParity parity = enParity::None, enStopBits stopBits = enStopBits::One, bool async = false, iFramePort* callback = nullptr);
    virtual ~cLxSerialPort();

    bool            open(std::string serialPortName, enBaud baud = enBaud::Bd4800, enBytes bytes = enBytes::b8, enParity parity = enParity::None, enStopBits stopBits = enStopBits::One, bool async = false, iFramePort* callback = nullptr);
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
    static speed_t  LxBaud(enBaud baud);    ///< Retrieves the Linux baud rate register constant

public:
    cDebug              dbg;                 ///< Logging interface.

private:
    struct termios      ttyConfig;           ///< Configuration struct for the serial interface
    int                 fd;                  ///< File descriptor of the serial port.
    char                message[LibCpp_cSerialPort_BUFFERSIZE]; ///< Message buffer of received messages.
    std::atomic<int>    messageLen;          ///< Length of the message stored in the message buffer (controls access between background process and main process.
    std::atomic<bool>   threadEnabled;       ///< Flag indicatin a started background process and to be used to friendly close the process.
    std::thread*        pThread;             ///< Class instance representing the background task.
    bool                async;               ///< Asynchronous call of callback objects
};

}

#endif
