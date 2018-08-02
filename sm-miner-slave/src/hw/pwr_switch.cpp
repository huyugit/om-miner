#include "pwr_switch.h"

#include "stm_gpio.h"
#include "mytime.h"
#include "format.hpp"


PwrSwitch::PwrSwitch()
    : isOn(false),
      pinOn(PXX),
      pinClk(PXX)
{}

void PwrSwitch::init(uint8_t _pinOn, uint8_t _pinClk)
{
    pinOn   = _pinOn;
    pinClk  = _pinClk;

    g_StmGPIO.configurePin( pinOn,  kOUTPUT );
    g_StmGPIO.configurePin( pinClk, kOUTPUT );

    set(false);
}

void PwrSwitch::set(bool on)
{
    log("PWR SWITCH: %d\n", on);

    g_StmGPIO.writePin(pinClk, 0);
    g_StmGPIO.writePin(pinOn, on);

    mysleep(250);

    g_StmGPIO.writePin(pinClk, 1);

    mysleep(250);

    isOn = on;
}
