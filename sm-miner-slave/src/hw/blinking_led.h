#ifndef BLINKING_LED_H
#define BLINKING_LED_H

#include <stdint.h>
#include "ms_defines.h"


class BlinkingLed
{
public:
    static uint32_t timerValue;

    BlinkingLed()
        : blinkType(LED_ON)
    {}

    void setBlinkType(uint8_t _blinkType);
    void onTimerTick();
    virtual void setPins() = 0;

//protected:
    LedStateEnum blinkType;
    bool isLedOn();
};


class OneColorLed
        : public BlinkingLed
{
public:
    void init(uint8_t _pin);
    void setPins();

private:
    uint8_t pin;
};


class TwoColorLed
        : public BlinkingLed
{
public:
    void setColor(bool _color);

    void init(uint8_t _pin1, uint8_t _pin2);
    void setPins();

    void reInit();

protected:
    uint8_t pin1, pin2;
    bool color;
};

#endif // BLINKING_LED_H
