#ifndef GPIOKEY_H
#define GPIOKEY_H

#include <stdint.h>
#include "hw/GpioPin.h"


class GpioKey
{
public:
    GpioKey();

    void init(GpioPin *_pin);
    void poll();

    bool shortPress;
    bool longPress;

private:
    GpioPin *pin;
    uint32_t counter;
    bool longPressReported;
};

#endif // GPIOKEY_H
