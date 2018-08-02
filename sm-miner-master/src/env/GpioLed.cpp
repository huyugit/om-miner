#include "GpioLed.h"

#include <cassert>

namespace {
static const uint32_t TIMING_PER_STATE[GpioLed::MAX_LED_STATE][2] =
{
    // first  - total interval length
    // second - active (led on) interval length
    { 1,  0 }, // LED_OFF
    { 1,  1 }, // LED_ON
    { 6,  2 }, // LED_BLINK_SLOW
    { 2,  1 }, // LED_BLINK_QUICK
};
}

uint32_t GpioLed::timerValue = 0;


GpioLed::GpioLed()
    : state(LED_ON)
{}

void GpioLed::init(GpioPin *_pin)
{
    pin = _pin;
    setPins();
}

void GpioLed::setState(LedState _state)
{
    assert(_state < MAX_LED_STATE);

    if (state != _state)
    {
        state = _state;
        setPins();
    }
}

void GpioLed::onTimerTick()
{
    if (state == LED_OFF || state == LED_ON)
        return;

    uint32_t len = TIMING_PER_STATE[state][0];
    uint32_t t2  = TIMING_PER_STATE[state][1];

    uint32_t t = timerValue % len;
    if (t == 0 || t == t2)
    {
        // switch led
        setPins();
    }
}

void GpioLed::setPins()
{
    pin->set(isLedOn());
}

bool GpioLed::isLedOn()
{
    uint32_t len = TIMING_PER_STATE[state][0];
    uint32_t t2  = TIMING_PER_STATE[state][1];

    uint32_t t = timerValue % len;
    return (t < t2);
}
