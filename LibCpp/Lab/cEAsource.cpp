#define _CRT_SECURE_NO_WARNINGS

#define COMTIMEOUT 4500
#define EASOURCETIMEOUT 6000
#define CYCLE 50
#define RESPONSETIME (5+5)
//#define RECONNECTTIMEOUT 1000
#define BUFFERSIZE 512
//#define PENDINGMESSAGESLIMIT 2

#include <cstring>

#include "../HAL/Tools.h"

#include "cEAsource.h"

using namespace std;
using namespace LibCpp;

cEAsource::stDevice::stDevice()
{
    current = 0.0;
    voltage = 0.0;
    power = 0.0;
}

void cEAsource::stDevice::fromResponseStringIdent(std::string cmdRes)
{
    cDebug dbg("cEAsource::stDevice::fromResponseStringIdent");
    vector<string> list = stringSplit(cmdRes, ",");
    if (list.size()>=4)
    {
        manufacturer = list[0];
        type = list[1];
        serial = list[2];
        version = list[3];
    }
    else
        dbg.printf(enDebugLevel_Error, "IDN response malformed!");
}

void cEAsource::stDevice::fromResponseStringNominal(std::string cmdRes)
{
    cDebug dbg("cEAsource::stDevice::fromResponseStringScale");
    vector<string> list = stringSplit(cmdRes, ";");
    if (list.size()==3)
    {
        voltage = (float)atof(list[0].c_str());
        current = (float)atof(list[1].c_str());
        power = (float)atof(list[2].c_str());
    }
    else
        dbg.printf(enDebugLevel_Error, "SYST:NOM response malformed!");
}

cEAsource::stActualValues::stActualValues()
{
    online = false;
    enabled = false;
    current = 0.0;
    voltage = 0.0;
    power = 0.0;
}

void cEAsource::stActualValues::fromResponseString(std::string cmdRes)
{
    cDebug dbg("cEAsource::stActualValues::fromResponseString");
    vector<string> list = stringSplit(cmdRes, ";");
    if (list.size()==4)
    {
        voltage = (float)atof(list[0].c_str());
        current = (float)atof(list[1].c_str());
        power = (float)atof(list[2].c_str());
        enabled = (list[3] == "ON") ? true : false;
        online = true;
    }
    else
        dbg.printf(enDebugLevel_Error, "MEAS response malformed!");
}

string cEAsource::stActualValues::toString()
{
    char text[128] = "";
    sprintf(text, "VOLT %.2f V, CURR %.2f A, POW %.2f W, OUT %s, LINK %s", voltage, current, power, enabled ? "ON " : "OFF", online ? "ONLINE " : "OFFLINE");
    return text;
}

cEAsource::stDemandValues::stDemandValues()
{
    enable = false;
    voltage = 0.0;
    currentSource = 0.0;
    currentSink = 0.0;
}

std::string cEAsource::stDemandValues::toCommandStringValues()
{
    char text[128] = "";
    sprintf(text, "VOLT %.2f V;CURR %.2f A;SINK:CURR %.2f A;POW %.2f W;SINK:POW %.2f W\n", voltage, currentSource, currentSink, powerSource, powerSink);
    return text;
}

std::string cEAsource::stDemandValues::toCommandStringState()
{
    return enable ? "OUTP ON\n" : "OUTP OFF\n";
}

void cEAsource::stDemandValues::fromResponseStringValues(std::string cmdRes)
{
    cDebug dbg("cEAsource::stDemandValues::fromResponseStringValues");
    vector<string> list = stringSplit(cmdRes, ";");
    if (list.size()==5)
    {
        voltage = (float)atof(list[0].c_str());
        currentSource = (float)atof(list[1].c_str());
        currentSink = (float)atof(list[2].c_str());
        powerSource = (float)atof(list[3].c_str());
        powerSink = (float)atof(list[4].c_str());
    }
    else
        dbg.printf(enDebugLevel_Error, "DEMAND VALUES response malformed!");
}

void cEAsource::stDemandValues::fromResponseStringState(std::string cmdRes)
{
    cDebug dbg("cEAsource::stDemandValues::fromResponseStringState");
    cmdRes = stringTrim(cmdRes);
    if (cmdRes == "ON")
        enable = true;
    else if (cmdRes == "OFF")
        enable = false;
    else
        dbg.printf(enDebugLevel_Error, "DEMAND STATE response malformed!");
    return;
}

