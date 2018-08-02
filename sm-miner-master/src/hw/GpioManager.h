#ifndef GPIO_MANAGER_H
#define GPIO_MANAGER_H

#include <stdint.h>
#include "hw/GpioPin.h"

class GpioManager
{
public:
    GpioManager();
    void init();

    void slaveResetByMask(uint32_t mask);

    void slaveReset(uint32_t sid);
    void slaveSelect(uint32_t sid, uint8_t value);

private:
    GpioPin* createPin(uint8_t pinNumber, bool isInput=false);

    void setMuxAddr(uint32_t addr);
    void readHwVer();

    GpioPin *pinMux0;
    GpioPin *pinMux1;

    GpioPin *pinHwVer;

    GpioPin *pinNRst;
    GpioPin *pinBoot;
    GpioPin *pinNss;

public:
    bool    spiSwMode;
    uint8_t mbHwVer;

    GpioPin *pinLedR;
    GpioPin *pinLedG;

    GpioPin *pinKey0;
    GpioPin *pinKey1;

    GpioPin *pinSpiMosi;
    GpioPin *pinSpiMiso;
    GpioPin *pinSpiClk;
};

extern GpioManager g_gpioManager;

#endif  // GPIO_MANAGER_H
