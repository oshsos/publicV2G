// https://tldp.org/HOWTO/Serial-Programming-HOWTO/x115.html
// https://stackoverflow.com/questions/6947413/how-to-open-read-and-write-from-serial-port-in-c
// https://www.ing.iac.es/%7Edocs/external/serial/serial.pdf

#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "cLxSerialPort.h"

using namespace std;
using namespace LibCpp;

cLxSerialPort::cLxSerialPort(bool open, string serialPortName, enBaud baud, enBytes bytes, enParity parity, enStopBits stopBits, bool async, iFramePort* callback) :
    dbg("cLxSerialPort")
{
    fd = -1;
    messageLen = 0;
    threadEnabled = false;
    pThread = nullptr;
    if (open)
        this->open(serialPortName, baud, bytes, parity, stopBits, async, callback);
}

cLxSerialPort::~cLxSerialPort()
{
    cLxSerialPort::close();
}

void cLxSerialPort::close()
{
    threadEnabled = false;
    if (pThread)
    {
        if (pThread->joinable())
            pThread->join();
        delete pThread;
        pThread = nullptr;
    }
    if (fd >= 0)
        ::close(fd);
    fd = -1;
    messageLen = 0;
}

/**
 * @brief Prepares for closing by friendly closing the background task.
 * This way several objects requiring to finish a background task can do so in parallel.
 * Just calling 'close' requires much time as each 'close' needs to wait for the backgrond task actually
 * having finished.
 */
bool cLxSerialPort::prepareClose()
{
    threadEnabled = false;
    return !pThread;
}

