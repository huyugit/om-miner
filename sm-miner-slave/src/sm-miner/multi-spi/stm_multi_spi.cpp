#include "stm_multi_spi.h"

#include <cstring>
#include "format.hpp"
#include "stm_gpio.h"
#include "mytime.h"
#include "ms-protocol/ms_defines.h"
#include "mother_board_pins.h"
#include "utils.h"


namespace {
inline void delayByNops(volatile uint32_t n) {
    // We are going to make short delay for several CPU cycles
    for (uint32_t i=0; i < n; i++) {
        __NOP();
    }
}
}


MultySPI g_spiExchange;


SpiDrv::SpiDrv()
    : gpioClk(nullptr), gpioOut(nullptr), gpioIn(nullptr),
      maskClk(0), maskOut(0), maskIn(0)
{}

void SpiDrv::addSpi(const SpiPins &pins)
{
    GPIO_TypeDef* gpioClk2 = StmGPIO::gpioToGpioTypeDef(pins.clk);
    GPIO_TypeDef* gpioOut2 = StmGPIO::gpioToGpioTypeDef(pins.out);
    GPIO_TypeDef* gpioIn2  = StmGPIO::gpioToGpioTypeDef(pins.in);

    if ((gpioClk && (gpioClk != gpioClk2)) ||
        (gpioOut && (gpioOut != gpioOut2)) ||
        (gpioIn  && (gpioIn  != gpioIn2)))
    {
        log("MultySPI: can not aggregate pins!\n");
        return;
    }

    gpioClk = gpioClk2;
    gpioOut = gpioOut2;
    gpioIn  = gpioIn2;

    maskOut |= 0x1 << StmGPIO::gpioToPin(pins.out);
    maskClk |= 0x1 << StmGPIO::gpioToPin(pins.clk);
    maskIn  |= 0x1 << StmGPIO::gpioToPin(pins.in);
}

void SpiDrv::clk0() const { gpioClk->BSRRH = maskClk; }
void SpiDrv::clk1() const { gpioClk->BSRRL = maskClk; }
void SpiDrv::out0() const { gpioOut->BSRRH = maskOut; }
void SpiDrv::out1() const { gpioOut->BSRRL = maskOut; }

void SpiDrv::resetLine()
{
    gpioOut->BSRRH = maskOut; // out => 0

    for (uint32_t i = 0; i < g_slaveCfg.spiResetCycles; i++)
    {
        gpioClk->BSRRL = maskClk; // clk => 1

        delayByNops(g_slaveCfg.spiTimeRst);

        gpioClk->BSRRH = maskClk; // clk => 0

        delayByNops(g_slaveCfg.spiTimeRst);
    }

    gpioOut->BSRRL = maskOut; // out => 1

    txBit(1); // IMPORTANT!!!
    txBit(1); // IMPORTANT!!!
}

void SpiDrv::txBit(bool bit)
{
    delayByNops(g_slaveCfg.spiTimeTx0);

    if (bit)
        gpioOut->BSRRL = maskOut;
    else
        gpioOut->BSRRH = maskOut;

    delayByNops(g_slaveCfg.spiTimeTx1);

    gpioClk->BSRRL = maskClk;

    delayByNops(g_slaveCfg.spiTimeTx2);

    gpioClk->BSRRH = maskClk;
}

bool SpiDrv::rxBit()
{
    delayByNops(g_slaveCfg.spiTimeRx0);

    gpioClk->BSRRL = maskClk; // clk => 1

    delayByNops(g_slaveCfg.spiTimeRx1);

    int result = gpioIn->IDR & maskIn; // read in

    gpioClk->BSRRH = maskClk; // clk => 0
    return result;
}



MultySPI::MultySPI()
    : debugOn(false), debugBits(false), spiNum(0)
{
//    debugOn = true;
}

