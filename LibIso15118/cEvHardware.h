#ifndef CEVHARDWARE_H
#define CEVHARDWARE_H

#include "../LibCpp/HAL/cSuperwisedOutput.h"
#include "../LibCpp/HAL/cAdc.h"

class cEvHardware
{
public:
    cEvHardware();

    virtual float                       batteryVoltage();

    inline  LibCpp::cSuperwisedOutput&  socketContactor()   {return *pSocketContactor;};
    inline  LibCpp::cAdc&               socketVoltage()     {return *pSocketVoltage;};
    inline  LibCpp::cAdc&               socketCurrent()     {return *pSocketCurrent;};

    virtual bool                        isInitialized();

private:
    LibCpp::cSuperwisedOutput   outSocketContactor;
    LibCpp::cAdc                adcSocketVoltage;
    LibCpp::cAdc                adcSocketCurrent;

protected:
    LibCpp::cSuperwisedOutput*  pSocketContactor;
    LibCpp::cAdc*               pSocketVoltage;
    LibCpp::cAdc*               pSocketCurrent;
};

#endif
