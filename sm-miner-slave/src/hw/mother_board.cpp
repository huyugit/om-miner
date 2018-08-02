#include "mother_board.h"

#include "common.h"
#include "stm32f4xx.h"
#include "stm32f4xx_it.h"
#include "stm32f4xx_iwdg.h"
#include "uart.hpp"
#include "stm_gpio.h"
#include "mytime.h"
#include "format.hpp"
#include "mother_board_pins.h"
#include "blinking_led.h"
#include "data_noise_filter.h"
#include "stm_multi_spi.h"
#include "i2c_board_access.h"
#include "pwrmodule_mgr.h"

MotherBoard g_motherBoard;

void sysTickDivBy100Handler()
{
    g_motherBoard.ledsTick();
	g_ModuleMgr.process100ms();
}


void exceptionNotifierLed(bool on, int len)
{
    g_StmGPIO.writePin(MotherBoardPins::greenLedPin, !on);
    g_StmGPIO.writePin(MotherBoardPins::redLedPin, !on);

    static volatile int x = 0;
    for (int i = 0; i < 2000000 * len; i++)
    {
        x += i;
    }
}

void exceptionNotifier(int id)
{
    const int SHORT = 2;
    const int LONG  = 10;

    for (int step = 0; step < 5; step++)
    {
        log("EXCEPTION ID: %d\n", id);
    }

    exceptionNotifierLed(1, LONG);
    exceptionNotifierLed(0, SHORT);

    for (int i = 0; i < id; i++)
    {
        exceptionNotifierLed(1, SHORT);
        exceptionNotifierLed(0, SHORT);
    }
}

void exceptionNotifierUT()
{
    // genegate Hard Fault exception
    volatile int* p = (int*)0x30003000;
    *p = 12345;
}


void checkRebootReason()
{
    const char* reason = "N/A";

    if (RCC_GetFlagStatus(RCC_FLAG_BORRST) == SET)
    {
        reason = "POR/PDR or BOR";
    }
    if (RCC_GetFlagStatus(RCC_FLAG_PINRST) == SET)
    {
        reason = "Pin";
    }
    if (RCC_GetFlagStatus(RCC_FLAG_PORRST) == SET)
    {
        reason = "POR/PDR";
    }
    if (RCC_GetFlagStatus(RCC_FLAG_SFTRST) == SET)
    {
        reason = "Software";
    }
    if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) == SET)
    {
        reason = "Independent Watchdog";
    }
    if (RCC_GetFlagStatus(RCC_FLAG_WWDGRST) == SET)
    {
        reason = "Window Watchdog";
    }
    if (RCC_GetFlagStatus(RCC_FLAG_LPWRRST) == SET)
    {
        reason = "Low Power";
    }

    log("RESET REASON: %s\n", reason);

    RCC_ClearFlag();
}


MotherBoard::MotherBoard()
    : initDone(false)
{
    // load uid
    uid = *(McuUID*)0x1FFF7A10;
}

void MotherBoard::init()
{
    SysTick_Config(SystemCoreClock / 1000);  // 1000 Hz

    // uart init
    const uint32_t size = 8*1024;
    const uint32_t size2 = 4*1024;

    uartDebug.init_tx_rb(g_staticAllocator.alloc(size), size);
    uartDebug2.init_tx_rb(g_staticAllocator.alloc(size2), size2);

    uartDebug.init(230400);
    uartDebug2.init(115200);

    initHwVer();
    initRandom();
    initI2C();
    initPwrSwitch();
    initAdcSwitch();
    initAdcPAx();
    initAdcPCx();

    initLeds();
    initFan();
    initWatchDog();

    initDone = true;

    log("SystemCoreClock = %u\n", SystemCoreClock);

    checkRebootReason();

    // enable FPU
    SCB->CPACR |= (0x3 << 10 * 2 | 0x3 << 11 * 2);
}

//-------------------------------------------------------
//-----                RANDOM                       -----
//-------------------------------------------------------

void MotherBoard::initRandom()
{
    RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);
    RNG_Cmd(ENABLE);
}

uint32_t MotherBoard::generateRandom() const
{
    while (RNG_GetFlagStatus(RNG_FLAG_DRDY) == RESET) {}
    return RNG_GetRandomNumber();
}

//-------------------------------------------------------
//-----                HW VER                       -----
//-------------------------------------------------------

void MotherBoard::initHwVer()
{
    for (int i = 0; i < MotherBoardPins::HW_VER_PINS; i++)
    {
        uint8_t pin = MotherBoardPins::hwVerPins[i];

        g_StmGPIO.configurePin(pin, kINPUT);
        mbInfo.hwVer |= g_StmGPIO.readPin(pin) << i;
    }
}

//-------------------------------------------------------
//-----                  I2C                        -----
//-------------------------------------------------------

void MotherBoard::initI2C()
{
    for (int i = 0; i < MAX_BOARD_PER_SLAVE; i++)
    {
        i2cBoards[i].init(MotherBoardPins::i2cScl[i], MotherBoardPins::i2cSda[i]);
    }
}

