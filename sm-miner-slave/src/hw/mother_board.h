#ifndef MOTHERBOARD_H
#define MOTHERBOARD_H

#include <stdint.h>
#include "blinking_led.h"
#include "pwr_switch.h"
#include "i2c_sw.h"
#include "ms_data.h"


enum BoardAdcLine
{
    BOARD_ADC_LINE_REV = 0,
    BOARD_ADC_LINE_I0,
    BOARD_ADC_LINE_U,
    BOARD_ADC_LINE_I1,
    MAX_BOARD_ADC
};


class MotherBoard
{
public:
    MotherBoard();

    void init();

    static void resetWatchDog();

    void loadBoardsAdc();
    void logBoardsAdc();

    uint16_t getBoardsAdc(uint32_t boardId, BoardAdcLine adcLine);

    uint32_t generateRandom() const;

    bool loadHeaterErrPin(uint32_t boardId);

    void setupFan(FanConfig &cfg);

    // LEDs
    OneColorLed greenLed;
    OneColorLed redLed;
    TwoColorLed boardLeds[MAX_BOARD_PER_SLAVE];

    PwrSwitch pwrSwitch[MAX_BOARD_PER_SLAVE];
    I2CSw i2cBoards[MAX_BOARD_PER_SLAVE];

    SlaveMbInfo mbInfo;

    void ledsTick();

    McuUID uid;

private:
    bool initDone;
    uint16_t adcValues[MAX_BOARD_PER_SLAVE][MAX_BOARD_ADC];

    void initRandom();
    void initHwVer();
    void initI2C();
    void initPwrSwitch();
    void initWatchDog();
    void initAdcSwitch();
    void initAdcPAx();
    void initAdcPCx();
    void initFan();
    void initLeds();

    uint16_t readBoardAdc(uint8_t adcChannel);
};

extern MotherBoard g_motherBoard;


void exceptionNotifierUT();

#endif // MOTHERBOARD_H