bool cLxSerialPort::open(string serialPortName, enBaud baud, enBytes bytes, enParity parity, enStopBits stopBits, bool async, iFramePort* callback)
{
    cDebug dbg("open", &this->dbg);

    fd = ::open(serialPortName.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0)
    {
        dbg.printf(enDebugLevel_Fatal, "Serial interface %s could not be opened: %s!", serialPortName.c_str(), strerror(errno));
        return false;
    }
    // propably not necessary if cfmakeraw() is used
    // if (tcgetattr(fd, &tty) < 0) {
    //     printf("Error from tcgetattr: %s\n", strerror(errno));
    //     return -1;
    // }

    // setup for non-canonical mode
    cfmakeraw(&ttyConfig);
    // equals to:
    // tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    // tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);  // other source states: options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    // tty.c_oflag &= ~OPOST;

    cfsetospeed(&ttyConfig, LxBaud(baud));
    cfsetispeed(&ttyConfig, LxBaud(baud));

    ttyConfig.c_cflag &= ~CSIZE;                        // clear all size bits before writing to them by or operation.
    switch(bytes)
    {
    case enBytes::b7: ttyConfig.c_cflag |= CS7; break;  // 7-bit characters
    case enBytes::b8: ttyConfig.c_cflag |= CS8; break;  // 8-bit characters
    }
    switch(parity)
    {
    case enParity::None:
        ttyConfig.c_cflag &= ~PARENB;                   // no parity bit
        break;
    case enParity::Even:
        ttyConfig.c_cflag |= PARENB;                    // enable parity bit
        ttyConfig.c_cflag &= ~PARODD;                   // even parity (disable odd parity)
        break;
    case enParity::Odd:
        ttyConfig.c_cflag |= PARENB;                    // enable parity bit
        ttyConfig.c_cflag |= PARODD;                    // odd parity
        break;
    }
    switch(stopBits)
    {
    case enStopBits::One:
        ttyConfig.c_cflag &= ~CSTOPB; // 1 stop bit (instead of 2)
        break;
    case enStopBits::Two:
        ttyConfig.c_cflag |= CSTOPB;  // 2 stop bits
        break;
    }

    ttyConfig.c_cflag &= ~CRTSCTS;                      // no hardware flowcontrol
    ttyConfig.c_cflag |= (CLOCAL | CREAD);              // ignore modem controls

    /* fetch bytes as they become available */
    ttyConfig.c_cc[VMIN] = 0;
    ttyConfig.c_cc[VTIME] = 1;                          // Timeout of 100 ms

    if (tcsetattr(fd, TCSANOW, &ttyConfig) != 0) {
        dbg.printf(enDebugLevel_Fatal, "Serial interface attributes could not be set: %s!\n", strerror(errno));
        ::close(fd);
        fd = -1;
        return false;
    }

    tcflush(fd, TCIOFLUSH);   // Clears remaining data with input and ouput buffer.

    dbg.printf(enDebugLevel_Info, "Opened serial port %s.", serialPortName.c_str());

    if (callback) setCallback(callback);
    this->async = async;
    threadEnabled = true;     // To be fast enough in case a direct 'receive' follows after 'open'
    pThread = new std::thread(&cLxSerialPort::threadReceiving, this);

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
int cLxSerialPort::send(const char* message, int messageLen, enBlocking blockingMode)
{
    if (threadEnabled)
    {
        //Send the message back to client
        //tcflow(fd, TCOOFF);
        return ::write(fd, message, messageLen);
        //tcflow(fd, TCOON);
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
int cLxSerialPort::receive(char* buffer, int bufferSize, enBlocking blockingMode)
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
    return len;
}

/**
 * @brief Receives a message without copying from the receive buffer
 * In case the message is processed, call LibCpp::cLxUdpSocket::receiveAcknowledge to free the receive buffer.
 * @param ppBuffer
 * @param blockingMode blockingMode Not implemented, method is not blocking
 * @return Number of bytes available in the receive buffer.
 */
int cLxSerialPort::receiveBuffer(char** ppBuffer, enBlocking blockingMode)
{
    *ppBuffer = message;
    return messageLen;
}

/**
 * @brief Frees the receive Buffer
 * Method is to be called after calling LibCpp::cLxUdpSocket::receiveBuffer to enable the reception of further messages.
 */
void cLxSerialPort::receiveAcknowledge()
{
    if (messageLen>0 && !callbacks.size())              // do not access for writing, if the receiving task has control of the buffer
        messageLen = 0;
}

/**
 * @brief Method to be called cycelic to synchronously call registered callbacks
 * This type of operation mode is a polling method.
 */
void cLxSerialPort::operate()
{
    if (fd<0 || async)
        return;
    internalOperate();
}

/**
 * @brief Method to call registered callback objects
 * The method is called either by 'operate' or the background thread
 * depending of 'async' is unset or set.
 */
void cLxSerialPort::internalOperate()
{
    if (messageLen && callbacks.size())
    {
        for (iFramePort* call : callbacks)
            call->onFrameReceive((cFramePort*)this);
        messageLen = 0;
    }
}

void cLxSerialPort::threadReceiving()
{
    cDebug dbg("threadReceiving", &this->dbg);

    threadEnabled = true;
    int read_size = 1;

    while(threadEnabled)
    {
        // Receive the first byte of a message
        read_size = read(fd , message, LibCpp_cSerialPort_BUFFERSIZE);
        if (read_size>0)
        {   // Message reception has been started
            int message_size = read_size;
            ttyConfig.c_cc[VTIME] = 0;
            tcsetattr(fd, TCSANOW, &ttyConfig);
            while(read_size && message_size<LibCpp_cSerialPort_BUFFERSIZE)
            {
                usleep(10000);      // End of message time out
                read_size = read(fd , message + message_size, LibCpp_cSerialPort_BUFFERSIZE - message_size);
                message_size += read_size;
            }
            ttyConfig.c_cc[VTIME] = 1;
            tcsetattr(fd, TCSANOW, &ttyConfig);

            // Ensure zero termination of message
            if (message_size < LibCpp_cSerialPort_BUFFERSIZE)
                message[message_size] = 0;
            messageLen = message_size;
            if (async && callbacks.size())
                internalOperate();
            else
                while (messageLen > 0 && threadEnabled)  // wait for message buffer to get read
                    this_thread::sleep_for(chrono::milliseconds(1));
        }
    }
    return;
}

/**
 * @brief Retrieves the Linux baud rate register constant
 * @param baud
 * @return
 */
speed_t cLxSerialPort::LxBaud(enBaud baud)
{
    switch (baud)
    {
        case enBaud::Bd0: return B0;		/* hang up */
        case enBaud::Bd50: return B50;
        case enBaud::Bd75: return B75;
        case enBaud::Bd110: return B110;
        case enBaud::Bd134: return B134;
        case enBaud::Bd150: return B150;
        case enBaud::Bd200: return B200;
        case enBaud::Bd300: return B300;
        case enBaud::Bd600: return B600;
        case enBaud::Bd1200: return B1200;
        case enBaud::Bd1800: return B1800;
        case enBaud::Bd2400: return B2400;
        case enBaud::Bd4800: return B4800;
        case enBaud::Bd9600: return B9600;
        case enBaud::Bd19200: return B19200;
        case enBaud::Bd38400: return B38400;
        case enBaud::Bd57600: return B57600;
        case enBaud::Bd115200: return B115200;
        case enBaud::Bd230400: return B230400;
        case enBaud::Bd460800: return B460800;
        case enBaud::Bd500000: return B500000;
        case enBaud::Bd576000: return B576000;
        case enBaud::Bd921600: return B921600;
        case enBaud::Bd1000000: return B1000000;
        case enBaud::Bd1152000: return B1152000;
        case enBaud::Bd1500000: return B1500000;
        case enBaud::Bd2000000: return B2000000;
        case enBaud::Bd2500000: return B2500000;
        case enBaud::Bd3000000: return B3000000;
        case enBaud::Bd3500000: return B3500000;
        case enBaud::Bd4000000: return B4000000;
    };
    return (speed_t)0;
}