cEAsource::stProtectionValues::stProtectionValues()
{
    voltage = 0;
    currentSource = 0;
    currentSink = 0;
    powerSource = 0;
    powerSink = 0;
}

std::string cEAsource::stProtectionValues::toCommandString()
{
    char text[128] = "";
    sprintf(text, "VOLT:PROT %.2f V;CURR:PROT %.2f A;SINK:CURR:PROT %.2f A;POW:PROT %.2f W;SINK:POW:PROT %.2f W\n", voltage, currentSource, currentSink, powerSource, powerSink);
    return text;
}

void cEAsource::stProtectionValues::fromResponseString(std::string cmdRes)
{
    cDebug dbg("cEAsource::stActualValues::fromResponseString");
    vector<string> list = stringSplit(cmdRes, ";");
    if (list.size()==5)
    {
        voltage = (float)atof(list[0].c_str());
        currentSource = (float)atof(list[1].c_str());
        currentSink = (float)atof(list[2].c_str());
        powerSource = (float)atof(list[3].c_str());
        powerSink = (float)atof(list[4].c_str());
    }
    else
        dbg.printf(enDebugLevel_Error, "PROT response malformed!");
}

cEAsource::cEAsource(cFramePort* pPort) :
    dbg("cEAsource"),
    comTimer(COMTIMEOUT)
    //reconnectTimer(RECONNECTTIMEOUT)
{
    port = EASOURCEPORT;
    resetRequest = false;
    reconnectRequest = false;
    configurationRequest = false;
    presetNominalPower = false;
    timeoutFlag = 0;
    //reconnecting = false;
    //pendingResponses =0;
    rcvMsgCount = 0;
    responseTimeMax = 0;
    responseLongTimeMax = 0;
    pTcpClient = nullptr;
    pSerialPort = nullptr;
    currentFaktor = 1;
    voltageFaktor = 1;
    this->pPort = pPort;
    state = enState::idle;
    pPort->setCallback(this);
    runUpdate();
}

bool cEAsource::isInitialized()
{
    return ((state>=enState::awaitSetProtection) && (state<enState::failure));
};

void cEAsource::reset()
{
    resetRequest = true;
}

void cEAsource::configure()
{
    configurationRequest = true;
}

void cEAsource::reconnect()
{
    reconnectRequest = true;
}

cEAsource::enStatus cEAsource::status()
{
    if (state <= enState::awaitSetComTimeout)
        return enStatus::initializing;
    if (state >= enState::failure)
        return enStatus::failure;
    if (values.actual.enabled && values.actual.online)
        return enStatus::active;
    return enStatus::passive;
}

unsigned int cEAsource::getPort()
{
    return port;
}

void cEAsource::send(const char* text)
{
    cDebug dbg("send", &this->dbg);
    int len = (int)strlen(text);
    if (len>BUFFERSIZE)
    {
        dbg.printf(enDebugLevel_Error, "String length greater than %i characters!", BUFFERSIZE);
        return;
    }
    pPort->send(text, len);
    if (timeoutFlag)
        dbg.printf(enDebugLevel_Info, "Resumed sending at %i ms!", comTimer.get());
    //reconnecting = false;
    //if (text[len-2] == '?')
    //    pendingResponses++;
    comTimer.start();

    char outBuffer[BUFFERSIZE] = "";
    strcpy(outBuffer, text);
    if ( state < enState::awaitMeasure)
        dbg.printf(enDebugLevel_Debug, "Sending: %s", stringTrim(outBuffer).c_str());
}

