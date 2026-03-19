// UTF8 (ü) //
/**
\file   cRaspGpio.h

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2025-02-12

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL_Raspberry
@{

\class LibCpp::cGpio
**/

#ifndef CGPIO_H
#define CGPIO_H

#include "cInput.h"
#include "cOutput.h"

namespace LibCpp
{

/**
 * \brief Enumeration of GPIO configuration modes.
 */
enum enGpioMode
{
    UNDEFINED,          ///< Uninitialized mode
    INPUT,              ///< Usage as input pin
    OUTPUT,             ///< Usage as output pin
    OPEN_COLLECTOR,     ///< Driving to low level, but floating to high level
    OPEN_SOURCE         ///< Driving to high level, but floating to low level
};

/**
 * \brief Abstract representation of a general purpose input/output pin resource
 */
class cGpio : public cInput, public cOutput
{
public:
    cGpio(int ioNumber = 0, enGpioMode mode = enGpioMode::UNDEFINED, bool initialValue = false, bool inverse = false);      ///< \copybrief LibCpp::cGpio::cGpio
    virtual bool get();                                                                                                     ///< \copybrief LibCpp::cGpio::get
    virtual void setMode(enGpioMode mode, bool initialValue = false, bool inverse = false);                                 ///< \copybrief LibCpp::cGpio::setMode
    
public:
    int ioNumber;       ///< I/O port number associated with this recource.
    bool inverse;
    enGpioMode mode;    ///< Operation mode this I/O is configured to.
};

}

#endif
/** @} */
