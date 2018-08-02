#ifndef GPIOPINOPI_H
#define GPIOPINOPI_H

#include "hw/GpioPin.h"

class GpioPinOPi
        : public GpioPin
{
public:
    static void initMapping();
    static uint8_t pinToGpio(uint8_t pinNumber);

public:
    static const uint8_t CFG_INPUT      = 0x0;
    static const uint8_t CFG_OUTPUT     = 0x1;
    static const uint8_t CFG_DISABLE    = 0x7;

    static const uint8_t PULL_DISABLE   = 0x0;
    static const uint8_t PULL_UP        = 0x1;
    static const uint8_t PULL_DOWN      = 0x2;

    static void setupCfg(uint8_t gpioNumber, uint8_t flags);
    static void setupPull(uint8_t gpioNumber, uint8_t flags);

public:
    GpioPinOPi(uint8_t _gpioNumber, bool isInput=false);

    void set(bool value) const;
    uint8_t read() const;

private:
    uint8_t gpioNumber;

    volatile uint32_t* datReg;
    uint32_t datMask;
};

#endif // GPIOPINOPI_H