//-------------------------------------------------------
//-----                PwrSwitch                    -----
//-------------------------------------------------------

void MotherBoard::initPwrSwitch()
{
    for (int i = 0; i < MAX_BOARD_PER_SLAVE; i++)
    {
        pwrSwitch[i].init(MotherBoardPins::pwrSwOn[i], MotherBoardPins::pwrSwClk[i]);
    }
}

//-------------------------------------------------------
//-----               Watch Dog                     -----
//-------------------------------------------------------

void MotherBoard::initWatchDog()
{
    // 40KHz / 256 / 0xfff => 26 sec

    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_256);
    IWDG_SetReload(0x0fff);
    IWDG_ReloadCounter();

    IWDG_Enable();
}

void MotherBoard::resetWatchDog()
{
    IWDG_ReloadCounter();
}

//-------------------------------------------------------
//-----             Board ADC Mux                   -----
//-------------------------------------------------------

void MotherBoard::initAdcSwitch()
{
    g_StmGPIO.configurePin(MotherBoardPins::adcMbSw, kOUTPUT);

    for (int boardId = 0; boardId < MAX_BOARD_PER_SLAVE; boardId++)
    {
        g_StmGPIO.configurePin(MotherBoardPins::adcSw0Pins[boardId], kOUTPUT);
        g_StmGPIO.configurePin(MotherBoardPins::adcSw1Pins[boardId], kOUTPUT);
    }
}

void MotherBoard::loadBoardsAdc()
{
    //log("loadBoardsAdc()\n");
    memset(adcValues, 0, sizeof(adcValues));

    for (int sw = 0; sw < (MAX_BOARD_ADC / 2); sw++)
    {
        // set board's adc switch
        for (int boardId = 0; boardId < MAX_BOARD_PER_SLAVE; boardId++)
        {
            g_StmGPIO.writePin(MotherBoardPins::adcSw0Pins[boardId], (sw >> 0) & 1);
            g_StmGPIO.writePin(MotherBoardPins::adcSw1Pins[boardId], (sw >> 1) & 1);

            for (int mbSw = 0; mbSw < 2; mbSw++)
            {
                g_StmGPIO.writePin(MotherBoardPins::adcMbSw, !mbSw);

                uint16_t v = g_motherBoard.readBoardAdc( MotherBoardPins::boardAdcCh[boardId] );
                adcValues[boardId][sw*2 + mbSw] = v;

                //log("BOARD[%d]:  SW=%d/%d  ADC=%4d\n", boardId, sw, i, v);
            }
        }
    }

    mbInfo.adc50V = g_motherBoard.readBoardAdc( MotherBoardPins::adcCh50V );
    mbInfo.fan.fanAdcU = g_motherBoard.readBoardAdc( MotherBoardPins::adcChFanU );
    mbInfo.fan.fanAdcI = g_motherBoard.readBoardAdc( MotherBoardPins::adcChFanI );

    logBoardsAdc();
}

void MotherBoard::logBoardsAdc()
{
    if (g_slaveCfg.debugOpt & SLAVE_DEBUG_LOG_ADC)
    {
        log("--- BOARDS ADC:\n");
        log("MB 50V:   ADC: %4d\n", mbInfo.adc50V);
        log("FAN U:    ADC: %4d\n", mbInfo.fan.fanAdcU);
        log("FAN I:    ADC: %4d\n", mbInfo.fan.fanAdcI);
        for (int boardId = 0; boardId < MAX_BOARD_PER_SLAVE; boardId++)
        {
            log("BOARD[%d]: ADC: ", boardId);
            for (int adcId = 0; adcId < MAX_BOARD_ADC; adcId++)
            {
                log("%4d ", getBoardsAdc(boardId, (BoardAdcLine)adcId));
            }
            log("\n");
        }
    }
}

uint16_t MotherBoard::getBoardsAdc(uint32_t boardId, BoardAdcLine adcLine)
{
    return adcValues[boardId][adcLine];
}

uint16_t MotherBoard::readBoardAdc(uint8_t adcChannel)
{
    static volatile uint32_t x = 0;
    for (int i = 0; i < 2000; i++) { x += i; }

    DataNoiseFilter<int, 9> filter;

    ADC_RegularChannelConfig(ADC1, adcChannel, 1, ADC_SampleTime_84Cycles);

    for (size_t i = 0; i < filter.getSize(); i++)
    {
        ADC_SoftwareStartConv(ADC1);

        while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET) {
            __NOP();
        }

        uint16_t value = ADC_GetConversionValue(ADC1);

        if (0) {
            int mv = 3.3 * 1000 * value / (1<<12);
            log("readBoardAdcA: channel = %d, adc = %d (%d mv)\n", adcChannel, value, mv);
        }

        filter.push(value);
    }

    uint16_t value = filter.getAtMiddle();

    if (0) {
        int mv = 3.3 * 1000 * value / (1<<12);
        log("readBoardAdcB: channel = %d, adc = %d (%d mv)\n", adcChannel, value, mv);
    }
    return value;
}

