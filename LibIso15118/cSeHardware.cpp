#include "cSeHardware.h"

cSeHardware::cSeHardware()
{
    pMainContactor = &simMainContactor;
    pSocketVoltage = &simSocketVoltage;
    pSocketCurrent = &simSocketCurrent;
    pPowerSource   = &simPowerSource;
}

bool cSeHardware::isInitialized()
{
    return true;
}
