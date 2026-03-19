#ifndef CSEHARDWARE_H
#define CSEHARDWARE_H

#include "../LibCpp/HAL/cSuperwisedOutput.h"
#include "../LibCpp/HAL/cAdc.h"
#include "../LibCpp/Lab/cPowerSource.h"

class cSeHardware
{
public:
    cSeHardware();

    inline  LibCpp::cSuperwisedOutput&  mainContactor() {return *pMainContactor;};
    inline  LibCpp::cAdc&               socketVoltage() {return *pSocketVoltage;};
    inline  LibCpp::cAdc&               outputCurrent() {return *pSocketVoltage;};
    inline  LibCpp::cPowerSource&       powerSource()   {return *pPowerSource;};

    virtual bool                        isInitialized();

private:
    LibCpp::cSuperwisedOutput   simMainContactor;
    LibCpp::cAdc                simSocketVoltage;
    LibCpp::cAdc                simSocketCurrent;
    LibCpp::cPowerSource        simPowerSource;

protected:
    LibCpp::cSuperwisedOutput*  pMainContactor;
    LibCpp::cAdc*               pSocketVoltage;
    LibCpp::cAdc*               pSocketCurrent;
    LibCpp::cPowerSource*       pPowerSource;
};

#endif
