#define COMTIMEOUT 500
#define CYCLE 100
#define RESPONSETIME 80
//#define RECONNECTTIMEOUT 1000
#define BUFFERSIZE 512
//#define PENDINGMESSAGESLIMIT 2

#include <cstring>

#include "../HAL/Tools.h"

#include "cVoltcraftSource.h"

using namespace std;
using namespace LibCpp;

cVoltcraftSource::stDevice::stDevice()
{
    current = 0.0;
    voltage = 0.0;
    power = 0.0;
}

void cVoltcraftSource::stDevice::fromResponseStringIdent(std::string cmdRes)
{
    cDebug dbg("cVoltcraftSource::stDevice::fromResponseStringIdent");
    vector<string> list = stringSplit(cmdRes, ",");
    if (list.size()==4)
    {
        manufacturer = list[0];
        type = list[1];
        serial = list[2];
        version = list[3];
    }
    else
        dbg.printf(enDebugLevel_Error, "IDN response malformed!");
}

cVoltcraftSource::stActualValues::stActualValues()
{
    online = false;
    enabled = false;
    current = 0.0;
    voltage = 0.0;
    power = 0.0;
}

void cVoltcraftSource::stActualValues::fromResponseString(std::string cmdRes)
{
    cDebug dbg("cVoltcraftSource::stActualValues::fromResponseString");
    vector<string> list = stringSplit(cmdRes, ",");
    if (list.size()==7)
    {
        voltage = atof(list[0].c_str());
        current = atof(list[1].c_str());
        power = atof(list[2].c_str());
        enabled = (list[6] == "0") ? false : true;
        online = true;
    }
    else
        dbg.printf(enDebugLevel_Error, "MEAS response malformed!");
}

string cVoltcraftSource::stActualValues::toString()
{
    char text[128] = "";
    sprintf_s(text, "VOLT %.2f V, CURR %.2f A, POW %.2f W, OUT %s, LINK %s", voltage, current, power, enabled ? "ON " : "OFF", online ? "ONLINE " : "OFFLINE");
    return text;
}

cVoltcraftSource::stDemandValues::stDemandValues()
{
    enable = false;
    voltage = 0.0;
    current = 0.0;
}

cVoltcraftSource::stLimitValues::stLimitValues()
{
    voltage = 0;
    current = 0;
}

cVoltcraftSource::cVoltcraftSource(cFramePort* pPort) :
    dbg("cVoltcraftSource"),
    comTimer(COMTIMEOUT)
{
    resetRequest = false;
    configurationRequest = false;
    presetFromDevice = false;
    currentFaktor = 1;
    voltageFaktor = 1;

    lastValidOutput = true;
    responseTime = 0;
    responseTimeMax = 0;
    rcvMsgCount = 0;

    pSerialPort = nullptr;
    this->pPort = pPort;

    state = enState::idle;
    device.voltage = 0;
    device.current = 0;
    device.power = 0;

    pPort->setCallback(this);
    runUpdate();
}

bool cVoltcraftSource::isInitialized()
{
    return ((state>=enState::awaitSetLimitCurrent) && (state<enState::failure));
};

void cVoltcraftSource::reset()
{
    resetRequest = true;
}

void cVoltcraftSource::configure()
{
    configurationRequest = true;
}

cVoltcraftSource::enStatus cVoltcraftSource::status()
{
    if (state < enState::awaitSetLimitVoltage)
        return enStatus::initializing;
    if (state >= enState::failure)
        return enStatus::failure;
    if (values.actual.enabled && values.actual.online)
        return enStatus::active;
    return enStatus::passive;
}

void cVoltcraftSource::send(const char* text)
{
    cDebug dbg("send", &this->dbg);

    int len = strlen(text);
    if (len>BUFFERSIZE)
    {
        dbg.printf(enDebugLevel_Error, "String length greater than %i characters!", BUFFERSIZE);
        return;
    }

    pPort->send(text, len);
    comTimer.start();

    char outBuffer[BUFFERSIZE] = "";
    strcpy_s(outBuffer, text);
    if ( state < enState::awaitMeasure)
        dbg.printf(enDebugLevel_Debug, "Sending: %s", stringTrim(outBuffer).c_str());
}

