#include "cGpio.h"

using namespace LibCpp;

/**
 * \brief Constructor
 */
cGpio::cGpio(int ioNumber, enGpioMode mode, bool initialValue, bool inverse)
{
    this->ioNumber = ioNumber;
    this->mode = mode;
}
    
/**
 * \brief Returns the I/O signal level
 * Usually a hight voltage level is represented by 'true'.
 */
bool cGpio::get()
{
    if (mode == enGpioMode::OUTPUT)
        return cOutput::get();
    else
        return cInput::get();
}

/**
 * \brief Sets the I/O signal level
 * Usually a hight voltage level is represented by 'true'.
 */
void cGpio::setMode(enGpioMode mode, bool initialValue, bool inverse)
{
    this->mode = mode;
    this->inverse = inverse;
}
