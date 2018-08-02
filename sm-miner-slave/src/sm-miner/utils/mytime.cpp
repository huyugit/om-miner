#include "mytime.h"

#include "stm32f4xx_it.h"
#include "format.hpp"

// get current time in mili-seconds since start
uint32_t getMiliSeconds()
{
    return getCurrentTicks();
}

uint32_t getSeconds()
{
    return getMiliSeconds() / 1000;
}

void mysleep(uint32_t msec)
{
    uint32_t now = getMiliSeconds();
    while (getMiliSeconds() - now < msec)
    {
        // active wait
    }
}
