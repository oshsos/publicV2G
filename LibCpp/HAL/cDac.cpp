#include "cDac.h"

using namespace LibCpp;

/**
 * @brief Constructor
 */
cDac::cDac()
{
}

/**
 * @brief sets (value * scale) + offset
 * @return
 */
void cDac::set(uint16_t value)
{
    int64_t val = value;
    val = val * scale / 0x2000;
    val += offset;
    this->value = val;
}

/**
 * @brief Sets the scale
 * @param scale
 * The scale is set as a fix point float. A value of 0x2000 = 1.0 .\n
 * Changing the scale does not influence the offset value. For calibration set and measure offset before setting the scale.
 */
void cDac::setScale(int16_t scale)
{
    this->scale = scale;
}

/**
 * @brief Sets the offset
 * @param offset
 * The scale is set as a fix point float. A value of 0x2000 = 1.0 .\n
 * Changing the scale does not influence the offset value. For calibration set and measure offset before setting the scale.
 */
void cDac::setOffset(int16_t offset)
{
    this->offset = offset;
}

/**
 * @brief Sets the output from a signed input value
 * @param value
 * This method is preimplemented!\n
 * This method assumes a value of zero delibering an output in the middle of the output voltage range.
 */
void cDac::setSigned(int16_t value)
{
    set((uint16_t)value + 0x8000);
}
