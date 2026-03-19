// UTF8 (ü) //
/**
\file   cWiringAnalog.h

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2025-02-12

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL_Raspberry
@{

\class LibCpp::cWiringAnalog
**/

#ifndef CWIRINGANALOG_H
#define CWIRINGANALOG_H

#define ADCMAX 8
#define DACMAX 2

#include "../cAdc.h"
#include "../cDac.h"
#include "HighPrecADDA/cHighPrecADDA.h"

namespace LibCpp
{

/**
 * \brief Class representing a WaveShare High Precision AD/DA header board for Raspberry Pi
 * 
 * The class contains an array of LibCpp::cHighPrecAdc and LibCpp::cHighPrecDac instances which are inherited from the HAL class
 * LibCpp::cAdc and LibCpp::cDac respectively.\n
 * 
 * This class is based on the wiringPi library (https://github.com/WiringPi/WiringPi) to be accessed by '<wiringPi.h>' and is required to be linked against this library (-lwiringPi).
 * It is also necessary to link against -pthread. The 'wiringpi' library is preinstalled on the Raspberry Pi, the -pthread library is a standard C library.
 * The BCM2835 driver must be installed on the Raspberry Pi. For installation and further information see
 * https://www.waveshare.com/wiki/High-Precision_AD/DA_Board.\n
 * Some Raspberry GPIO pins are occupied by the AD/DA header board and cann't be used for application purposes. The AD/DA access is controlled
 * by the SPI interface through the 'wiringpi' library.
 * Depending source files for the following example are:
 * - LibCpp/HAL/cInput.cpp
 * - LibCpp/HAL/cOutput.cpp
 * - LibCpp/HAL/cGpio.cpp
 * - LibCpp/HAL/Raspberry/cWiringPi.cpp
 * - LibCpp/HAL/Linux/cLxConsoleInput.cpp
 * 
 * \code
    #include <cstdio>
    #include <unistd.h>

    #include "LibCpp/HAL/Linux/cLxConsoleInput.h"
    #include "LibCpp/HAL/Raspberry/cWiringPi.h"

    using namespace std;
    using namespace LibCpp;

    cLxConsoleInput console;
    
    cWiringPi raspberry;

    int main()
    {
        printf("cRaspIO\n");
        
        raspberry.gpio(21).setMode(enGpioMode::OUTPUT);
        
        while(true)
        {
            if (console.hasKey())
            {
                printf("\n");
                char c = console.getKey();
                if (c == 'q')
                    break;
                if (c == 's')
                    raspberry.gpio(21).set(true);
                if (c == 'r')
                    raspberry.gpio(21).set(false);
            }
            usleep(10000);
        }
        
        printf("Ready\n");
        return 0;
    }
 */
class cWiringAnalog
{
public:
    cWiringAnalog();                            ///< \copybrief cWiringAnalog::cWiringAnalog
    virtual ~cWiringAnalog();                   ///< \copybrief cWiringAnalog::~cWiringAnalog
    cAdc& adc(unsigned int ioNumber);           ///< \copybrief cWiringAnalog::adc
    cDac& dac(unsigned int ioNumber);           ///< \copybrief cWiringAnalog::dac

protected:
    cHighPrecAdc adcs[ADCMAX];                 ///< Array of ADC channels
    cHighPrecDac dacs[DACMAX];                 ///< Array of DAC channels
};

}

#endif

/** @} */