//-------------------------------------------------------
//-----                    ADC                      -----
//-------------------------------------------------------

void MotherBoard::initAdcPAx()
{
    ASSERT(MotherBoardPins::adcCh50V  == ADC_Channel_0, "err adc");
    ASSERT(MotherBoardPins::adcChFanU == ADC_Channel_1, "err adc");
    ASSERT(MotherBoardPins::adcChFanI == ADC_Channel_2, "err adc");

    // Enable peripheral clocks
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    // Configure GPIO pins as analog input
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // ADC Common configuration
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    ADC_CommonStructInit(&ADC_CommonInitStructure);
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_CommonInit(&ADC_CommonInitStructure);

    // ADC regular channel configuration
    ADC_InitTypeDef ADC_InitStructure;
    ADC_StructInit(&ADC_InitStructure);
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_Init(ADC1, &ADC_InitStructure);

    // Enable ADC
    ADC_Cmd(ADC1, ENABLE);
}

void MotherBoard::initAdcPCx()
{
    for (int i = 0; i < MAX_BOARD_PER_SLAVE; i++)
    {
        ASSERT(MotherBoardPins::boardAdcCh[i] >= ADC_Channel_10, "err adc");
        ASSERT(MotherBoardPins::boardAdcCh[i] <= ADC_Channel_15, "err adc");
    }

    // Enable peripheral clocks
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    // Configure GPIO pins as analog input
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin =
            GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 |
            GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // ADC Common configuration
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    ADC_CommonStructInit(&ADC_CommonInitStructure);
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_CommonInit(&ADC_CommonInitStructure);

    // ADC regular channel configuration
    ADC_InitTypeDef ADC_InitStructure;
    ADC_StructInit(&ADC_InitStructure);
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_Init(ADC1, &ADC_InitStructure);

    // Enable ADC
    ADC_Cmd(ADC1, ENABLE);
}

bool MotherBoard::loadHeaterErrPin(uint32_t boardId)
{
    ASSERT(boardId < MAX_BOARD_PER_SLAVE, "err boardId");
    uint8_t pin = MotherBoardPins::hsErrPins[boardId];

    g_StmGPIO.configurePin(pin, kINPUT);
    return g_StmGPIO.readPin(pin) == 0;
}

//-------------------------------------------------------
//-----                  FANs                       -----
//-------------------------------------------------------

void MotherBoard::initFan()
{
    // init DAC
    ASSERT(MotherBoardPins::fanDac == PA4, "err dac pin");

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef gs;
    GPIO_StructInit(&gs);

    gs.GPIO_Pin  = GPIO_Pin_4;
    gs.GPIO_Mode = GPIO_Mode_AN;
    gs.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &gs);

    DAC_DeInit();

    DAC_InitTypeDef di;
    DAC_StructInit(&di);

    di.DAC_Trigger                      = DAC_Trigger_None;
    di.DAC_WaveGeneration               = DAC_WaveGeneration_None;
    di.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bit0;
    di.DAC_OutputBuffer                 = DAC_OutputBuffer_Enable;

    DAC_Init(DAC_Channel_1, &di);
    DAC_Cmd(DAC_Channel_1, ENABLE);

    // initial setup
    setupFan(g_slaveCfg.fanConfig);
}

void MotherBoard::setupFan(FanConfig &cfg)
{
    uint16_t dac = cfg.fanDac;
    dac = MAX(FanConfig::FAN_DAC_MIN, dac);
    dac = MIN(FanConfig::FAN_DAC_MAX, dac);

    log("SET FAN: dac %u\n", dac);

    DAC_SetChannel1Data(DAC_Align_12b_R, dac);
}

//-------------------------------------------------------
//-----                  LEDs                       -----
//-------------------------------------------------------

void MotherBoard::initLeds()
{
    greenLed.init(MotherBoardPins::greenLedPin);
    greenLed.setBlinkType(LED_ON);

    redLed.init(MotherBoardPins::redLedPin);
    redLed.setBlinkType(LED_BLINK_QUICK);

    for (int i = 0; i < MAX_BOARD_PER_SLAVE; i++)
    {
        boardLeds[i].init( MotherBoardPins::i2cSda[i], MotherBoardPins::i2cScl[i] );
        boardLeds[i].setBlinkType(LED_BLINK_SWITCH);
    }
}

void MotherBoard::ledsTick()
{
    if (initDone)
    {
        BlinkingLed::timerValue++;

        greenLed.onTimerTick();
        redLed.onTimerTick();

        if (I2CBoardAccess::sharedAccess.trylock())
        {
            MutexLocker lock(I2CBoardAccess::sharedAccess, false);

            for (int i = 0; i < MAX_BOARD_PER_SLAVE; i++)
            {
                TwoColorLed &led = boardLeds[i];
                led.onTimerTick();

                // led pins can be damaged due to I2C exchange
                led.setPins();
            }
        }
    }
}