bool MultySPI::init(uint32_t spiMask)
{
    spiNum = 0;

    for (int spiLine = 0; spiLine < MAX_SPI_PER_SLAVE; spiLine++)
    {
        if (spiMask & (1 << spiLine))
        {
            log("Mapping SPI: grid spi [%d] => stm spi [%d]\n", spiNum, spiLine);
            pins[spiNum] = MotherBoardPins::spiPins[spiLine];
            spiNum++;
        }
    }

    for (uint8_t i = 0; i < spiNum; i++)
    {
        if (!g_StmGPIO.configurePin( pins[i].clk, kOUTPUT ))
            return false;
        if (!g_StmGPIO.configurePin( pins[i].out, kOUTPUT ))
            return false;
        if (!g_StmGPIO.configurePin( pins[i].in, kINPUT ))
            return false;

        spiDrv[i].addSpi(pins[i]);
        allSpiDrv.addSpi(pins[i]);
    }

    return true;
}

SpiDrv& MultySPI::getSpiDrv(uint8_t spiId)
{
    if (0) {
        g_slaveCfg.spiTimeRst = 2;

        g_slaveCfg.spiTimeTx0 = 2;
        g_slaveCfg.spiTimeTx1 = 2;
        g_slaveCfg.spiTimeTx2 = 4;

        g_slaveCfg.spiTimeRx0 = 10;
        g_slaveCfg.spiTimeRx1 = 30;
    }

    return (spiId != ALL_SPI) ? spiDrv[spiId] : allSpiDrv;
}

void MultySPI::resetLine(uint8_t spiId)
{
    SpiDrv &drv = getSpiDrv(spiId);
    drv.resetLine();
}

void MultySPI::tx(uint8_t spiId, uint8_t *buff, uint32_t size)
{
//    log("SPI[%d]: TX: ", spiId); hexdump8(buff, size);

    SpiDrv &drv = getSpiDrv(spiId);

    for (int i = 0; i < 3; i++) {
        drv.txBit(1); // HW BUG FIX
    }

    for (size_t pos = 0; pos < size; pos++)
    {
        uint8_t &data = buff[pos];

        drv.txBit(0); // START bit

        for (int i = 7; i >= 0; i--)
        {
            drv.txBit(data & (1 << i));
        }

        drv.txBit(1); // STOP bit
    }
}

SpiRxResult MultySPI::rx(uint8_t spiId, uint8_t *buff, uint32_t size, uint32_t &received)
{
    SpiDrv &drv = getSpiDrv(spiId);

    uint8_t rxLine = 0;
    received = 0;

    for (size_t pos = 0; pos < size; pos++, received++)
    {
        if (pos == 0)
        {
            // load byte and choose RX line (one of two possible input lines)
            SpiRxResult res = rxByte2(drv, rxLine, buff[pos]);
            if (res != SPI_RX_OK) {
                return res;
            }
        }
        else
        {
            SpiRxResult res = rxByte(drv, rxLine, buff[pos]);
            if (res != SPI_RX_OK) {
                return res;
            }
        }
    }
    return SPI_RX_OK;
}


class UartRxMachine
{
public:
    UartRxMachine()
        : state(STATE_START_BIT), data(0)
    {}

    void pushBit(uint8_t bit)
    {
//        log("pushBit: %d, state = %d, data = 0x%02x\n", bit, state, data);

        if (state == STATE_START_BIT) // waiting START bit (0)
        {
            if (bit == 0)
            {
                state++;
            }
        }
        else if (state <= STATE_BIT_7) // data bit received
        {
            if (bit)
            {
                uint8_t bitIndex = STATE_BIT_7 - state;
                data |= (1 << bitIndex);
            }

            state++;
        }
        else if (state == STATE_STOP_BIT) // STOP bit received
        {
            if (bit != 0)
                state = STATE_OK;
            else
                state = STATE_ERROR;
        }
    }

    inline bool isOk() const {
        return state == STATE_OK;
    }

    inline bool isError() const {
        return state == STATE_ERROR;
    }

    inline uint8_t getData() const {
        return data;
    }

