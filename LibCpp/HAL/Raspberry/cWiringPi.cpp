#include "cWiringPi.h"

extern "C"
{
    #include <wiringPi.h>
}

#undef INPUT
#undef OUTPUT

#define WIRING_INPUT    0
#define WIRING_OUTPUT   1

using namespace LibCpp;

/**
 * \brief Constructor
 */
cWiringGpio::cWiringGpio(int ioNumber, enGpioMode mode, bool initialValue, bool inverse)
{
    this->ioNumber = ioNumber;
    setMode(mode, initialValue, inverse);
}

/**
 * \brief Returns the state of the pin
 * 3.3V high level is represented by 'true'.
 */
bool cWiringGpio::get()
{
    if (inverse)
        return !digitalRead(ioNumber);
    else
        return digitalRead(ioNumber);
}

/**
 * \brief Sets the state of the pin
 * 3.3V high level is represented by 'true'.
 */
void cWiringGpio::set(bool on)
{
    if (inverse) on = !on;
    switch (mode)
    {
    case enGpioMode::OUTPUT:
        digitalWrite(ioNumber, on);
        break;
    case enGpioMode::INPUT:
        break;
    case enGpioMode::OPEN_COLLECTOR:
        if (on)
            pinMode(ioNumber, WIRING_INPUT);
        else
            pinMode(ioNumber, WIRING_OUTPUT);
        break;
    case enGpioMode::OPEN_SOURCE:
        if (on)
            pinMode(ioNumber, WIRING_OUTPUT);
        else
            pinMode(ioNumber, WIRING_INPUT);
        break;
    default:
        ;
    }
}

/**
 * \brief Sets the input/output mode of the gpio pin
 */
void cWiringGpio::setMode(enGpioMode mode, bool initialValue, bool inverse)
{
    cGpio::setMode(mode, initialValue, inverse);
    if (inverse) initialValue = !initialValue;
    switch (mode)
    {
    case enGpioMode::OUTPUT:
        digitalWrite(ioNumber, initialValue);
        pinMode(ioNumber, WIRING_OUTPUT);
        break;
    case enGpioMode::INPUT:
        pinMode(ioNumber, WIRING_INPUT);
        break;
    case enGpioMode::OPEN_COLLECTOR:
        digitalWrite(ioNumber, 0);
        if (initialValue)
            pinMode(ioNumber, WIRING_INPUT);
        else
            pinMode(ioNumber, WIRING_OUTPUT);
        break;
    case enGpioMode::OPEN_SOURCE:
        digitalWrite(ioNumber, 1);
        if (initialValue)
            pinMode(ioNumber, WIRING_OUTPUT);
        else
            pinMode(ioNumber, WIRING_INPUT);
        break;
   default:
        ;
    }
}

/**
 * \brief Constructor
 */
cWiringPi::cWiringPi()
{
    for (int i=0; i<GPIOMAX; i++)
        gpios[i].ioNumber = i;
    wiringPiSetupGpio();
}

/**
 * \brief Returns a single gpio pin recource
 */
cGpio& cWiringPi::gpio(unsigned int ioNumber)
{
    if (ioNumber >= GPIOMAX)
        ioNumber = 0;
    return gpios[ioNumber];
}