void cEAsource::operate(bool run)
{
    cDebug dbg("operate", &this->dbg);

    enState nextState = state;
    bool entered = state!=lastState;

    bool connected = true;
    if (pTcpClient)
        connected = pTcpClient->isConnected();

    //bool enterRepetitiveSend = false;
    int result = comTimer.elapsed();
    //dbg.printf(enDebugLevel_Debug, "timer: %lli > %i ms.", result, comTimer.get());

//    if (comTimer.elapsed(COMTIMEOUT/4))
//        dbg.printf(enDebugLevel_Info, "Communication with long response at %lli.", comTimer.get());

    // Short timeout after communication retry
    if (timeoutFlag && comTimer.elapsed(CYCLE*2))
        result = 1;

    if (result>0)
    {   // Time out
        comTimer.stop();


        if (state == enState::idle)
        {   // Connecting failed
            dbg.printf(enDebugLevel_Error, "Connection timeout at %lli ms!", comTimer.get());
            nextState = enState::failure;
        }
        if (state == enState::awaitReconnect)
        {   // Reconnecting failed
            dbg.printf(enDebugLevel_Error, "Reconnection timeout at %lli ms!", comTimer.get());
            nextState = enState::failure;
        }
        else if (timeoutFlag)
        {
            dbg.printf(enDebugLevel_Debug, "Communication timed out the second time at %lli ms!", comTimer.get());
            nextState = enState::awaitReconnect;
            timeoutFlag = 0;
            comTimer.start();
        }
        else
        {
            dbg.printf(enDebugLevel_Debug, "Communication timed out first time at %lli ms!", comTimer.get());
        }

//        if (pendingResponses > PENDINGMESSAGESLIMIT)
//        {
//            dbg.printf(enDebugLevel_Error, "Final communication timeout at %i ms after %i retries!", comTimer.get(), PENDINGMESSAGESLIMIT-1);
//            nextState = enState::failure;
//        }
//        else
//        {
//            dbg.printf(enDebugLevel_Info, "Communication timed out at %i ms in state %i!", comTimer.get(), state);
//            entered = true;                     // Repeat the operation of the current state and
//            enterRepetitiveSend = true;         // forcing a send operation.
//            this->dbg.setDebugLevel(enDebugLevel_Debug);
//        }

    }

    //if ((result<0 && comTimer.get()>=CYCLE && !pendingResponses) || enterRepetitiveSend)
    if (result<0 && comTimer.get()>=CYCLE && connected)
    {   // Next send operation
        if (resetRequest)
        {
            resetRequest = false;
            nextState = enState::idle;
        }
        else if (configurationRequest)
        {
            configurationRequest = false;
            nextState = enState::awaitSetProtection;
        }
        else
        {
            switch(state)
            {
            case enState::idle:
                if (entered)
                {
                    values.actual.online = false;
                    runUpdate();
                }
                if (run)
                {
                    if (pTcpClient)
                    {
                        if (pTcpClient->isConnected())
                            nextState = enState::awaitIdn;
                    }
                    else
                        nextState = enState::awaitIdn;
                }
                else
                    runUpdate();
                break;
            case enState::awaitIdn:
                if (entered)
                    send("*IDN?\n");
                else
                {
                    values.actual.online = true;
                    runUpdate();
                    nextState = enState::awaitNominal;
                }
                break;
            case enState::awaitNominal:
                if (entered)
                    send("SYST:NOM:VOLT?;SYST:NOM:CURR?;SYST:NOM:POW?\n");
                else
                    nextState = enState::awaitDemandValues;
                break;
            case enState::awaitDemandValues:
                if (entered)
                    send("VOLT?;CURR?;SINK:CURR?;POW?;SINK:POW?\n");
                else
                    nextState = enState::awaitDemandState;
                break;
            case enState::awaitDemandState:
                if (entered)
                    send("OUTP?\n");
                else
                    nextState = enState::awaitProtection;
                break;
            case enState::awaitProtection:
                if (entered)
                    send("VOLT:PROT?;CURR:PROT?;SINK:CURR:PROT?;POW:PROT?;SINK:POW:PROT?\n");
                else
                    nextState = enState::awaitSetRemote;
                break;
            case enState::awaitSetRemote:
                if (entered)
                {
                    char txt[256] = "";
                    sprintf(txt, "SYST:LOCK ON\n");
                    send(txt);
                }
                else
                {
                    runUpdate(1);
                    nextState = enState::awaitSetComTimeout;
                }
                break;
            case enState::awaitSetComTimeout:                   ///< It is required to seperate this command from enabling remote control and needs to be called afterwards.
                if (entered)
                {
                    char txt[256] = "";
                    sprintf(txt, "SYST:COMM:MON:TIM %i;SYST:COMM:MON:ACT ON\n", EASOURCETIMEOUT/1000);
                    send(txt);
                }
                else
                {
                    runUpdate(1);
                    nextState = enState::awaitMeasure;
                }
                break;
            case enState::awaitSetProtection:
                if (entered)
                    send(values.protection.toCommandString().c_str());
                else
                    nextState = enState::awaitMeasure;
                break;
            case enState::awaitMeasure:
                if (entered)
                    send("MEAS:VOLT?;MEAS:CURR?;MEAS:POW?;OUTP?\n");
                else
                {
                    runUpdate();
                    nextState = enState::awaitSetDemand;
                }
                break;
            case enState::awaitSetDemand:
                if (entered)
                {
                    stDemandValues saveValues = values.demand;
                    if (get().current > 0.1 && !values.demand.enable)
                    {
                        saveValues.currentSource = 0;
                        saveValues.currentSink = 0;
                        saveValues.enable = true;
                    }
                    send(saveValues.toCommandStringValues().c_str());
                }
                else
                    nextState = enState::awaitSetState;
                break;
            case enState::awaitSetState:
                if (entered)
                    send(values.demand.toCommandStringState().c_str());
                else
                    nextState = enState::awaitMeasure;
                break;
            case enState::awaitReconnect:
                if (entered)
                {
                    values.actual.online = false;
                    runUpdate();
                }
                nextState = enState::awaitReconnect;
                break;
            case enState::failure:
                if (entered)
                {
                    values.actual.online = false;
                    runUpdate();
                }
                nextState = enState::failure;
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

    if (result<0 && !connected)
    {
        //if (state != enState::awaitReconnect)
        //    nextState = enState::idle;
    }

    if (result == 0)
    {   // Check for response time delay being elapsed after sending not answered commands.
//        if (comTimer.get() >= RECONNECTTIMEOUT && state != enState::awaitReconnect && !reconnecting)
//        {
//            dbg.printf(enDebugLevel_Info, "Requesting reconnect at %i ms!", comTimer.get());
//            nextState = enState::awaitReconnect;
//        }
        switch (state)
        {
//        case enState::awaitReconnect:
//            if (reconnectRequest)
//            {
//                reconnectRequest = false;
//                reconnecting = true;
//                dbg.printf(enDebugLevel_Debug, "Resuming communication at time %i ms.", comTimer.get());
//                nextState = enState::awaitIdn;
//            }
//            break;
        case enState::awaitSetProtection:
        case enState::awaitSetRemote:
        case enState::awaitSetComTimeout:
        case enState::awaitSetDemand:
        case enState::awaitSetState:
            if (comTimer.elapsed(RESPONSETIME))
                comTimer.stop();
            break;
        default:;
        }
    }

    if (nextState != state && (nextState < enState::awaitMeasure || nextState > enState::awaitSetState))
        dbg.printf(enDebugLevel_Debug, "Entering state %i.", (int)nextState);
    lastState = state;
    state = nextState;

    checkTcpClient();
}

void cEAsource::onFrameReceive(cFramePort* pPort)
{
    cDebug dbg("onFrameReceive", &this->dbg);
    char rcvBuffer[BUFFERSIZE] = "";
    int len = pPort->receive(rcvBuffer, BUFFERSIZE);
    uint64_t responseTime = comTimer.get();
    if (len > 0 && (rcvBuffer[0] == 0 || rcvBuffer[0] == 1))    // Indicates Modbus messages.
    {

        dbg.printf(enDebugLevel_Debug, "Received Modbus message at %8lli ms len = %4i: %s.", responseTime, len, ByteArrayToString(rcvBuffer, len).c_str());
        return;
    }
//    if (--pendingResponses != 0)
//    {
//        if (pendingResponses < 0)
//        {
//            dbg.printf(enDebugLevel_Debug, "Non pending message received and discarded.");
//            pendingResponses = 0;
//        }
//        else
//            dbg.printf(enDebugLevel_Info, "Repetitive message discarded.");
//        comTimer.start();
//        return;
//    }
    if (len > 0)
    {
        if ( state < enState::awaitMeasure)
            dbg.printf(enDebugLevel_Debug, "Received  at %8lli ms len = %4i: %s", responseTime, len, rcvBuffer);
        //dbg.printf(enDebugLevel_Debug, "Response time %i ms, len %i: %s", (int)responseTime, len, rcvBuffer);

        rcvMsgCount++;
        if (responseTime > COMTIMEOUT/4)
        {
            dbg.printf(enDebugLevel_Debug, "Long response time detected: %i ms", responseTime);
            if (responseTime > responseLongTimeMax) responseLongTimeMax = responseTime;
        }
        else
            if (responseTime > responseTimeMax) responseTimeMax = responseTime;

        comTimer.stop();
        string rcvString = stringTrim(rcvBuffer);

        switch (state)
        {
        case enState::awaitIdn:
        {
            dbg.printf(enDebugLevel_Info, "IDN response: %s", rcvString.c_str());
            device.fromResponseStringIdent(rcvString);
        } break;
        case enState::awaitNominal:
        {
            dbg.printf(enDebugLevel_Debug, "NOM: %s", rcvString.c_str());
            device.fromResponseStringNominal(rcvString);
        } break;
        case enState::awaitDemandValues:
        {
            dbg.printf(enDebugLevel_Debug, "VOLT: %s", rcvString.c_str());
            if (!presetDemand)
                values.demand.fromResponseStringValues(rcvString);
            if (presetNominalPower) //  && !values.actual.online)
            {
                values.demand.powerSource = device.power;
                values.demand.powerSink = device.power;
                values.demand.enable = false;
            }
        } break;
        case enState::awaitProtection:
        {
            dbg.printf(enDebugLevel_Debug, "PROT: %s", rcvString.c_str());
            values.protection.fromResponseString(rcvString);
        } break;
        case enState::awaitMeasure:
        {
            //dbg.printf(enDebugLevel_Debug, "MEAS: %s", rcvString.c_str());
            bool lastOn = values.demand.enable && values.actual.enabled;
            values.actual.fromResponseString(rcvString);
            // Disable output state demand value in case the power source switched off the output itself.
            if (lastOn && !values.actual.enabled)
                values.demand.enable = false;
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

void cEAsource::openTcpClient(cTcpClient* pClient, std::string eaSourceAddress, int port)
{
    if (!pClient)
        return;

    this->eaSourceAddress = eaSourceAddress;
    this->port = port;
    this->pPort = pClient;
    pTcpClient = pClient;
    pSerialPort = nullptr;

    pTcpClient->open(port, eaSourceAddress);
//    if (!pTcpClient->isConnected())
//        reconnectTimer.start();
}

void cEAsource::checkTcpClient()
{
    if (!pTcpClient)
        return;

    // Automatic reconnect
    if (pTcpClient->isClosed())
    {
        comTimer.start();
        pTcpClient->open(port, eaSourceAddress);
    }

    if (state == cEAsource::enState::awaitReconnect && !resetRequest)
        if (pTcpClient->prepareClose())
            pTcpClient->close();

//    if (state == cEAsource::enState::awaitReconnect && !resetRequest)
//    {
//        if (pTcpClient->prepareClose() && !pTcpClient->isClosed())
//        {
//            pTcpClient->close();
//            reconnectTimer.start();
//        }
//    }
//    if (pTcpClient->isClosed() && reconnectTimer.elapsed()>0 && (state != cEAsource::enState::failure))
//    {
//        reconnectTimer.stop();
//        pTcpClient->open(port, eaSourceAddress);
//        if (pTcpClient->isConnected())
//        {
//            reconnect();
//        }
//        else
//        {
//            reconnectTimer.start();
//        }
//    }
}

void cEAsource::openSerialPort(cSerialPort* pPort, std::string serialPortName, cSerialPort::enBaud baud)
{
    if (!pPort)
        return;

    this->eaSourceAddress = serialPortName;
    this->pPort = pPort;
    pSerialPort = pPort;
    pTcpClient = nullptr;

    pPort->open(serialPortName.c_str(), baud, cSerialPort::enBytes::b8, cSerialPort::enParity::None, cSerialPort::enStopBits::One, 1000);
}

/**
 * @brief Setting demand values
 * @param demand
 */
void cEAsource::set(stPowerControl demand)
{
    if (!demand.currentControl && demand.voltage > 0 && demand.current == 0)
        demand.current = 1;                           // a minimum of output current is necessary to let the output voltage rise
    values.demand.enable = demand.enable;
    values.demand.voltage = demand.voltage / voltageFaktor;
    values.demand.currentSource = demand.current / currentFaktor;
}

/**
 * @brief Getting actual output values
 * @return
 */
stPowerControl cEAsource::get()
{
    return stPowerControl(values.actual.enabled, values.actual.voltage * voltageFaktor, values.actual.current * currentFaktor, isOperational());
}

/**
 * @brief Returns the demanded output values (set point values)
 * @return
 */
stPowerControl cEAsource::getDemand()
{
    return stPowerControl(values.demand.enable, values.demand.voltage * voltageFaktor, values.demand.currentSource * currentFaktor);
}

/**
 * @brief Used for synchronous state machine operation or communication
 */
void cEAsource::operate()
{
    operate(true);
}

/**
 * @brief Retrieves the actual output values having recieved from the last get() call
 * @return
 */
stPowerControl cEAsource::lastActual()
{
    return get();
}

/**
 * @brief Retrieves the desired (set point) values having set by the last get() call
 * @return
 */
stPowerControl cEAsource::lastDemand()
{
    return getDemand();
}
