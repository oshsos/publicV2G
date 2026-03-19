#ifndef CHIGHPRECADDA_H
#define CHIGHPRECADDA_H

#include "../../cAdc.h"
#include "../../cDac.h"

namespace LibCpp
{
class cWiringAnalog;
    
/**
 * \brief Class representing a single analog input of the High Precision AD/DA header board.
 */
class cHighPrecAdc : public cAdc
{
    friend cWiringAnalog;
    
public:
    cHighPrecAdc(int adcNumber = 0);            ///< \copybrief LibCpp::cHighPrecAdc::cHighPrecAdc
    virtual uint16_t get();                     ///< \copybrief LibCpp::cHighPrecAdc::get
    
protected:
    int     adcNumber;                          ///< Number of the ADC input pin
};

/**
 * \brief Class representing a single analog output of the High Precision AD/DA header board.
 */
class cHighPrecDac : public cDac
{
    friend cWiringAnalog;
    
public:
    cHighPrecDac(int dacNumber = 0);            ///< \copybrief LibCpp::cHighPrecDac::cHighPrecDac
    virtual void set(uint16_t value);           ///> \copybrief LibCpp::cHighPrecDac::set
    
protected:
    int     dacNumber;                          ///< Number of the ADC input pin
};

}

#endif
