// UTF8 (ü) //
/**
\file   cAdvantech.h

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2025-02-12

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL_Windows
@{

 * \class LibCpp::cAdvantech
 *
 * This class is based on the Advantech C++ API library. It is required to link against the 'biodaq' library (-lbiodaq).
 * This library is installed by the XNavi installation program from Advantech within the folder 'C:/Windows/System32/biodaq.dll' (-LC:/Windows/System32).
 *
 * Depending source files for the following example are
 * \code
SOURCES += \
    ../LibCpp/HAL/cAdc.cpp \
    ../LibCpp/HAL/cDac.cpp \
    ../LibCpp/HAL/cDebug.cpp \
    ../LibCpp/HAL/cGpio.cpp \
    ../LibCpp/HAL/cInput.cpp \
    ../LibCpp/HAL/cOutput.cpp \
    ../LibCpp/HAL/Windows/cWinConsoleInput.cpp \
    ../LibCpp/HAL/Windows/cAdvantech.cpp\
    main.cpp

HEADERS += \
    ../LibCpp/HAL/Windows/cAdvantech.h \
    ../LibCpp/HAL/Windows/cWinConsoleInput.h \
    ../LibCpp/HAL/cAdc.h \
    ../LibCpp/HAL/cDac.h \
    ../LibCpp/HAL/cDebug.h \
    ../LibCpp/HAL/cGpio.h \
    ../LibCpp/HAL/cInput.h \
    ../LibCpp/HAL/cOutput.h

LIBS += \
    -L../LibCpp/HAL/Windows/Advantech
    -lAutomation.BDaq4
 * \endcode
 *
 * The following is a simple test program using the LibCpp::cAdvantech class.
 *
 * \code
#include <stdio.h>

#include "../LibCpp/HAL/Windows/cWinConsoleInput.h"
#include "../LibCpp/HAL/Windows/cAdvantech.h"

using namespace std;
using namespace LibCpp;

cWinConsoleInput console;
cAdvantech advantech;

int main()
{
    cDebug::instance().setDebugLevel(LibCpp::enDebugLevel_Debug);

    printf("Advantech\n");

    // The following command is not necessary, although allowed, as Advantech USB-4704 has hardware defined directions
    // advantech.gpio(0x00).setMode(enGpioMode::OUTPUT);

    advantech.adc(0).setScale(0x1E19);      // Scaled for 2.5V at input pin = 0x4000 as digital value (10.5V at signal input)
    advantech.adc(1).setScale(0x1f70);      // Scaled for 2.5V at input pin = 0x4000 as digital value (10.6V at signal input)

    int cnt=100;
    int sum0 = 0;
    int sum1 = 0;
    while(true)
    {
        if (console.hasKey())
        {
            printf("\n");
            char c = console.getKey();
            if (c == 'q')
                break;
            if (c == 's')
            {
                advantech.gpio(0x00).set(true);
            }
            if (c == 'r')
            {
                advantech.gpio(0x00).set(false);
            }
        }
        int value0 = advantech.adc(0).get();
        int value1 = advantech.adc(1).get();
        if (cnt-- <= 0)
        {
            value0 = sum0 / 100;
            value1 = sum1 / 100;
            float fval0 = (float)value0 / 0x4000 * 10.5;
            float fval1 = (float)value1 / 0x4000 * 10.6;
            printf("Analog 0: 0x%4x %fV Analog 1: 0x%4x %fV    \r", value0, fval0, value1, fval1);
            cnt = 100;
            sum0 = 0;
            sum1 = 0;
        }
        else
        {
            sum0 += value0;
            sum1 += value1;
        }

        Sleep(10);
    }

    printf("Ready\n");
    return 0;
}
**/

#ifndef CADVANTECH_H
#define CADVANTECH_H

#define DEMO_DEVICE  "DemoDevice,BID#0"
#define HARDWARE_DEVICE "USB-4704,BID#0"

//#define PROFILE_PATH L"../../profile/DemoDevice.xml"
#define GPIOMAX 32
#define ADCMAX 8
#define DACMAX 2

#include <string>
#include <vector>

#include "../cDebug.h"
#include "../cGpio.h"
#include "../cAdc.h"
#include "../cDac.h"
#include "../cDebug.h"
#include "Advantech/BDaqCtrl.h"

