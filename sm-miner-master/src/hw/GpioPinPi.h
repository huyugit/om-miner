#ifndef GPIOPINPI_H
#define GPIOPINPI_H

#include "hw/GpioPin.h"

class GpioPinPi
        : public GpioPin
{
public:
    static void initMapping();
    static uint8_t pinToGpio(uint8_t pinNumber);

    GpioPinPi(uint8_t _gpioNumber, bool isInput=false);

    void set(bool value) const;
    uint8_t read() const;

private:
    uint8_t gpioNumber;
};

#endif // GPIOPINPI_H
