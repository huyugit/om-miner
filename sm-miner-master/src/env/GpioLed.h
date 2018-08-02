#ifndef GPIOLED_H
#define GPIOLED_H

#include <stdint.h>
#include "hw/GpioPin.h"


class GpioLed
{
public:
    enum LedState
    {
        LED_OFF = 0,
        LED_ON,
        LED_BLINK_SLOW,
        LED_BLINK_QUICK = 3,
        MAX_LED_STATE = 5
    };

    static uint32_t timerValue;

public:
    GpioLed();

    void init(GpioPin *_pin);
    void setState(LedState _state);

    void onTimerTick();

private:
    GpioPin *pin;
    LedState state;

    void setPins();
    bool isLedOn();
};

#endif // GPIOLED_H
