/**
 * https://github.com/EenVan5/InfoDsp3005v
**/

#ifndef CVOLTCRAFTSOURCE_H
#define CVOLTCRAFTSOURCE_H

#define EASOURCEPORT 5025

#include <string>

#include "../HAL/cFramePort.h"
#include "../HAL/cTcpClient.h"
#include "../HAL/cSerialPort.h"
#include "../HAL/cDebug.h"
#include "../HAL/cUpdate.h"
#include "../cTimer.h"
#include "cPowerSource.h"

namespace LibCpp
{

class cVoltcraftSource : public cPowerSource, public iFramePort, public cUpdate
{
public:
    enum class enStatus
    {
        initializing,
        passive,
        active,
        failure
    };
    enum class enUpdate
    {
        initialize,
        actual
    };
    enum class enState
    {
        idle,
        awaitIdn,
        awaitOutput,
        awaitDemandVoltage,
        awaitDemandCurrent,
        awaitLimitVoltage,
        awaitLimitCurrent,
        awaitSetLimitVoltage,
        awaitSetLimitCurrent,
        awaitMeasure,
        awaitSetVoltage,
        awaitSetCurrent,
        awaitSetOutput,
        failure,
        finished
    };
    struct stDevice
    {
        std::string manufacturer;
        std::string type;
        std::string serial;
        std::string version;
        float voltage;
        float current;
        float power;

        stDevice();
        void fromResponseStringIdent(std::string cmdRes);
    };
    struct stActualValues
    {
        bool online;
        bool enabled;
        float voltage;
        float current;
        float power;

        stActualValues();
        void fromResponseString(std::string cmdRes);
        std::string toString();
    };
    struct stDemandValues
    {
        bool enable;
        float voltage;
        float current;

        stDemandValues();
    };
    struct stLimitValues
    {
        float voltage;
        float current;

        stLimitValues();
    };
    struct stValues
    {
        stDemandValues demand;
        stActualValues actual;
        stLimitValues limit;
    };

public:
    cVoltcraftSource(cFramePort* pPort);                       ///< Constructor

    virtual void set(stPowerControl demand) override;   ///< Setting demand values
    virtual stPowerControl get() override;              ///< Getting actual output values
    virtual stPowerControl getDemand() override;        ///< Returns the demanded output values (set point values)

    virtual void operate() override;                    ///< Used for synchronous state machine operation or communication
    virtual stPowerControl lastActual() override;       ///< Retrieves the actual output values having recieved from the last get() call
    virtual stPowerControl lastDemand() override;       ///< Retrieves the demand (set point) values having set by the last get() call

    void operate(bool run);
    void reset();
    void configure();
    enStatus status();
    virtual bool isInitialized() override;

    void openSerialPort(cSerialPort* pPort, std::string serialPortName, cSerialPort::enBaud baud = cSerialPort::enBaud::Bd115200);

private:
    void onFrameReceive(cFramePort* pPort) override;
    void send(const char* text);

public:
    cDebug          dbg;
    stDevice        device;
    stValues        values;
    enState         state;
    cTimer          comTimer;

    bool            resetRequest;
    bool            configurationRequest;
    bool            presetFromDevice;
    float           currentFaktor;          // real output current is demand / currentFaktor
    float           voltageFaktor;

    bool            lastValidOutput;

    uint64_t        responseTime;
    uint64_t        responseTimeMax;
    int             rcvMsgCount;

private:
    cFramePort*     pPort;
    enState         lastState;
    cTimer          reconnectTimer;

    cSerialPort*    pSerialPort;
};

}
#endif
