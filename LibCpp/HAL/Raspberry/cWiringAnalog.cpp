#include "cWiringAnalog.h"
#include "HighPrecADDA/cHighPrecADDA.h"

extern "C"
{
    #include "HighPrecADDA/BoardSpi.h"
    #include "HighPrecADDA/ADS1256.h"
    //#include <wiringPi.h>
}

using namespace LibCpp;

/**
 * \brief Constructor
 */
cWiringAnalog::cWiringAnalog()
{
    int i;
    for (i=0; i<ADCMAX; i++)
        adcs[i].adcNumber = i;
    for (i=0; i<DACMAX; i++)
        dacs[i].dacNumber = i;
    DEV_ModuleInit();
    ADS1256_init();
}

/**
 * \brief Destructor
 */
cWiringAnalog::~cWiringAnalog()
{
    DEV_ModuleExit();
}

/**
 * \brief Returns a single adc pin recource
 */
cAdc& cWiringAnalog::adc(unsigned int adcNumber)
{
    if (adcNumber >= ADCMAX)
        adcNumber = 0;
    return adcs[adcNumber];
}

/**
 * \brief Returns a single dac pin recource
 */
cDac& cWiringAnalog::dac(unsigned int dacNumber)
{
    if (dacNumber >= DACMAX)
        dacNumber = 0;
    return dacs[dacNumber];
}
