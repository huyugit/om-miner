#ifndef POLLING_TIMER_H
#define POLLING_TIMER_H

#include <stdint.h>


class PollingTimer
{
public:
    PollingTimer(int _period);

    bool inProgress();
    bool testAndRestart();

private:
    int period;
    uint32_t startTime;
};

#endif // POLLING_TIMER_H
