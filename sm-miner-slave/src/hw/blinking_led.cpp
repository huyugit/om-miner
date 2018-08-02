#include "blinking_led.h"

#include "format.hpp"
#include "stm_gpio.h"


static const uint32_t INTERVALS_PER_BLINK_TYPE[MAX_LED_BLINK_TYPES][2] =
{
    // first  - total interval length
    // second - active (led on) interval length
    { 1,  0 }, // LED_OFF
    { 1,  1 }, // LED_ON
    { 6,  2 }, // LED_BLINK_SLOW
    { 2,  1 }, // LED_BLINK_QUICK
    { 10, 5 }, // LED_BLINK_SWITCH
};

uint32_t BlinkingLed::timerValue = 0;


void BlinkingLed::setBlinkType(uint8_t _blinkType)
{
    ASSERT(_blinkType < MAX_LED_BLINK_TYPES, "Wrong blinkType!");

    if (blinkType != _blinkType)
    {
        blinkType = (LedStateEnum)_blinkType;
        setPins();
    }
}

void BlinkingLed::onTimerTick()
{
    if (blinkType == LED_OFF || blinkType == LED_ON)
        return;

    uint32_t len = INTERVALS_PER_BLINK_TYPE[blinkType][0];
    uint32_t t2  = INTERVALS_PER_BLINK_TYPE[blinkType][1];

    uint32_t t = timerValue % len;
    if (t == 0 || t == t2)
    {
        // switch led
        setPins();
    }
}

bool BlinkingLed::isLedOn()
{
    uint32_t len = INTERVALS_PER_BLINK_TYPE[blinkType][0];
    uint32_t t2  = INTERVALS_PER_BLINK_TYPE[blinkType][1];

    uint32_t t = timerValue % len;
    return (t < t2);
}


void OneColorLed::init(uint8_t _pin)
{
    pin = _pin;
    g_StmGPIO.configurePin( pin, kOUTPUT );
    setPins();
}

void OneColorLed::setPins()
{
    g_StmGPIO.writePin( pin, !isLedOn() );
}


void TwoColorLed::setColor(bool _color)
{
    if (color != _color)
    {
        color = _color;
        setPins();
    }
}

void TwoColorLed::init(uint8_t _pin1, uint8_t _pin2)
{
    pin1 = _pin1;
    pin2 = _pin2;
    color = true;

    g_StmGPIO.configurePin( pin1, kOUTPUT );
    g_StmGPIO.configurePin( pin2, kOUTPUT );

    setPins();
}

void TwoColorLed::reInit()
{
    g_StmGPIO.configurePin( pin1, kOUTPUT );
    g_StmGPIO.configurePin( pin2, kOUTPUT );

    setPins();
}

void TwoColorLed::setPins()
{
    bool firstPhase = isLedOn();

    if (firstPhase)
    {
        g_StmGPIO.writePin( pin1, color );
        g_StmGPIO.writePin( pin2, !color );
    }
    else {
        if (blinkType == LED_BLINK_SWITCH)
        {
            g_StmGPIO.writePin( pin1, !color );
            g_StmGPIO.writePin( pin2, color );
        }
        else {
            g_StmGPIO.writePin( pin1, true );
            g_StmGPIO.writePin( pin2, true );
        }
    }
}
