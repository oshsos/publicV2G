#ifndef CADC_H
#define CADC_H

#include <stdint.h>

namespace LibCpp
{

/**
 * @brief Virtual class abstracting an Analog to Digital Converter
 * The analog input is calculated by (measured value * scale) - offset .\n
 * The scale is set as a fix point integer. A value of 0x2000 represents a 1.0.\n
 */
class cAdc
{
public:
    cAdc();                                     ///> @copybrief LibCpp::cAdc::cAdc

    virtual void trigger();                     ///> @copybrief LibCpp::cAdc::trigger
    virtual uint16_t get();                     ///> @copybrief LibCpp::cAdc::get
    // preimplemented
    virtual void setScale(int16_t scale);       ///> @copybrief LibCpp::cAdc::setScale
    virtual void setOffset(int16_t offset);     ///> @copybrief LibCpp::cAdc::setOffset
    virtual int16_t getSigned();                ///> @copybrief LibCpp::cAdc::getSigned

public:
    uint16_t value;                             ///> Value memorizing the last value fetched from the ADC. You might write that value for simulation purpose.
    int16_t scale;                              ///< Scale value of this channel
    int16_t offset;                             ///< Offset value of this channel
};

}

#endif