    void dump() {
        log("state = %d, data = 0x%02x\n", state, data);
    }

private:
    static const uint8_t STATE_START_BIT    = 0;
    static const uint8_t STATE_BIT_0        = 1;
    static const uint8_t STATE_BIT_7        = 8;
    static const uint8_t STATE_STOP_BIT     = 9;
    static const uint8_t STATE_OK           = 10;
    static const uint8_t STATE_ERROR        = 11;

    uint8_t state;
    uint8_t data;
};


inline SpiRxResult MultySPI::rxByte2(SpiDrv &drv, uint8_t &rxLine, uint8_t &b)
{
    rxLine = 0;
    b = 0;

    drv.out1();
    drv.clk0();

    UartRxMachine rxMachine0, rxMachine1;

    for (uint32_t i = 0; i <= TIMEOUT; i++)
    {
        uint8_t bits[2];
        bits[0] = bits[1] = drv.rxBit();
        if (debugBits) log("%u%u ", bits[0], bits[1]);

        rxMachine0.pushBit(bits[0]);
        rxMachine1.pushBit(bits[1]);

        if (rxMachine0.isOk())
        {
            rxLine = 0;
            b = rxMachine0.getData();
            if (debugBits) log("\n");
            if (debugOn) log("SPI: rxBytes2 OK - LINE 0: 0x%02x!\n", b);
//            log("LINE 1: "); rxMachine1.dump();
            return SPI_RX_OK;
        }

        if (rxMachine1.isOk())
        {
            rxLine = 1;
            b = rxMachine1.getData();
            if (debugBits) log("\n");
            if (debugOn) log("SPI: rxBytes2 OK - LINE 1: 0x%02x!\n", b);
//            STOP();
            return SPI_RX_OK;
        }

        if (rxMachine0.isError() && rxMachine1.isError())
        {
            if (debugBits) log("\n");
            if (debugOn) log("SPI: SPI_RX_FRAME_ERROR\n");
            return SPI_RX_FRAME_ERROR;
        }
    }

    if (debugBits) log("\n");
    if (debugOn) log("SPI: rx timeout!\n");
    return SPI_RX_TIMEOUT;
}

inline SpiRxResult MultySPI::rxByte(SpiDrv &drv, uint8_t rxLine, uint8_t &b)
{
    b = 0;

    drv.out1();
    drv.clk0();

    uint8_t bits[2];
    UartRxMachine rxMachine;

    if (debugBits) log("SPI[%d]: ", drv);
    for (uint32_t i = 0; i <= TIMEOUT; i++)
    {
        //rxBit2(spiId, bits);
        bits[0] = bits[1] = drv.rxBit();

        rxMachine.pushBit(bits[rxLine]);
        if (debugBits) log("%u%u ", bits[0], bits[1]);

        if (rxMachine.isOk())
        {
            b = rxMachine.getData();

            if (debugBits) log("\n");
            if (debugOn) log("SPI[%d]: rx OK: 0x%02x!\n", drv, b);
            return SPI_RX_OK;
        }

        if (rxMachine.isError())
        {
            b = rxMachine.getData();

            if (debugBits) log("\n");
            if (debugOn) log("SPI[%d]: rx frame error (b = 0x%02x)!\n", drv, b);
            return SPI_RX_FRAME_ERROR;
        }
    }

    if (debugBits) log("\n");
    if (debugOn) log("SPI[%d]: rx timeout!\n", drv);
    return SPI_RX_TIMEOUT;
}