void cVoltcraftSource::operate(bool run)
{
    cDebug dbg("operate", &this->dbg);

    enState nextState = state;
    bool entered = state!=lastState;

    int result = comTimer.elapsed();

    if (result>0)
    {   // Time out
        comTimer.stop();
        values.actual.online = false;
        dbg.printf(enDebugLevel_Error, "Connection timeout!");
        nextState = enState::failure;
    }

    if (result<0 && comTimer.get()>=CYCLE)
    {   // Next send operation
        if (resetRequest)
        {
            resetRequest = false;
            nextState = enState::idle;
        }
        else if (configurationRequest)
        {
            configurationRequest = false;
            nextState = enState::awaitSetLimitVoltage;
        }
        else
        {
            switch(state)
            {
            case enState::idle:
                if (entered)
                {
                    values.actual.online = false;
                    responseTime = 0;
                    responseTimeMax = 0;
                    runUpdate();
                }
                if (run)
                    nextState = enState::awaitIdn;
                break;
            case enState::awaitIdn:
                if (entered)
                    send("*IDN?\n");
                else
                {
                    values.actual.online = true;
                    runUpdate();
                    nextState = enState::awaitOutput;
                }
                break;
            case enState::awaitOutput:
                if (entered)
                    send("OUTP?\n");
                else
                {
                    lastValidOutput = values.actual.enabled;
                    if (presetFromDevice)
                    {
                        values.demand.enable = values.actual.enabled;
                        nextState = enState::awaitDemandVoltage;
                    }
                    else
                        nextState = enState::awaitSetLimitVoltage;
                }
                break;
            case enState::awaitDemandVoltage:
                if (entered)
                    send("VOLT?\n");
                else
                    nextState = enState::awaitDemandCurrent;
                break;
            case enState::awaitDemandCurrent:
                if (entered)
                    send("CURR?\n");
                else
                    nextState = enState::awaitLimitVoltage;
                break;
            case enState::awaitLimitVoltage:
                if (entered)
                    send("VOLT:LIM?\n");
                else
                    nextState = enState::awaitLimitCurrent;
                break;
            case enState::awaitLimitCurrent:
                if (entered)
                    send("CURR:LIM?\n");
                else
                    nextState = enState::awaitMeasure;
                break;
            case enState::awaitSetLimitVoltage:
                if (entered)
                    send(stringFormat("VOLT:LIM %.2f V\n", values.limit.voltage).c_str());
                else
                    nextState = enState::awaitSetLimitCurrent;
                break;
            case enState::awaitSetLimitCurrent:
                if (entered)
                    send(stringFormat("CURR:LIM %.2f V\n", values.limit.current).c_str());
                else
                    nextState = enState::awaitMeasure;
                break;
            case enState::awaitMeasure:
                if (entered)
                    send("MEAS:ALL:INFO?\n");
                else
                {
                    runUpdate();
                    nextState = enState::awaitSetVoltage;
                }
                break;
            case enState::awaitSetVoltage:
                if (entered)
                    send(stringFormat("VOLT %.2f V\n", values.demand.voltage).c_str());
                else
                    nextState = enState::awaitSetCurrent;
                break;
            case enState::awaitSetCurrent:
                if (entered)
                    send(stringFormat("CURR %.2f V\n", values.demand.current).c_str());
                else
                    nextState = enState::awaitSetOutput;
                break;
            case enState::awaitSetOutput:
                if (entered)
                {
                    if (values.demand.enable != lastValidOutput)
                    {
                        send(stringFormat("OUTP %s\n", values.demand.enable ? "ON" : "OFF").c_str());
                        lastValidOutput = values.demand.enable;
                    }
                }
                else
                    nextState = enState::awaitMeasure;
                break;
            case enState::failure:
                if (entered)
                {
                    send("OUTP OFF");
                    values.actual.online = false;
                    runUpdate();
                }
                if (!run)
                    nextState = enState::idle;
                break;
            case enState::finished:
                if (entered)
                {
                    values.actual.online = false;
                    runUpdate();
                }
                nextState = enState::finished;
                break;
            default:
                nextState = enState::failure;
            }
        }
    }

    if (result == 0)
    {   // Check for response time delay being elapsed after sending not answered commands.
        switch (state)
        {
        case enState::awaitSetLimitVoltage:
        case enState::awaitSetLimitCurrent:
        case enState::awaitSetVoltage:
        case enState::awaitSetCurrent:
        case enState::awaitSetOutput:
            if (comTimer.get() > RESPONSETIME)
                comTimer.stop();
            break;
        default:;
        }
    }

    if (nextState != state && (nextState < enState::awaitMeasure || nextState >= enState::failure))
        dbg.printf(enDebugLevel_Debug, "Entering state %i.", (int)nextState);
    lastState = state;
    state = nextState;
}

