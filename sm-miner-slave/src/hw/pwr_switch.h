#ifndef PWR_SWITCH_H
#define PWR_SWITCH_H

#include <stdint.h>

class PwrSwitch
{
public:
    PwrSwitch();

    void init(uint8_t _pinOn, uint8_t _pinClk);
    void set(bool on);

    bool isOn;

private:
    uint8_t pinOn, pinClk;
};

#endif // PWR_SWITCH_H
