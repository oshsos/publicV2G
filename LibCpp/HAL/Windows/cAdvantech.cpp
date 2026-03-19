#include "cAdvantech.h"

using namespace std;
using namespace LibCpp;
using namespace Automation::BDaq;

//const wchar_t* profilePath = PROFILE_PATH;

/**
 * \brief Constructor
 */
cAdvantechGpio::cAdvantechGpio(int ioNumber, enGpioMode mode)
{
    this->ioNumber = ioNumber;
    this->mode = mode;
    this->pDevice = nullptr;
}

/**
 * \brief Returns the state of the pin
 * 3.3V high level is represented by 'true'.
 */
bool cAdvantechGpio::get()
{
    unsigned char state = 0;
    pDevice->pInstantDoCtrl->ReadBit(ioNumber>>4, ioNumber&0x0F, &state);
    return state;
}

/**
 * \brief Sets the state of the pin
 * 3.3V high level is represented by 'true'.
 */
void cAdvantechGpio::set(bool on)
{
    pDevice->pInstantDoCtrl->WriteBit(ioNumber>>4, ioNumber&0x0F, on);
}

/**
 * \brief Sets the input/output mode of the gpio pin
 */
void cAdvantechGpio::setMode(enGpioMode mode)
{
//    int mask;
    switch (mode)
    {
    case enGpioMode::OUTPUT:
//        mask = pDevice->pDioPort->getItem(ioNumber>>4).getDirectionMask();
//        mask |= ioNumber & 0x0F;
//        pDevice->pDioPort->getItem(ioNumber>>4).setDirectionMask(mask);
//        mode = enGpioMode::OUTPUT;
        break;
    case enGpioMode::INPUT:
//        mask = pDevice->pDioPort->getItem(ioNumber>>4).getDirectionMask();
//        mask &= ~(ioNumber & 0x0F);
//        pDevice->pDioPort->getItem(ioNumber>>4).setDirectionMask(mask);
//        mode = enGpioMode::INPUT;
        break;
    default:;
//        mode = enGpioMode::UNDEFINED;
    }
}

cAdvantechAdc::cAdvantechAdc()
{
    this->pDevice = nullptr;
    this->adcNumber = 0;
}

cAdvantechAdc::cAdvantechAdc(int adcNumber)
{
    this->pDevice = nullptr;
    this->adcNumber = adcNumber;
}

/**
 * @brief returns = (measured value * scale) - offset
 * @return
 */
uint16_t cAdvantechAdc::get()
{
    double data[1];
    pDevice->pInstantAiCtrl->Read((int32)adcNumber, (int32)1, data);
    this->value = (uint16_t)(data[0] * 0x8000);
    return cAdc::get();
}

/**
 * @brief Constructor
 */
cAdvantechDac::cAdvantechDac()
{
    this->pDevice = nullptr;
    this->dacNumber = 0;
}

/**
 * @brief Constructor
 */
cAdvantechDac::cAdvantechDac(int dacNumber)
{
    this->pDevice = nullptr;
    this->dacNumber = dacNumber;
}

/**
 * @brief sets (value * scale) + offset
 * @return
 */
void cAdvantechDac::set(uint16_t value)
{
    pDevice->pInstantDoCtrl->Write(dacNumber, value>>8);
}

/**
 * @brief Default constructor
 * @param open  If true, opens the standard device HARDWARE_DEVICE.
 */
cAdvantech::cAdvantech(bool open) :
    dbg("cAdvantech", enDebugLevel_Info)
{
    pInstantDoCtrl = nullptr;
    if (open) this->open(HARDWARE_DEVICE);
}

/**
 * \brief Specific constructor, also opening the device
 * @param deviceName  String opening the device.
 */
cAdvantech::cAdvantech(string deviceName) :
    dbg("cAdvantech", enDebugLevel_Info)
{
    pInstantDoCtrl = nullptr;
    open(deviceName);
}

/**
 * @brief Opens the device in case it was created by the standard constructor.
 * @param deviceName
 */
void cAdvantech::open(string deviceName)
{
    cDebug dbg("open", &this->dbg);

    int i;
    if (deviceName.empty()) deviceName = HARDWARE_DEVICE;
    ErrorCode ret = Success;
    if (pInstantDoCtrl) return;
    pInstantDoCtrl = InstantDoCtrl::Create();
    wstring wDeviceName(deviceName.begin(), deviceName.end());
    DeviceInformation devInfo(wDeviceName.c_str());
    ret = pInstantDoCtrl->setSelectedDevice(devInfo);     // Returns ErrorCode ret = Success;
    if (ret == Success)
        dbg.printf(enDebugLevel_Info, "Advantech device %s initialized.", deviceName.c_str());
    else
        dbg.printf(enDebugLevel_Fatal, "Advantech device %s not found!", deviceName.c_str());

    pDioPort = pInstantDoCtrl->getPorts();

    for (i=0; i<pDioPort->getCount(); i++)
    {
        // The Advantech USB-4704 has hardware defined directions.
        // if (i==0) pDioPort->getItem(i).setDirectionMask(0xFF);
        for (int j=0; j<8; j++)
        {
            int index = i<<4 | j;
            if (index < GPIOMAX)
            {
                gpios[index].pDevice = this;
                gpios[index].ioNumber = i;
                if (i==0)
                    gpios[index].mode = enGpioMode::INPUT;
                else
                    gpios[index].mode = enGpioMode::OUTPUT;
            }
            else
                dbg.printf(enDebugLevel_Fatal, "Not enough memory reserved for GPIOs! Adjust the define GPIOMAX.");
        }
    }

    pInstantAiCtrl = InstantAiCtrl::Create();
    pInstantAiCtrl->setSelectedDevice(devInfo);     // Returns ErrorCode ret = Success; Error is checked above!
    pInstantAoCtrl = InstantAoCtrl::Create();
    pInstantAoCtrl->setSelectedDevice(devInfo);     // Returns ErrorCode ret = Success;

    //int32 channelCountMax = instantAiCtrl->getFeatures()->getChannelCountMax();
    for (i=0; i<ADCMAX; i++)
    {
        adcs[i].pDevice = this;
        adcs[i].adcNumber = i;
    }
    for (i=0; i<DACMAX; i++)
    {
        dacs[i].pDevice = this;
        dacs[i].dacNumber = i;
    }
}

/**
 * \brief Denstructor
 */
cAdvantech::~cAdvantech()
{
    cDebug dbg("~cAdvantech", 0, enDebugLevel_Info);

    //dbg.printf(enDebugLevel_Info, "Destructing.");
    if (pInstantAiCtrl) pInstantAiCtrl->Dispose();
    if (pInstantAoCtrl) pInstantAoCtrl->Dispose();
    //dbg.printf(enDebugLevel_Info, "Destruction finished.");
}

/**
 * \brief Returns a single gpio pin recource
 */
cGpio& cAdvantech::gpio(unsigned int ioNumber)
{
    if (ioNumber >= GPIOMAX)
        ioNumber = 0;
    return gpios[ioNumber];
}

/**
 * \brief Returns a single adc pin recource
 */
cAdc& cAdvantech::adc(unsigned int adcNumber)
{
    if (adcNumber >= ADCMAX)
        adcNumber = 0;
    return adcs[adcNumber];
}
/**
 * \brief Returns a single dac pin recource
 */
cDac& cAdvantech::dac(unsigned int dacNumber)
{
    if (dacNumber >= DACMAX)
        dacNumber = 0;
    return dacs[dacNumber];
}
