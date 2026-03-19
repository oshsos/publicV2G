#include "../HAL/Tools.h"

#include "cPowerSource.h"

using namespace std;
using namespace LibCpp;

/**
 * @brief Initializer
 * @param enable
 * @param voltage
 * @param current
 * @param operational
 */
stPowerControl::stPowerControl(bool enable, float voltage, float current, bool currentControl, bool operational) :
    enable(enable),
    voltage(voltage),
    current(current),
    currentControl(currentControl),
    operational(operational)
{
};

string stPowerControl::toString(bool includeOperational)
{
    string out;
    out += "Output = ";
    out += enable ? " ON" : "OFF";
    out += stringFormat(", Voltage = %8.2g", voltage);
    out += stringFormat(", Current = %8.2g", current);
    out += stringFormat(", %s control", currentControl ? "Current" : "Voltage");
    if (includeOperational)
    {
        out += ", Operational = ";
        out += operational ? " TRUE" : "FALSE";
    }
    return out;
}

string stPowerControl::toShortString(bool includeOperational)
{
    string out;
    out += enable ? " ON" : "OFF";
    out += stringFormat(", %8.2g V", voltage);
    out += stringFormat(", %8.2g A", current);
    out += currentControl ? "CC" : "VC";
    if (includeOperational)
    {
        out += operational ? ", OK" : ", NO";
    }
    return out;
}

cPowerSource::cPowerSource()
{
}

bool cPowerSource::isInitialized()
{
    return true;
}

void cPowerSource::set(stPowerControl demand)
{
    value = demand;
}

stPowerControl cPowerSource::get()
{
    return value;
}

stPowerControl cPowerSource::getDemand()
{
    return value;
}

stPowerControl cPowerSource::lastActual()
{
    return value;
}

stPowerControl cPowerSource::lastDemand()
{
    return value;
}

void cPowerSource::operate()
{
}

string cPowerSource::toString(bool includeOperational)
{
    string out;
    stPowerControl act = lastActual();
    stPowerControl dem = lastDemand();
    out += "Actual: " + act.toString(includeOperational);
    out += "; Demand: " + dem.toString();
    return out;
}

string cPowerSource::toShortString(bool includeOperational)
{
    string out;
    stPowerControl act = lastActual();
    stPowerControl dem = lastDemand();
    out += "Actual: " + act.toShortString(includeOperational);
    out += "; Demand: " + dem.toShortString();
    return out;
}
