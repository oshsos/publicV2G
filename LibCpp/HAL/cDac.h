#ifndef CDAC_H
#define CDAC_H

#include <stdint.h>

namespace LibCpp
{

/**
 * @brief Virtual class abstracting a Digital to Analog Converter
 * The analog output is calculated by: (value * scale) + offset.\n
 * The scale is set as a fix point integer. A value of 0x2000 represents a 1.0.\n
 */
class cDac
{
public:
    cDac();

    virtual void set(uint16_t value);           ///< @copybrief LibCpp::cDac::cDac
    // preimplemented
    virtual void setScale(int16_t scale);       ///< @copybrief LibCpp::cDac::cDac
    virtual void setOffset(int16_t offset);     ///< @copybrief LibCpp::cDac::cDac
    virtual void setSigned(int16_t value);      ///< @copybrief LibCpp::cDac::cDac

public:
    uint16_t value;                             ///< public acces for simulation purposes
    int16_t scale;                              ///< Scale value of this channel
    int16_t offset;                             ///< Offset value of this channel
};

}

#endif
