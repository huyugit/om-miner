#ifndef MOTHER_BOARD_PINS_H
#define MOTHER_BOARD_PINS_H

#include <stdint.h>
#include "ms_defines.h"


struct SpiPins {
    uint8_t clk, out, in;
};


class MotherBoardPins
{
public:
    MotherBoardPins();

    static const uint8_t HW_VER_PINS = 4;
    static const uint8_t hwVerPins[HW_VER_PINS];

    static const uint8_t greenLedPin;
    static const uint8_t redLedPin;

    static const uint8_t adcMbSw;

    static const uint8_t fanDac;

    static const uint8_t adcCh50V;
    static const uint8_t adcChFanU;
    static const uint8_t adcChFanI;

    static const uint8_t i2cScl[MAX_BOARD_PER_SLAVE];
    static const uint8_t i2cSda[MAX_BOARD_PER_SLAVE];

    static const uint8_t pwrSwOn [MAX_BOARD_PER_SLAVE];
    static const uint8_t pwrSwClk[MAX_BOARD_PER_SLAVE];

    static const uint8_t adcSw0Pins[MAX_BOARD_PER_SLAVE];
    static const uint8_t adcSw1Pins[MAX_BOARD_PER_SLAVE];

    static const uint8_t boardAdcCh[MAX_BOARD_PER_SLAVE];

    static const SpiPins spiPins[MAX_SPI_PER_SLAVE];

    static const uint8_t hsErrPins[MAX_BOARD_PER_SLAVE];
};

#endif // MOTHER_BOARD_PINS_H