void MultySPI::measureSpeed()
{
    SpiDrv &drv = getSpiDrv(ALL_SPI);
    {
        int n = 200;

        uint32_t t0 = getMiliSeconds();
        for (int i = 0; i < n; i++)
        {
            drv.resetLine();
        }
        uint32_t dt = getMiliSeconds() - t0;

        int khz = (dt > 0 ? n * g_slaveCfg.spiResetCycles / dt : 0);
        log("measureSpeed: resetLine: n=%d, dt=%d, %d KHz\n", n, dt, khz);
    }
    {
        int n = 100000;

        uint32_t t0 = getMiliSeconds();
        for (int i = 0; i < n; i++)
        {
            drv.txBit(0);
        }
        uint32_t dt = getMiliSeconds() - t0;

        int khz = (dt > 0 ? n / dt : 0);
        log("measureSpeed: TX: n=%d, dt=%d, TX SPEED %d KHz\n", n, dt, khz);
    }
    {
        int n = 100000;

        uint32_t t0 = getMiliSeconds();
        for (int i = 0; i < n; i++)
        {
            drv.rxBit();
        }
        uint32_t dt = getMiliSeconds() - t0;

        int khz = (dt > 0 ? n / dt : 0);
        log("measureSpeed: RX: n=%d, dt=%d, RX SPEED %d KHz\n", n, dt, khz);
    }

    if (0)
    {
        for (int delay = 1; delay < 20; delay++)
        {
            if (delay > 10) delay++;

            g_slaveCfg.spiTimeRst = delay;

            int n = 200;

            uint32_t t0 = getMiliSeconds();
            for (int i = 0; i < n; i++)
            {
                drv.resetLine();
            }
            uint32_t dt = getMiliSeconds() - t0;

            int khz = (dt > 0 ? n * g_slaveCfg.spiResetCycles / dt : 0);
            log("measureSpeed: resetLine: delay=%d, n=%d, dt=%d, %d KHz\n",
                g_slaveCfg.spiTimeRst,
                n, dt, khz);
        }

        uint16_t delayList[] = {
            1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 16, 18, 20,
            25, 30, 35, 40, 45, 50, 60, 70, 80, 90
        };
        for (uint32_t j = 0; j < ARRAY_SIZE(delayList); j++)
        {
            g_slaveCfg.spiTimeTx0 = delayList[j];
            g_slaveCfg.spiTimeTx1 = delayList[j];
            g_slaveCfg.spiTimeTx2 = delayList[j] * 2;

            int n = 50000;

            uint32_t t0 = getMiliSeconds();
            for (int i = 0; i < n; i++)
            {
                drv.txBit(0);
            }
            uint32_t dt = getMiliSeconds() - t0;

            int khz = (dt > 0 ? n / dt : 0);
            log("measureSpeed: TX: delay=%d/%d/%d, n=%d, dt=%d, %d KHz\n",
                g_slaveCfg.spiTimeTx0, g_slaveCfg.spiTimeTx1, g_slaveCfg.spiTimeTx2,
                n, dt, khz);
        }

        for (int delay = 10; delay < 50; delay += 5)
        {
            g_slaveCfg.spiTimeRx0 = delay;
            g_slaveCfg.spiTimeRx1 = delay * 3;

            int n = 10000;

            uint32_t t0 = getMiliSeconds();
            for (int i = 0; i < n; i++)
            {
                drv.rxBit();
            }
            uint32_t dt = getMiliSeconds() - t0;

            int khz = (dt > 0 ? n / dt : 0);
            log("measureSpeed: RX: delay=%d/%d, n=%d, dt=%d, %d KHz\n",
                g_slaveCfg.spiTimeRx0, g_slaveCfg.spiTimeRx1,
                n, dt, khz);
        }

        STOP();
    }
}

void MultySPI::testSignal()
{
    log("testSignal: endless loop...");

    SpiDrv &drv = getSpiDrv(ALL_SPI);
    while (0)
    {
        for (int i = 0; i < 4; i++) drv.txBit(0);
        for (int i = 0; i < 4; i++) drv.txBit(1);
    }
}

