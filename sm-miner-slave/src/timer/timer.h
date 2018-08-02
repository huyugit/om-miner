#ifndef STM_TIMER_H
#define STM_TIMER_H

#include <stdint.h>

enum TimerId {
    TID_TIM2 = 0,
    TID_TIM3,
    TID_TIM4,
    TID_TIM5,
    TID_NUM,
};

struct TimerEventHandler
{
    virtual void onTimer(TimerId tid) = 0;
};

class TimerMgr
{
public:
    TimerMgr();

    void registerHandler(TimerId tid, TimerEventHandler *handler);
    void start(TimerId tid, uint32_t period);

    void onIrq(TimerId tid);

private:
    TimerEventHandler* handlers[TID_NUM];
};

extern TimerMgr g_timerMgr;

#endif // STM_TIMER_H
