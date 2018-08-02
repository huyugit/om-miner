#include "GpioPinOPi.h"

#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <cassert>

#include "except/ApplicationException.h"
#include "sys/Mutex.h"

namespace {
volatile uint8_t *gpio = nullptr;
Mutex gpioMutex;
}

#define PORT_CFG_BASE 0x00
#define PORT_DAT_BASE 0x10
#define PORT_CFG_PULL_BASE 0x1C

void GpioPinOPi::initMapping()
{
    const char* fileName = "/dev/mem";

    int fdGpio = open(fileName, O_RDWR | O_SYNC);
    if (fdGpio < 0) {
        throw SystemException("GpioPinOPi: unable to open %s", fileName);
    }

    gpio = (uint8_t*) mmap(NULL, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED, fdGpio, 0x01C20000);
    if (gpio == MAP_FAILED) {
        close(fdGpio);
        throw SystemException(errno, "GpioPinOPi: gpio mapping failed");
    }

    gpio += 0x800;

    if (0) {
        GpioPinOPi p2(10);
        for (int i = 0; i < 1000000; i++)
        {
            printf("set %u\n", i %2);
            p2.set(i % 2);
            usleep(500 * 1000);
        }
    }
}

uint8_t GpioPinOPi::pinToGpio(uint8_t pinNumber)
{
    uint8_t gpioNumber = 0;

    switch (pinNumber)
    {
    case  3: gpioNumber = 12; break;
    case  5: gpioNumber = 11; break;
    case  7: gpioNumber =  6; break;
  //case  8: gpioNumber =  x; break; // PG6
  //case 10: gpioNumber =  x; break; // PG7
    case 11: gpioNumber =  1; break;
    case 12: gpioNumber =  7; break;
    case 13: gpioNumber =  0; break;
    case 15: gpioNumber =  3; break;
    case 16: gpioNumber = 19; break;
    case 18: gpioNumber = 18; break;
    case 19: gpioNumber = 15; break;
    case 21: gpioNumber = 16; break;
    case 22: gpioNumber =  2; break;
    case 23: gpioNumber = 14; break;
    case 24: gpioNumber = 13; break;
    case 26: gpioNumber = 10; break;

    default:
        throw ApplicationException("GpioPinOPi: can't map pin %u to gpio", pinNumber);
    }

    //printf("GpioPinOPi: map pin %u to gpio %u\n", pinNumber, gpioNumber);
    return gpioNumber;
}

void GpioPinOPi::setupCfg(uint8_t gpioNumber, uint8_t flags)
{
    Mutex::Lock lock(gpioMutex);

    assert(gpio != nullptr);
    assert(gpioNumber < 32);

    volatile uint32_t* cfgReg = (uint32_t*)(gpio + PORT_CFG_BASE) + (gpioNumber / 8);
    uint32_t cfgShift = (gpioNumber % 8) * 4;

    //printf("gpio=%u, flags=0x%x, cfgReg=0x%08x, cfgShift=%u\n", gpioNumber, flags, cfgReg, cfgShift);

    assert(flags <= 0x7);
    uint32_t cfgMask  = (0x7 << cfgShift);
    uint32_t cfgValue = (flags << cfgShift);

    //printf("cfgReg1 = 0x%08x\n", *cfgReg);
    *cfgReg &= ~cfgMask;
    *cfgReg |= cfgValue;
    //printf("cfgReg2 = 0x%08x\n", *cfgReg);
}

void GpioPinOPi::setupPull(uint8_t gpioNumber, uint8_t flags)
{
    Mutex::Lock lock(gpioMutex);

    assert(gpio != nullptr);
    assert(gpioNumber < 32);

    volatile uint32_t* pullReg = (uint32_t*)(gpio + PORT_CFG_PULL_BASE) + (gpioNumber / 16);
    uint32_t pullShift = (gpioNumber % 16) * 2;

    //printf("gpio=%u, flags=0x%x, pullReg=0x%08x, pullShift=%u\n", gpioNumber, flags, pullReg, pullShift);

    assert(flags <= 0x3);
    uint32_t pullMask  = (0x3 << pullShift);
    uint32_t pullValue = (flags << pullShift);

    //printf("pullReg = 0x%08x\n", *pullReg);
    *pullReg &= ~pullMask;
    *pullReg |= pullValue;
    //printf("pullReg = 0x%08x\n", *pullReg);
}

GpioPinOPi::GpioPinOPi(uint8_t _gpioNumber, bool isInput)
{
    assert(gpio != nullptr);
    assert(_gpioNumber < 32);

    gpioNumber = _gpioNumber;

    setupCfg(gpioNumber, isInput ? CFG_INPUT : CFG_OUTPUT);
    setupPull(gpioNumber, isInput ? PULL_UP : PULL_DISABLE);

    datReg = (uint32_t*)(gpio + PORT_DAT_BASE);
    datMask = 1 << gpioNumber;

    //printf("gpio=%2u, datReg=0x%08x, datMask=0x%08x\n", gpioNumber, datReg, datMask);
}

void GpioPinOPi::set(bool f) const
{
    Mutex::Lock lock(gpioMutex);

    //printf("gpio=%2u, set %u, data1 = 0x%08x\n", gpioNumber, f, *datReg);
    if (f) {
        *datReg |= datMask;
    }
    else {
        *datReg &= ~datMask;
    }
    //printf("gpio=%2u, set %u, data2 = 0x%08x\n", gpioNumber, f, *datReg);
}

uint8_t GpioPinOPi::read() const
{
    return ((*datReg) & datMask ? 1 : 0);
}
