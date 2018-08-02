#include "polling_timer.h"

#include "mytime.h"

PollingTimer::PollingTimer(int _period)
    : period(_period), startTime(0)
{
    startTime = getMiliSeconds();
}

bool PollingTimer::inProgress()
{
    return (getMiliSeconds() < startTime + period);
}

bool PollingTimer::testAndRestart()
{
    uint32_t now = getMiliSeconds();

    if (now < startTime + period)
    {
        return false;
    }
    else {
        startTime = now;
        return true;
    }
}
