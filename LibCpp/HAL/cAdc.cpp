#include "cAdc.h"

using namespace LibCpp;

cAdc::cAdc()
{
    offset = 0;
    scale = 0x2000;
    value = 0;
}

void cAdc::trigger()
{
}

/**
 * @brief returns = (measured value * scale) - offset
 * @return
 */
uint16_t cAdc::get()
{
    int64_t val = value;
    val -= offset;
    if (val < 0) val = 0;
    int32_t sc = scale;
    if (sc < 0) sc = -sc;
    val = (val * scale) / 0x2000;
    return (int32_t) val;
}

/**
 * @brief Scales the uint16_t output. It is a fix point float with 0x2000 = 1.0 .
 * @param scale
 * See LibCpp::cAdc::cAdc also.\n
 * Changing the scale adapts the offset value to keep consistence to the measured value
 */
void cAdc::setScale(int16_t scale)
{
    this->scale = scale;
}

/**
 * @brief Offset value of the value read from the ADC the uint16_t output.
 * @param scale
 * Changing the scale adapts the offset value to keep consistence to the measured value
 */
void cAdc::setOffset(int16_t offset)
{
    this->offset = offset;
}

/**
 * @brief Return a signed interpretation of the ADC converted value.
 * @param value
 */
int16_t cAdc::getSigned()
{
    int16_t val = (int16_t)(get() - 0x8000);
    if (scale < 0) val *= -1;
    return val;
}
