#include "mother_board_pins.h"

#include "stm_gpio.h"


const uint8_t MotherBoardPins::hwVerPins[MotherBoardPins::HW_VER_PINS] = {
    PD15, PD14, PD13, PD12
};

const uint8_t MotherBoardPins::greenLedPin  = PD13;
const uint8_t MotherBoardPins::redLedPin    = PD14;

const uint8_t MotherBoardPins::adcMbSw      = PD12;

const uint8_t MotherBoardPins::fanDac       = PA4;

const uint8_t MotherBoardPins::adcCh50V     = ADC_Channel_0; // PA0
const uint8_t MotherBoardPins::adcChFanU    = ADC_Channel_1; // PA1
const uint8_t MotherBoardPins::adcChFanI    = ADC_Channel_2; // PA2


// Board I2C (shared with board LED)
const uint8_t MotherBoardPins::i2cScl[MAX_BOARD_PER_SLAVE] = {
    PB6, PB10, PA8, PB8, PB0, PB3
};
const uint8_t MotherBoardPins::i2cSda[MAX_BOARD_PER_SLAVE] = {
    PB7, PB11, PC9, PA3, PB1, PB4
};

// Board Power Switch (shared with board MOSI)
const uint8_t MotherBoardPins::pwrSwOn [MAX_BOARD_PER_SLAVE] = {
    PD1, PD3, PD5, PD7, PD9, PD11
};
const uint8_t MotherBoardPins::pwrSwClk[MAX_BOARD_PER_SLAVE] = {
    PB5, PD15, PB13, PC6, PB14, PB15
};

// Board ADC Switch
const uint8_t MotherBoardPins::adcSw0Pins[MAX_BOARD_PER_SLAVE] = {
    PB2,  PC13, PC15, PC8, PE15, PE13
};
const uint8_t MotherBoardPins::adcSw1Pins[MAX_BOARD_PER_SLAVE] = {
    PB12, PC12, PC14, PC7, PE14, PE12
};

// Board ADC
const uint8_t MotherBoardPins::boardAdcCh[MAX_BOARD_PER_SLAVE] = {
    ADC_Channel_10, // PC0
    ADC_Channel_11, // PC1
    ADC_Channel_12, // PC2
    ADC_Channel_13, // PC3
    ADC_Channel_14, // PC4
    ADC_Channel_15, // PC5
};

const SpiPins MotherBoardPins::spiPins[MAX_SPI_PER_SLAVE] = {
    //clk   out   in
    { PD0,  PD1,  PE0 },
    { PD2,  PD3,  PE1 },
    { PD4,  PD5,  PE2 },
    { PD6,  PD7,  PE3 },
    { PD8,  PD9,  PE4 },
    { PD10, PD11, PE5 },
};

const uint8_t MotherBoardPins::hsErrPins[MAX_BOARD_PER_SLAVE] = {
    PE6, PE7, PE8, PE9, PE10, PE11
};


MotherBoardPins::MotherBoardPins()
{
}
