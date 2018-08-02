#include "timer.h"

#include <stm32f4xx_tim.h>
#include "format.hpp"
#include "mytime.h"
#include "uart.hpp"


static uint32_t g_rccPeriph[TID_NUM] = {
    RCC_APB1Periph_TIM2,
    RCC_APB1Periph_TIM3,
    RCC_APB1Periph_TIM4,
    RCC_APB1Periph_TIM5,
};

TIM_TypeDef* g_TIMx[TID_NUM] = {
    TIM2,
    TIM3,
    TIM4,
    TIM5,
};

uint8_t g_irqChannel[TID_NUM] = {
    TIM2_IRQn,
    TIM3_IRQn,
    TIM4_IRQn,
    TIM5_IRQn,
};


TimerMgr g_timerMgr;


TimerMgr::TimerMgr()
{
    memset(handlers, 0, sizeof(handlers));
}

void TimerMgr::registerHandler(TimerId tid, TimerEventHandler *handler)
{
    ASSERT(tid < TID_NUM, "timer id is out of range!");
    handlers[tid] = handler;
}

void TimerMgr::start(TimerId tid, uint32_t period)
{
    ASSERT(tid < TID_NUM, "timer id is out of range!");
    ASSERT(period > 0, "timer period is out of range!");

    // CPU freq:        168 MHz
    // ClockDivision:     4
    // TIMER freq:       42 MHz
    // Prescaler:     42000
    // 1 period:          1 ms

    RCC_APB1PeriphClockCmd(g_rccPeriph[tid], ENABLE);

    TIM_TimeBaseInitTypeDef timerInitStructure;
    timerInitStructure.TIM_Prescaler = 42000;
    timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    timerInitStructure.TIM_Period = period * 2;
    timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV4;
    timerInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(g_TIMx[tid], &timerInitStructure);

    // That last function caused the UIF flag to get set. Clear it.
    TIM_ClearITPendingBit(g_TIMx[tid], TIM_IT_Update);

    TIM_Cmd(g_TIMx[tid], ENABLE);

    TIM_ITConfig(g_TIMx[tid], TIM_IT_Update, ENABLE);

    NVIC_InitTypeDef nvicStructure;
    nvicStructure.NVIC_IRQChannel = g_irqChannel[tid];
    nvicStructure.NVIC_IRQChannelPreemptionPriority = 0;
    nvicStructure.NVIC_IRQChannelSubPriority = 1;
    nvicStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvicStructure);


    //t0 = getMiliSeconds();
    //log("Timer::start: t=%u, period=%u\n", t0, period);
}

void TimerMgr::onIrq(TimerId tid)
{
    ASSERT(tid < TID_NUM, "timer id is out of range!");

    if (TIM_GetITStatus(g_TIMx[tid], TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(g_TIMx[tid], TIM_IT_Update);
        TIM_Cmd(g_TIMx[tid], DISABLE);

        //uint32_t t = getMiliSeconds();
        //uint32_t dt = t - t0;
        //log("Timer::onIrq: t=%u, dt=%u\n", t, dt);

        if (handlers[tid])
        {
            handlers[tid]->onTimer(tid);
        }
    }
}

extern "C" void TIM2_IRQHandler()
{
    g_timerMgr.onIrq(TID_TIM2);
}

extern "C" void TIM3_IRQHandler()
{
    g_timerMgr.onIrq(TID_TIM3);
}

extern "C" void TIM4_IRQHandler()
{
    g_timerMgr.onIrq(TID_TIM4);
}

extern "C" void TIM5_IRQHandler()
{
    g_timerMgr.onIrq(TID_TIM5);
}
