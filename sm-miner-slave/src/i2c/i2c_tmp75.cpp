#include "i2c_tmp75.h"

#include "format.hpp"
#include "mytime.h"


#define TMP75_I2C_ADDR 0x90

#define TMP75_PTR_TEMP      0
#define TMP75_PTR_CONFIG    1
#define TMP75_PTR_T_LO      2
#define TMP75_PTR_T_HI      3


I2CTmp75::I2CTmp75(I2CSw &_i2c, uint8_t addr)
    : debug(false),
      temp(0),
      alertLo(0), alertHi(0),
      i2c(_i2c),
      i2cAddr(TMP75_I2C_ADDR | ((addr & 0x7) << 1))
{}

void I2CTmp75::debugRegisters()
{
    debug = true;

    uint8_t d8;
    uint16_t d16;

    readReg(TMP75_PTR_TEMP, &d8, sizeof(d8));
    log("I2CTmp75::debugRegisters: temp8=%u\n", d8);
    readReg(TMP75_PTR_TEMP, (uint8_t*)&d16, sizeof(d16));
    log("I2CTmp75::debugRegisters: temp16=%u\n", d16);

    readReg(TMP75_PTR_CONFIG, &d8, sizeof(d8));
    log("I2CTmp75::debugRegisters: cfg=%u\n", d8);

    readReg(TMP75_PTR_T_LO, &d8, sizeof(d8));
    log("I2CTmp75::debugRegisters: lo=%u\n", d8);

    readReg(TMP75_PTR_T_HI, &d8, sizeof(d8));
    log("I2CTmp75::debugRegisters: hi=%u\n", d8);

    d8 = 40;
    writeReg(TMP75_PTR_T_LO, &d8, sizeof(d8));
    readReg(TMP75_PTR_T_LO, &d8, sizeof(d8));
    log("I2CTmp75::debugRegisters: lo=%u\n", d8);

    d8 = 50;
    writeReg(TMP75_PTR_T_HI, &d8, sizeof(d8));
    readReg(TMP75_PTR_T_HI, &d8, sizeof(d8));
    log("I2CTmp75::debugRegisters: hi=%u\n", d8);

    while (1) {
        readReg(TMP75_PTR_TEMP, &d8, sizeof(d8));
        log("I2CTmp75::debugRegisters: temp8=%u\n", d8);
        mysleep(500);
    }

    STOP();
}

bool I2CTmp75::read()
{
    uint8_t data;
    if (!readReg(TMP75_PTR_TEMP, &data, sizeof(data))) {
        return false;
    }

    temp = data;

    if (debug) log("I2CTmp75::read: temp=%u\n", temp);
    return true;
}

bool I2CTmp75::readAlertLo()
{
    return readReg(TMP75_PTR_T_LO, &alertLo, sizeof(alertLo));
}

bool I2CTmp75::readAlertHi()
{
    return readReg(TMP75_PTR_T_HI, &alertHi, sizeof(alertHi));
}

bool I2CTmp75::writeAlertLo(uint8_t tempAlertLo)
{
    return writeReg(TMP75_PTR_T_LO, &tempAlertLo, sizeof(tempAlertLo));
}

bool I2CTmp75::writeAlertHi(uint8_t tempAlertHi)
{
    return writeReg(TMP75_PTR_T_HI, &tempAlertHi, sizeof(tempAlertHi));
}

bool I2CTmp75::readReg(int ptr, uint8_t *dest, int size)
{
    i2c.stopCond();
    i2c.startCond();

    if (!i2c.txByte(i2cAddr | I2C_WRITE)) // slave address + WRITE operation bit
    {
        i2c.stopCond();
        if (debug) log("TMP75_ERROR_SA\n");
        return false;
    }

    if (!i2c.txByte(ptr & 0x3)) // pointer register
    {
        i2c.stopCond();
        if (debug) log("TMP75_ERROR_PTR\n");
        return false;
    }

    i2c.stopCond();

    i2c.startCond();

    if (!i2c.txByte(i2cAddr | I2C_READ)) // slave address + READ operation bit
    {
        i2c.stopCond();
        if (debug) log("TMP75_ERROR_SA_2\n");
        return false;
    }

    for (int i = 0; i < size; i++)
    {
        i2c.rxByte(dest[i], (i == size - 1));
    }

    i2c.stopCond();


    if (debug) hexdump8(dest, size);
    return true;
}

bool I2CTmp75::writeReg(int ptr, uint8_t *dest, int size)
{
    i2c.stopCond();
    i2c.startCond();

    if (!i2c.txByte(i2cAddr | I2C_WRITE)) // slave address + WRITE operation bit
    {
        i2c.stopCond();
        if (debug) log("TMP75_ERROR_SA\n");
        return false;
    }

    if (!i2c.txByte(ptr & 0x3)) // pointer register
    {
        i2c.stopCond();
        if (debug) log("TMP75_ERROR_PTR\n");
        return false;
    }

    for (int i = 0; i < size; i++)
    {
        if (!i2c.txByte(dest[i])) {
            if (debug) log("TMP75_ERROR_WD\n");
        }
    }

    i2c.stopCond();


    if (debug) hexdump8(dest, size);
    return true;
}
