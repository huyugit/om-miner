#ifndef I2C_TMP75_H
#define I2C_TMP75_H

#include <stdint.h>
#include "i2c_sw.h"


class I2CTmp75
{
public:
    I2CTmp75(I2CSw &_i2c, uint8_t addr);

    void debugRegisters();

    bool read();

    bool readAlertLo();
    bool readAlertHi();

    bool writeAlertLo(uint8_t tempAlertLo);
    bool writeAlertHi(uint8_t tempAlertHi);

    bool debug;
    uint8_t temp;
    uint8_t alertLo, alertHi;

private:
    I2CSw &i2c;
    uint8_t i2cAddr;

    bool readReg(int ptr, uint8_t* dest, int size);
    bool writeReg(int ptr, uint8_t* dest, int size);
};

#endif // I2C_TMP75_H
