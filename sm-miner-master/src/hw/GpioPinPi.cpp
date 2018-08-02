#include "GpioPinPi.h"

#include "except/ApplicationException.h"
#include "hw/bcm2835/bcm2835.h"


void GpioPinPi::initMapping()
{
    if (!bcm2835_init())
        throw ApplicationException("GpioPinPi: bcm2835 lib init failed");
}

uint8_t GpioPinPi::pinToGpio(uint8_t pinNumber)
{
    uint8_t gpioNumber = 0;

    switch (pinNumber)
    {
    case  3: gpioNumber =  2; break;
    case  5: gpioNumber =  3; break;
    case  7: gpioNumber =  4; break;
    case  8: gpioNumber = 14; break;
    case 10: gpioNumber = 15; break;
    case 11: gpioNumber = 17; break;
    case 12: gpioNumber = 18; break;
    case 13: gpioNumber = 27; break;
    case 15: gpioNumber = 22; break;
    case 16: gpioNumber = 23; break;
    case 18: gpioNumber = 24; break;
    case 19: gpioNumber = 10; break;
    case 21: gpioNumber =  9; break;
    case 22: gpioNumber = 25; break;
    case 23: gpioNumber = 11; break;
    case 24: gpioNumber =  8; break;
    case 26: gpioNumber =  7; break;

    default:
        throw ApplicationException("GpioPinPi: can't map pin %u to gpio", pinNumber);
    }

    //printf("GpioPinPi: map pin %u to gpio %u\n", pinNumber, gpioNumber);
    return gpioNumber;
}


GpioPinPi::GpioPinPi(uint8_t _gpioNumber, bool isInput)
    : gpioNumber(_gpioNumber)
{
    if (isInput)
        bcm2835_gpio_fsel(gpioNumber, BCM2835_GPIO_FSEL_OUTP);
    else
        bcm2835_gpio_fsel(gpioNumber, BCM2835_GPIO_FSEL_INPT);
}

void GpioPinPi::set(bool value) const
{
    if (value)
        bcm2835_gpio_write(gpioNumber, HIGH);
    else
        bcm2835_gpio_write(gpioNumber, LOW);
}

uint8_t GpioPinPi::read() const
{
    return bcm2835_gpio_lev(gpioNumber);
}
