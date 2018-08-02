#ifndef I2C_REG_H
#define I2C_REG_H

#include <stdint.h>
#include "i2c_sw.h"


class I2CReg
{
public:
    I2CReg(I2CSw &_i2c);

    bool read(uint8_t &data);
    bool write(uint8_t data);

private:
    I2CSw &i2c;
};

#endif // I2C_REG_H
