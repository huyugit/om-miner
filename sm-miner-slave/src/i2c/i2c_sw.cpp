#include "i2c_sw.h"

#include "stm_gpio.h"
#include "mytime.h"
#include "format.hpp"


I2CSw::I2CSw()
    : debug(true),
      sclPin(PXX),
      sdaPin(PXX)
{}

void I2CSw::init(uint8_t _sclPin, uint8_t _sdaPin)
{
    sclPin = _sclPin;
    sdaPin = _sdaPin;
}

void I2CSw::configure()
{
    g_StmGPIO.configurePin(sclPin, kOUTPUT);
    g_StmGPIO.configurePin(sdaPin, kOUTPUT);

    g_StmGPIO.writePin(sclPin, 1);
    g_StmGPIO.writePin(sdaPin, 1);
}

void I2CSw::delay()
{
    // 30  180 KHz
    // 50  130 KHz
    // 70  105 KHz

    for (uint32_t i=0; i < 70; i++) {
        __NOP();
    }
}

void I2CSw::startCond()
{
    delay();
    delay();

    g_StmGPIO.configurePin(sdaPin, kOUTPUT);
    g_StmGPIO.writePin(sdaPin, 0);

    delay();
    delay();
}

void I2CSw::stopCond()
{
    g_StmGPIO.writePin(sclPin, 0);
    delay();

    g_StmGPIO.configurePin(sdaPin, kOUTPUT);
    g_StmGPIO.writePin(sdaPin, 0);
    delay();

    g_StmGPIO.writePin(sclPin, 1);
    delay();

    g_StmGPIO.writePin(sdaPin, 1);
    delay();
}

void I2CSw::txBit(uint8_t x)
{
    g_StmGPIO.writePin(sclPin, 0);
    delay();

    g_StmGPIO.configurePin(sdaPin, kOUTPUT);
    g_StmGPIO.writePin(sdaPin, x);
    delay();

    g_StmGPIO.writePin(sclPin, 1);
    delay();

    delay();
}

uint8_t I2CSw::rxBit()
{
    g_StmGPIO.writePin(sclPin, 0);
    delay();

    g_StmGPIO.configurePin(sdaPin, kINPUT);
    delay();

    g_StmGPIO.writePin(sclPin, 1);
    delay();

    uint8_t result = g_StmGPIO.readPin(sdaPin);
    delay();

    return result;
}

bool I2CSw::txByte(uint8_t x)
{
    for (int i = 0; i < 8; i++)
    {
        txBit(x & 0x80);
        x <<= 1;
    }

    return (rxBit() == 0);
}

void I2CSw::rxByte(uint8_t &x, bool isLast)
{
    x = 0;
    for (int i = 0; i < 8; i++)
    {
        x = (x << 1) | (rxBit() ? 1 : 0);
    }

    if (!isLast)
        txBit(0); // send ACK
    else
        rxBit(); // after the last byte - NO ACK
}

bool I2CSw::readReg(uint8_t addr, uint8_t &data)
{
    if (debug) log("I2C::readReg:  SA=0x%02x", addr);

    stopCond();
    startCond();

    if (!txByte(addr | I2C_READ))
    {
        stopCond();
        if (debug) log(" - ERROR_SA\n");
        return false;
    }

    rxByte(data, true);

    stopCond();

    if (debug) log(" - 0x%02x\n", data);
    return true;
}

bool I2CSw::writeReg(uint8_t addr, uint8_t data)
{
    if (debug) log("I2C::writeReg: SA=0x%02x - 0x%02x ", addr, data);

    startCond();

    if (!txByte(addr | I2C_WRITE))
    {
        stopCond();
        if (debug) log("ERROR_SA\n");
        return false;
    }

    if (!txByte(data))
    {
        stopCond();
        if (debug) log("ERROR_DATA\n");
        return false;
    }

    stopCond();

    if (debug) log("\n");
    return true;
}

bool I2CSw::writeRegAndValidate(uint8_t addr, uint8_t data)
{
    uint8_t d1 = data;
    uint8_t d2 = 0xff;

    for (int attempts = 0; attempts < 5; attempts++)
    {
        writeReg(addr, d1);
        readReg(addr, d2);

        if (d1 == d2)
        {
            return true;
        }
        else
        {
            //log("I2CSw:writeReg: ERROR: write 0x%02x != read 0x%02x\n", d1, d2);
        }
    }

    //log("I2CSw:writeReg: ERROR: failed after several attempts\n");
    return false;
}

void I2CSw::scan()
{
    log("i2c::scan: begin\n");
    for (int i = 0; i < 0x80; i++)
    {
        uint8_t addr = (i << 1);

        stopCond();
        startCond();

        if (txByte(addr | I2C_READ))
        {
            log("i2c::scan: addr 0x%02x: FOUND\n", addr);
        }

        stopCond();
    }
    log("i2c::scan: end\n");
}

void I2CSw::measureSpeed()
{
    int n = 200000;
    {
        uint32_t t0 = getMiliSeconds();
        for (int i = 0; i < n; i++)
        {
            txBit(0);
        }
        uint32_t dt = getMiliSeconds() - t0;

        int khz = (dt > 0 ? n / dt : 0);
        log("I2CSw: n=%d, dt=%d, TX SPEED %d KHz\n", n, dt, khz);
    }
    {
        uint32_t t0 = getMiliSeconds();
        for (int i = 0; i < n; i++)
        {
            rxBit();
        }
        uint32_t dt = getMiliSeconds() - t0;

        int khz = (dt > 0 ? n / dt : 0);
        log("I2CSw: n=%d, dt=%d, RX SPEED %d KHz\n", n, dt, khz);
    }
}
