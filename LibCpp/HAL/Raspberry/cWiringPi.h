// UTF8 (ü) //
/**
\file   cWiringPi.h

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2025-02-12

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL_Raspberry
@{
**/

#ifndef CWIRINGPI_H
#define CWIRINGPI_H

#define GPIOMAX 32

#include "../cGpio.h"

namespace LibCpp
{

/**
 * \class LibCpp::cWiringPi
 * \brief Class representing a gerneral purpose input output pin at the Raspberry Pi
 * This class is based on the wiringPi library (https://github.com/WiringPi/WiringPi) to be accessed by '<wiringPi.h>' and is required to be linked against this library (-lwiringPi).
 * This library is preinstalled on the Raspberry Pi.
 * 
 * The pin numbers are indicated as BCM code (which is not WiringPi nor header pin number).
 * 
 * \image WiringPiNumbers.jpg
 * [https://raspberrypihobbyist.blogspot.com/2015/03/new-raspberry-pi-gpio-pinout-diagram.html]
 * 
 * Depending source files for the following example are
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
 
/**
 * \brief Class representing a single GPIO pin of the Raspberry Pi
 * The class uses the library 'wiringpi'. 
 */
class cWiringGpio : public cGpio
{
public:
    cWiringGpio(int ioNumber = 0, enGpioMode mode = enGpioMode::UNDEFINED, bool initialValue = false, bool inverse = false);    ///< \copybrief LibCpp::cRaspGpio::cRaspGpio
    virtual bool get();                                                                                                         ///< \copybrief LibCpp::cRaspGpio::get
    virtual void set(bool on = true);                                                                                           ///< \copybrief LibCpp::cRaspGpio::set
    virtual void setMode(enGpioMode mode, bool initialValue = false, bool inverse = false);                                     ///< \copybrief LibCpp::cRaspGpio::setMode
};

/**
 * \class LibCpp::cWiringPi
 */
class cWiringPi
{
public:
    cWiringPi();                            ///< \copybrief LibCpp::cWiringPi::cWiringPi
    cGpio& gpio(unsigned int ioNumber);     ///< \copybrief LibCpp::cWiringPi::gpio

protected:
    cWiringGpio gpios[GPIOMAX];             ///< Array of the Raspberry Pi GPIO-pins.
};

}

#endif

/** @} */
