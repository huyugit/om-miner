#ifndef GPIOPIN_H
#define GPIOPIN_H

#include <stdint.h>

class GpioPin
{
public:
    virtual void set(bool value) const = 0;
    virtual uint8_t read() const = 0;
};

class GpioPinDummy
        : public GpioPin
{
public:
    virtual void set(bool value) const {}
    virtual uint8_t read() const {return 0;}
};

#endif // GPIOPIN_H
