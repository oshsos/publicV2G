#ifndef CSERIALPORT_H
#define CSERIALPORT_H

#ifndef LibCpp_cSerialPort_BUFFERSIZE
#define LibCpp_cSerialPort_BUFFERSIZE 1024
#endif

#include <string>
#include "cFramePort.h"

namespace LibCpp
{

class cSerialPort : public cFramePort
{
public:
    enum class enBaud
    {
        Bd0,		///< hang up
        Bd50,       ///< Baud rate
        Bd75,       ///< Baud rate
        Bd110,      ///< Baud rate
        Bd134,      ///< Baud rate
        Bd150,      ///< Baud rate
        Bd200,      ///< Baud rate
        Bd300,      ///< Baud rate
        Bd600,      ///< Baud rate
        Bd1200,     ///< Baud rate
        Bd1800,     ///< Baud rate
        Bd2400,     ///< Baud rate
        Bd4800,     ///< Baud rate
        Bd9600,     ///< Baud rate
        Bd19200,    ///< Baud rate
        Bd38400,    ///< Baud rate
        Bd57600,    ///< Baud rate
        Bd115200,   ///< Baud rate
        Bd230400,   ///< Baud rate
        Bd460800,   ///< Baud rate
        Bd500000,   ///< Baud rate
        Bd576000,   ///< Baud rate
        Bd921600,   ///< Baud rate
        Bd1000000,  ///< Baud rate
        Bd1152000,  ///< Baud rate
        Bd1500000,  ///< Baud rate
        Bd2000000,  ///< Baud rate
        Bd2500000,  ///< Baud rate
        Bd3000000,  ///< Baud rate
        Bd3500000,  ///< Baud rate
        Bd4000000   ///< Baud rate
    };
    enum class enBytes{
        b7,
        b8
    };
    enum class enParity{
        None,
        Even,
        Odd
    };
    enum class enStopBits{
        One,
        Two
    };

public:
    cSerialPort();
    virtual ~cSerialPort();

    virtual bool        open(std::string serialPortName, enBaud baud = enBaud::Bd9600, enBytes bytes = enBytes::b8, enParity parity = enParity::None, enStopBits stopBits = enStopBits::One, int frameTimeout_us = 1000, bool async = false, iFramePort* callback = nullptr);
    virtual void        close();
    virtual bool        prepareClose();

};

}

#endif
