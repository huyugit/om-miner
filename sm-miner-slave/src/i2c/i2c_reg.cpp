#include "i2c_reg.h"

#include "format.hpp"


#define REG_ADDR 0x5C


I2CReg::I2CReg(I2CSw &_i2c)
    : i2c(_i2c)
{
}

bool I2CReg::read(uint8_t &data)
{
    return i2c.readReg(REG_ADDR, data);
}

bool I2CReg::write(uint8_t data)
{
    return i2c.writeRegAndValidate(REG_ADDR, data);
}
