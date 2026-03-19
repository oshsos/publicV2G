/**
@code
#define IP_ADDRESS "192.168.178.74"

#include <string>
#include <conio.h>
#include <stdio.h>
#include <windows.h>

#include "../LibCpp/HAL/Tools.h"
#include "../LibCpp/HAL/cDebug.h"
#include "../LibCpp/Lab/cEAsource.h"

#include "../LibCpp/HAL/Windows/cWinTcpClient.h"
#include "../LibCpp/HAL/Windows/cWinConsoleInput.h"

using namespace std;
using namespace LibCpp;

cWinConsoleInput console;
cDebug dbg("main", enDebugLevel_Debug);

cWinTcpClient client;
cEAsource power(&client);

void printHelp()
{
    printf_flush("q - quit\n");
    printf_flush("h - help\n");
    printf_flush("o - toggle output enable\n");
    printf_flush("d - disconnect\n");
    printf_flush("v - increase voltage\n");
    printf_flush("r - reset\n");
    printf_flush("\n");
}

int main()
{
    printf_flush("EA power source control\n\n");
    printHelp();

    dbg.setDebugLevel(enDebugLevel_Debug);
    dbg.setNewLineOutput();
    client.dbg.setInstanceName("client", &dbg);
    power.dbg.setInstanceName("power", &dbg, LibCpp::enDebugLevel_Info);

    power.presetNominalPower = true;

    printf_flush("Trying to connect to EA power source ...\n");

    power.openTcpClient(&client, IP_ADDRESS);

    while (1)
    {
        if (power.state == cEAsource::enState::awaitMeasure)
            printf_flush("%s Count %i\r", power.values.actual.toString().c_str(), power.rcvMsgCount);

        if (console.isKey())
        {
            char input = console.getKey();
            printf_flush("\n");

            if (input == 'q')
                break;
            if (input == 'h')
                printHelp();
            if (input == 'v')
            {
                if (power.values.demand.voltage < 10.0)
                    power.values.demand.voltage += 1.0;
                else
                    power.values.demand.voltage = 1.0;
            }
            if (input == 'o')
            {
                if (power.values.actual.enabled)
                {
                    printf_flush("Output disabled.\n");
                    power.values.demand.enable = false;
                }
                else
                {
                    printf_flush("Output enabled.\n");
                    power.values.demand.enable = true;
                }
            }
            if (input == 'r')
            {
                printf_flush("Reset\n");
                power.reset();
            }
        }

        client.operate();
        power.operate();

        Sleep(10);
    }

    client.close();

    printf_flush("Ready\n");
    return 0;
}
@endcode
**/

#ifndef CEASOURCE_H
#define CEASOURCE_H

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

class cEAsource : public cPowerSource, public iFramePort, public cUpdate
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
        awaitNominal,
        awaitDemandValues,
        awaitDemandState,
        awaitProtection,
        awaitSetRemote,
        awaitSetComTimeout,
        awaitSetProtection,
        awaitMeasure,
        awaitSetDemand,
        awaitSetState,
        awaitReconnect,
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
        void fromResponseStringNominal(std::string cmdRes);
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
        float currentSource;
        float currentSink;
        float powerSource;
        float powerSink;

        stDemandValues();
        std::string toCommandStringValues();
        std::string toCommandStringState();
        void fromResponseStringValues(std::string cmdRes);
        void fromResponseStringState(std::string cmdRes);
    };
    struct stProtectionValues
    {
        float voltage;
        float currentSource;
        float currentSink;
        float powerSource;
        float powerSink;

        stProtectionValues();
        std::string toCommandString();
        void fromResponseString(std::string cmdRes);
    };
    struct stValues
    {
        stDemandValues demand;
        stActualValues actual;
        stProtectionValues protection;
    };

public:
    cEAsource(cFramePort* pPort);                       ///< Constructor
    unsigned int getPort();                             ///< Get the standard or explicitly set port to address the power source hardware

    virtual void set(stPowerControl demand) override;   ///< Setting demand values
    virtual stPowerControl get() override;              ///< Getting actual output values
    virtual stPowerControl getDemand() override;        ///< Returns the demanded output values (set point values)

    virtual void operate() override;                    ///< Used for synchronous state machine operation or communication
    virtual stPowerControl lastActual() override;       ///< Retrieves the actual output values having recieved from the last get() call
    virtual stPowerControl lastDemand() override;       ///< Retrieves the demand (set point) values having set by the last get() call

    void operate(bool run);
    void reset();
    void reconnect();
    void configure();
    enStatus status();
    virtual bool isInitialized() override;
    inline bool isOperational(){return (state>=enState::awaitSetProtection && state<=enState::awaitSetState);};

    void openTcpClient(cTcpClient* pClient, std::string eaSourceAddress, int port = EASOURCEPORT);
    void openSerialPort(cSerialPort* pPort, std::string serialPortName, cSerialPort::enBaud baud = cSerialPort::enBaud::Bd9600);

private:
    void onFrameReceive(cFramePort* pPort) override;
    void send(const char* text);
    void checkTcpClient();

public:
    cDebug          dbg;
    unsigned int    port;
    stDevice        device;
    stValues        values;
    enState         state;
    bool            resetRequest;
    bool            configurationRequest;
    bool            reconnectRequest;
    bool            presetNominalPower;
    bool            presetDemand;
    int             rcvMsgCount;
    //bool            reconnecting;
    cTimer          comTimer;
    std::string     eaSourceAddress;
    //int             pendingResponses;
    int             timeoutFlag;
    uint64_t        responseTimeMax;
    uint64_t        responseLongTimeMax;
    float           currentFaktor;          // real output current is demand / currentFaktor
    float           voltageFaktor;

private:
    cFramePort*     pPort;
    enState         lastState;
    uint64_t        responseTime;
    cTimer          reconnectTimer;

    cTcpClient*     pTcpClient;
    cSerialPort*    pSerialPort;
};

}
#endif
