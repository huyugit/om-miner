#ifndef I2C_BOARD_ACCESS_H
#define I2C_BOARD_ACCESS_H

#include <stdint.h>
#include "i2c_sw.h"
#include "i2c_reg.h"
#include "i2c_tmp75.h"
#include "mutex.h"

#include "i2c_nt3h1x01.h"     //chenbo add 20180108

class I2CBoardAccess
{
public:
    I2CBoardAccess(uint8_t _boardId);
    ~I2CBoardAccess();

    I2CSw& i2c() const;
    I2CReg ocpReg() const;
    I2CTmp75 tmp75(uint8_t addr) const;

	I2CNT3H1X01 nt3h1x01(uint8_t addr) const;

    static Mutex sharedAccess;

private:
    uint8_t boardId;
};

#endif // I2C_BOARD_ACCESS_H
