#include "cSerialPort.h"

using namespace std;
using namespace LibCpp;

cSerialPort::cSerialPort()
{
}

cSerialPort::~cSerialPort()
{
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
bool cSerialPort::open(string serialPortName, enBaud baud, enBytes bytes, enParity parity, enStopBits stopBits, int frameTimeout, bool async, iFramePort* callback)
{
    return false;
}
#pragma GCC diagnostic warning "-Wunused-parameter"

void cSerialPort::close()
{

}

bool cSerialPort::prepareClose()
{
    return true;
}
