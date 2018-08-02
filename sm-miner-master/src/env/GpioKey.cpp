#include "GpioKey.h"

#include <cassert>

#define SHORT_PERIOD 20
#define LONG_PERIOD 300


GpioKey::GpioKey()
{}

void GpioKey::init(GpioPin *_pin)
{
    pin = _pin;
}

void GpioKey::poll()
{
    assert(pin);

    shortPress = false;
    longPress = false;

    if (pin->read())
    {
        if (counter > LONG_PERIOD && !longPressReported)
        {
            longPress = true;
            longPressReported = true;
        }

        counter++;
    }
    else {
        if (counter > SHORT_PERIOD)
        {
            shortPress = true;
        }

        counter = 0;
        longPressReported = false;
    }
}
