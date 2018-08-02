#ifndef STM_MULTI_SPI_H
#define STM_MULTI_SPI_H

#include <stdint.h>
#include "common.h"
#include "mother_board_pins.h"
#include "stm_gpio.h"


enum SpiRxResult
{
    SPI_RX_OK           = 0x0,
    SPI_RX_TIMEOUT      = 0x1,
    SPI_RX_FRAME_ERROR  = 0x2,
};


struct SpiDrv
{
    GPIO_TypeDef* gpioClk;
    GPIO_TypeDef* gpioOut;
    GPIO_TypeDef* gpioIn;

    uint32_t maskClk;
    uint32_t maskOut;
    uint32_t maskIn;

    SpiDrv();
    void addSpi(const SpiPins &pins);

    inline void clk0() const;
    inline void clk1() const;
    inline void out0() const;
    inline void out1() const;

    void resetLine();
    void __attribute__ ((noinline)) txBit(bool bit);
    inline bool rxBit();
};


class MultySPI
{
public:
    static const uint8_t  ALL_SPI   = 0xff;
    static const uint32_t TIMEOUT   = 20;

public:
    MultySPI();
    bool init(uint32_t spiMask);

    void resetLine(uint8_t spiId);
    void tx(uint8_t spiId, uint8_t *buff, uint32_t size);
    SpiRxResult rx(uint8_t spiId, uint8_t *buff, uint32_t size, uint32_t &received);

    void measureSpeed();
    void testSignal();

    bool debugOn, debugBits;

private:
    int spiNum;

    SpiPins pins[MAX_SPI_PER_SLAVE];

    SpiDrv allSpiDrv;
    SpiDrv spiDrv[MAX_SPI_PER_SLAVE];

    SpiDrv& getSpiDrv(uint8_t spiId);

    SpiRxResult rxByte2(SpiDrv &drv, uint8_t &rxLine, uint8_t &b);
    SpiRxResult rxByte(SpiDrv &drv, uint8_t rxLine, uint8_t &b);
};

extern MultySPI g_spiExchange;

#endif // STM_MULTI_SPI_H