namespace LibCpp
{

class cAdvantech;

/**
 * \brief Class representing gerneral purpose input output pins using Advantech USB-4704
 * The ioNumber should be given as a hex value. The last 4 bit indicate the bit of the port being addressed.
 * The higher four bits indicate the port being addressed.
 * The ioNumber of 0x13 addresses the third bit of port 1.
 */
class cAdvantechGpio : public cGpio
{
    friend cAdvantech;

public:
    cAdvantechGpio(int ioNumber = 0, enGpioMode mode = enGpioMode::UNDEFINED);  ///< \copybrief LibCpp::cAdvantechGpio::cAdvantechGpio
    virtual bool get();                                                         ///< \copybrief LibCpp::cAdvantechGpio::get
    virtual void set(bool on = true);                                           ///< \copybrief LibCpp::cAdvantechGpio::set
    virtual void setMode(enGpioMode mode);                                      ///< \copybrief LibCpp::cAdvantechGpio::setMode

protected:
    cAdvantech* pDevice;                                                        ///< Pointer to the cAdvantech instance
};

/**
 * \brief Class representing a single analog input of the Advantech Acquisition module.
 */
class cAdvantechAdc : public cAdc
{
    friend cAdvantech;

public:
    cAdvantechAdc();                            ///< \copybrief LibCpp::cAdvantechAdc::cAdvantechAdc
    cAdvantechAdc(int adcNumber);               ///< \copybrief LibCpp::cAdvantechAdc::cAdvantechAdc(int)
    uint16_t get();                     ///< \copybrief LibCpp::cAdvantechAdc::get

protected:
    int         adcNumber;                      ///< Number of the ADC input pin
    cAdvantech* pDevice;                        ///< Pointer to the cAdvantech instance
};

/**
 * \brief Class representing a single analog output of the Advantech Acquisition module.
 */
class cAdvantechDac : public cDac
{
    friend cAdvantech;

public:
    cAdvantechDac();                            ///< \copybrief LibCpp::cAdvantechDac::cAdvantechDac
    cAdvantechDac(int dacNumber);               ///< \copybrief LibCpp::cAdvantechDac::cAdvantechDac(int)
    virtual void set(uint16_t value);           ///< \copybrief LibCpp::cAdvantechDac::set

protected:
    int         dacNumber;                      ///< Number of the ADC input pin
    cAdvantech* pDevice;                        ///< Pointer to the cAdvantech instance
};


/**
 * \brief  Advantech hardware access representation
 * 
 * The class contains an array of LibCpp::cAdvantechGpio instances which are inherited from the HAL class LibCpp::cGpio, itself inherited from LibCpp::cOutput and LibCpp::cInput.
 * The same applies for ADC and DAC purpose.
 */
class cAdvantech
{
    friend cAdvantechGpio;
    friend cAdvantechAdc;
    friend cAdvantechDac;

public:
    cAdvantech(bool open = false);                                  ///< \copybrief LibCpp::cAdvantech::cAdvantech
    cAdvantech(std::string deviceName);                             ///< \copybrief LibCpp::cAdvantech::cAdvantech
    ~cAdvantech();                                                  ///< \copybrief LibCpp::cAdvantechGpio::~cAdvantechGpio

    void open(std::string deviceName = HARDWARE_DEVICE);            ///< \copybrief LibCpp::cAdvantechGpio::open
    cGpio& gpio(unsigned int ioNumber);                             ///< \copybrief LibCpp::cAdvantech::gpio
    cAdc&  adc (unsigned int adcNumber);                            ///< \copybrief LibCpp::cAdvantech::adc
    cDac&  dac (unsigned int dacNumber);                            ///< \copybrief LibCpp::cAdvantech::dac

public:
    cDebug      dbg;

protected:
    cAdvantechGpio gpios[GPIOMAX];                                  ///< Array of gpio pin recources.
    cAdvantechAdc  adcs[ADCMAX];                                    ///< Array of adc pin recources.
    cAdvantechDac  dacs[DACMAX];                                    ///< Array of dac pin recources.
    Automation::BDaq::InstantDoCtrl* pInstantDoCtrl;                ///< Device pointer for instant io control access
    Automation::BDaq::Array<Automation::BDaq::DioPort>* pDioPort;   ///< List of available ports of 8 bit each
    Automation::BDaq::InstantAiCtrl* pInstantAiCtrl;                ///< Device pointer for instant analog input access
    Automation::BDaq::InstantAoCtrl* pInstantAoCtrl;                ///< Device pointer for instant analog output access
};

}

#endif

/** @} */