void cVoltcraftSource::onFrameReceive(cFramePort* pPort)
{
    cDebug dbg("onFrameReceive", &this->dbg);
    char rcvBuffer[BUFFERSIZE] = "";
    int len = pPort->receive(rcvBuffer, BUFFERSIZE);

    if (len > 0)
    {
        responseTime = comTimer.get();
        comTimer.stop();
        if (responseTime > responseTimeMax) responseTimeMax = responseTime;
        values.actual.online = true;
        rcvMsgCount++;

        string rcvString = stringTrim(rcvBuffer);
        if (rcvString.empty())
            return;
        switch (state)
        {
        case enState::awaitIdn:
        {
            dbg.printf(enDebugLevel_Info, "IDN response: %s", rcvString.c_str());
            device.fromResponseStringIdent(rcvString);
            if (device.type.substr(0,3) == "DSP")
            {
                device.voltage = atof(device.type.substr(3,2).c_str());
                device.current = atof(device.type.substr(5,2).c_str());
                device.power = device.voltage * device.current;
            }
        } break;
        case enState::awaitOutput:
        {
            dbg.printf(enDebugLevel_Debug, "OUTP: %s", rcvString.c_str());
            values.actual.enabled = (rcvString == "ON") ? true : false;
        } break;
        case enState::awaitDemandVoltage:
        {
            dbg.printf(enDebugLevel_Debug, "VOLT: %s", rcvString.c_str());
            values.demand.voltage = atof(rcvString.c_str());
        } break;
        case enState::awaitDemandCurrent:
        {
            dbg.printf(enDebugLevel_Debug, "CURR: %s", rcvString.c_str());
            values.demand.current = atof(rcvString.c_str());
        } break;
        case enState::awaitLimitVoltage:
        {
            dbg.printf(enDebugLevel_Debug, "VOLT:LIM: %s", rcvString.c_str());
            values.limit.voltage = atof(rcvString.c_str());
        } break;
        case enState::awaitLimitCurrent:
        {
            dbg.printf(enDebugLevel_Debug, "CURR:LIM: %s", rcvString.c_str());
            values.limit.current = atof(rcvString.c_str());
        } break;
        case enState::awaitMeasure:
        {
            //dbg.printf(enDebugLevel_Debug, "MEAS: %s", rcvString.c_str());
            values.actual.fromResponseString(rcvString);
            // Disable output state demand value in case the power source switched off the output itself.
            if (values.actual.enabled != lastValidOutput)
            {
                lastValidOutput = values.actual.enabled;
                values.demand.enable = values.actual.enabled;
            }
        } break;
        default:;
        }
    }
    else if (len < 0)
    {
        dbg.printf(enDebugLevel_Error, "Communication interrupted!");
        reset();
    }
}

void cVoltcraftSource::openSerialPort(cSerialPort* pPort, std::string serialPortName, cSerialPort::enBaud baud)
{
    if (!pPort)
        return;

    this->pPort = pPort;
    pSerialPort = pPort;

    pPort->open(serialPortName.c_str(), baud, cSerialPort::enBytes::b8, cSerialPort::enParity::None, cSerialPort::enStopBits::One, 5000);
}

/**
 * @brief Setting demand values
 * @param demand
 */
void cVoltcraftSource::set(stPowerControl demand)
{
    values.demand.enable = demand.enable;
    values.demand.voltage = demand.voltage / voltageFaktor;
    values.demand.current = demand.current / currentFaktor;
}

/**
 * @brief Getting actual output values
 * @return
 */
stPowerControl cVoltcraftSource::get()
{
    return stPowerControl(values.actual.enabled, values.actual.voltage * voltageFaktor, values.actual.current * currentFaktor, isInitialized());
}

/**
 * @brief Returns the demanded output values (set point values)
 * @return
 */
stPowerControl cVoltcraftSource::getDemand()
{
    return stPowerControl(values.demand.enable, values.demand.voltage * voltageFaktor, values.demand.current * currentFaktor);
}

/**
 * @brief Used for synchronous state machine operation or communication
 */
void cVoltcraftSource::operate()
{
    operate(true);
}

/**
 * @brief Retrieves the actual output values having recieved from the last get() call
 * @return
 */
stPowerControl cVoltcraftSource::lastActual()
{
    return get();
}

/**
 * @brief Retrieves the desired (set point) values having set by the last get() call
 * @return
 */
stPowerControl cVoltcraftSource::lastDemand()
{
    return getDemand();
}