//measureSpeed: resetLine: n=200, dt=18, 5688 KHz
//measureSpeed: TX: n=100000, dt=52, TX SPEED 1923 KHz
//measureSpeed: RX: n=100000, dt=124, RX SPEED 806 KHz
//measureSpeed: resetLine: delay=1, n=200, dt=12, 8533 KHz
//measureSpeed: resetLine: delay=2, n=200, dt=18, 5688 KHz
//measureSpeed: resetLine: delay=3, n=200, dt=24, 4266 KHz
//measureSpeed: resetLine: delay=4, n=200, dt=30, 3413 KHz
//measureSpeed: resetLine: delay=5, n=200, dt=37, 2767 KHz
//measureSpeed: resetLine: delay=6, n=200, dt=42, 2438 KHz
//measureSpeed: resetLine: delay=7, n=200, dt=48, 2133 KHz
//measureSpeed: resetLine: delay=8, n=200, dt=54, 1896 KHz
//measureSpeed: resetLine: delay=9, n=200, dt=61, 1678 KHz
//measureSpeed: resetLine: delay=10, n=200, dt=66, 1551 KHz
//measureSpeed: resetLine: delay=12, n=200, dt=78, 1312 KHz
//measureSpeed: resetLine: delay=14, n=200, dt=91, 1125 KHz
//measureSpeed: resetLine: delay=16, n=200, dt=103, 994 KHz
//measureSpeed: resetLine: delay=18, n=200, dt=116, 882 KHz
//measureSpeed: resetLine: delay=20, n=200, dt=127, 806 KHz
//measureSpeed: TX: delay=1/1/2, n=50000, dt=21, 2380 KHz
//measureSpeed: TX: delay=2/2/4, n=50000, dt=26, 1923 KHz
//measureSpeed: TX: delay=3/3/6, n=50000, dt=32, 1562 KHz
//measureSpeed: TX: delay=4/4/8, n=50000, dt=38, 1315 KHz
//measureSpeed: TX: delay=5/5/10, n=50000, dt=45, 1111 KHz
//measureSpeed: TX: delay=6/6/12, n=50000, dt=50, 1000 KHz
//measureSpeed: TX: delay=7/7/14, n=50000, dt=56, 892 KHz
//measureSpeed: TX: delay=8/8/16, n=50000, dt=62, 806 KHz
//measureSpeed: TX: delay=9/9/18, n=50000, dt=68, 735 KHz
//measureSpeed: TX: delay=10/10/20, n=50000, dt=74, 675 KHz
//measureSpeed: TX: delay=12/12/24, n=50000, dt=86, 581 KHz
//measureSpeed: TX: delay=14/14/28, n=50000, dt=97, 515 KHz
//measureSpeed: TX: delay=16/16/32, n=50000, dt=110, 454 KHz
//measureSpeed: TX: delay=18/18/36, n=50000, dt=122, 409 KHz
//measureSpeed: TX: delay=20/20/40, n=50000, dt=133, 375 KHz
//measureSpeed: TX: delay=25/25/50, n=50000, dt=163, 306 KHz
//measureSpeed: TX: delay=30/30/60, n=50000, dt=193, 259 KHz
//measureSpeed: TX: delay=35/35/70, n=50000, dt=223, 224 KHz
//measureSpeed: TX: delay=40/40/80, n=50000, dt=253, 197 KHz
//measureSpeed: TX: delay=45/45/90, n=50000, dt=282, 177 KHz
//measureSpeed: TX: delay=50/50/100, n=50000, dt=312, 160 KHz
//measureSpeed: TX: delay=60/60/120, n=50000, dt=372, 134 KHz
//measureSpeed: TX: delay=70/70/140, n=50000, dt=431, 116 KHz
//measureSpeed: TX: delay=80/80/160, n=50000, dt=491, 101 KHz
//measureSpeed: TX: delay=90/90/180, n=50000, dt=550, 90 KHz
//measureSpeed: RX: delay=10/30, n=10000, dt=13, 769 KHz
//measureSpeed: RX: delay=15/45, n=10000, dt=18, 555 KHz
//measureSpeed: RX: delay=20/60, n=10000, dt=25, 400 KHz
//measureSpeed: RX: delay=25/75, n=10000, dt=30, 333 KHz
//measureSpeed: RX: delay=30/90, n=10000, dt=37, 270 KHz
//measureSpeed: RX: delay=35/105, n=10000, dt=42, 238 KHz
//measureSpeed: RX: delay=40/120, n=10000, dt=48, 208 KHz
//measureSpeed: RX: delay=45/135, n=10000, dt=55, 181 KHz
