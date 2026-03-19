#include "cHighPrecADDA.h"

extern "C"
{
    #include "ADS1256.h"
}

using namespace LibCpp;

cHighPrecAdc::cHighPrecAdc(int adcNumber)
{
    this->adcNumber = adcNumber;
}

/**
 * @brief returns = (measured value * scale) - offset
 * @return
 */
uint16_t cHighPrecAdc::get()
{
    value = ADS1256_GetChannalValue(adcNumber)>>7;
    return cAdc::get();
}

/**
 * @brief Constructor
 */
cHighPrecDac::cHighPrecDac(int dacNumber)
{
    this->dacNumber = dacNumber;
}

/**
 * @brief sets (value * scale) + offset
 * @return
 */
void cHighPrecDac::set(uint16_t value)
{
    // ADS1256_SetChannalValue(cDac::set(value), dacNumber);
}
