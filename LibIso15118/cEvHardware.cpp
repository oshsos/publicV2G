#include "cEvHardware.h"

cEvHardware::cEvHardware()
{
    pSocketContactor = &outSocketContactor;
    pSocketVoltage   = &adcSocketVoltage;
    pSocketCurrent   = &adcSocketCurrent;
}

bool cEvHardware::isInitialized()
{
    return true;
}

float cEvHardware::batteryVoltage()
{
    return 380;
}
