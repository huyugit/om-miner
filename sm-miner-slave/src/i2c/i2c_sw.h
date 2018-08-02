#ifndef I2C_SW_H
#define I2C_SW_H

#include <stdint.h>


#define I2C_READ    1
#define I2C_WRITE   0


class I2CSw
{
public:
    I2CSw();
    void init(uint8_t _sclPin, uint8_t _sdaPin);

    void configure();

    void delay();

    void startCond();
    void stopCond();

    void txBit(uint8_t x);
    uint8_t rxBit();

    bool txByte(uint8_t x);
    void rxByte(uint8_t &x, bool isLast=false);

    bool readReg(uint8_t addr, uint8_t &data);
    bool writeReg(uint8_t addr, uint8_t data);

    bool writeRegAndValidate(uint8_t addr, uint8_t data);

    void scan();
    void measureSpeed();

    bool debug;

private:
    uint8_t sclPin, sdaPin;
};

#endif // I2C_SW_H
