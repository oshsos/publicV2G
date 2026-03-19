#ifndef CPOWERSOURCE_H
#define CPOWERSOURCE_H

#include <string>

namespace LibCpp
{

/** @brief Demand and actual values struct */
struct stPowerControl
{
    bool    enable;         ///< Enables the output and voltage/current control
    float   voltage;        ///< voltage
    float   current;        ///< current
    bool    currentControl; ///< Sets the power source to current control using 'voltage' as voltage limit
    bool    operational;    ///< Power source is ready to operate and for setting command values (Just for read operation)

    stPowerControl(bool enable = false, float voltage = 0, float current = 0, bool currentControl = false, bool operational = true);   ///< Initializer
    std::string toString(bool includeOperational = false); ///< String representation
    std::string toShortString(bool includeOperational = false); ///< String representation
};

/** @brief Abstraction class for power electronical energy sources */
class cPowerSource
{
public:
    cPowerSource();                                 ///< Constructor
    virtual bool isInitialized();                   ///< Returns true in case the corresponding power source hardware is ready to operate.

    virtual void set(stPowerControl demand);        ///< Setting demand values
    virtual stPowerControl get();                   ///< Getting actual output values
    virtual stPowerControl getDemand();             ///< Returns the demanded output values (set point values)

    virtual void operate();                         ///< Used for synchronous state machine operation or communication
    virtual stPowerControl lastActual();            ///< Retrieves the actual output values having recieved from the last get() call
    virtual stPowerControl lastDemand();            ///< Retrieves the desired (set point) values having set by the last get() call

    virtual std::string toString(bool includeOperational = true);                 ///< String repressentation of the source state
    virtual std::string toShortString(bool includeOperational = true);            ///< String repressentation of the source state

    inline void setVoltage(float voltage)   {stPowerControl cmd = getDemand(); cmd.voltage = voltage; set(cmd);};
    inline void setCurrent(float current)   {stPowerControl cmd = getDemand(); cmd.current = current; set(cmd);};
    inline void setEnable(bool on)          {stPowerControl cmd = getDemand(); cmd.enable = on; set(cmd);};
    inline void setCurrentControl(bool on)  {stPowerControl cmd = getDemand(); cmd.currentControl = on; set(cmd);};

private:
    stPowerControl value;           ///< Just to be used for simulation purpose
};

}
#endif
