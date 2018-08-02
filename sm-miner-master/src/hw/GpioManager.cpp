#include "GpioManager.h"

#include "app/Application.h"
#include "config/Config.h"
#include "env/EnvManager.h"
#include "except/ApplicationException.h"
#include "hw/CpuInfo.h"
#include "hw/GpioPinPi.h"
#include "hw/GpioPinOPi.h"

#include <unistd.h>


GpioManager g_gpioManager;


GpioManager::GpioManager()
    : spiSwMode(false), mbHwVer(0)
{}

void GpioManager::init()
{
    switch (Application::configRW().slaveSpiDrv)
    {
    case SPI_DRV_SW:
        spiSwMode = true;
        break;
    case SPI_DRV_AUTO:
        break;
    }

    if (spiSwMode) {
        printf("WARNING: GpioManager: running SPI in SW mode\n");
    }


    switch (g_cpuInfo.cpuType)
    {
    case CpuInfo::CPU_RPI: GpioPinPi::initMapping(); break;
    case CpuInfo::CPU_OPI: GpioPinOPi::initMapping(); break;
    default:
        throw ApplicationException("GpioManager: unexpected cpu type");
    }

    // create pin managers based on pin header number (not gpio),
    // int this case we can map pins to gpio correctly for different
    // devices (RaspberryPi, OrangePi, etc.)

    pinLedR         = createPin(16);
    pinLedG         = createPin(18);

    pinKey0         = createPin(22, true);
    pinKey1         = createPin(13, true);

    if (spiSwMode) {
        pinSpiMosi      = createPin(19);
        pinSpiMiso      = createPin(21, true);
        pinSpiClk       = createPin(23);
    }
    else {
        // we can not create pins in hw mode, as we will switch
        // pins from alternative function, and we will lost possibility
        // to use hw drivers, so we are creating dummy pins

        pinSpiMosi      = new GpioPinDummy();
        pinSpiMiso      = new GpioPinDummy();
        pinSpiClk       = new GpioPinDummy();

        // for OrangePi Zero pull-up (or down) input MISO pin to avoid
        // random data, when slave is not ready for communication

        if (g_cpuInfo.cpuType == CpuInfo::CPU_OPI)
        {
            GpioPinOPi::setupPull(GpioPinOPi::pinToGpio(21), GpioPinOPi::PULL_UP);
        }
    }

    pinMux0    = createPin(12);
    pinMux1    = createPin(15);

    pinHwVer   = createPin(26, true);

    pinNRst    = createPin(7);
    pinBoot    = createPin(11);
    pinNss     = createPin(24);


    readHwVer();

    if (Application::configRW().noMcuReset)
    {
        printf("GpioManager: no MCU reset mode\n");
    }
    else {
        uint32_t slaveMask = Application::configRW().slaveMask;
        printf("GpioManager: reset slaves, mask 0x%02x\n", slaveMask);

        slaveResetByMask(slaveMask);
    }
}

GpioPin *GpioManager::createPin(uint8_t pinNumber, bool isInput)
{
    switch (g_cpuInfo.cpuType)
    {
    case CpuInfo::CPU_RPI: return new GpioPinPi(GpioPinPi::pinToGpio(pinNumber), isInput);
    case CpuInfo::CPU_OPI: return new GpioPinOPi(GpioPinOPi::pinToGpio(pinNumber), isInput);

    default:
        throw ApplicationException("GpioManager: unexpected cpu type");
    }
}

void GpioManager::setMuxAddr(uint32_t addr)
{
    pinMux0->set((addr >> 0) & 1);
    pinMux1->set((addr >> 1) & 1);
}

void GpioManager::readHwVer()
{
    for (int i = 0; i < 4; i++)
    {
        setMuxAddr(i);
        mbHwVer |= (pinHwVer->read() << i);
    }

    printf("GpioManager: mbHwVer 0x%x\n", mbHwVer);

    if (Application::configRW().mbHwVer != 0xff)
    {
        mbHwVer = Application::configRW().mbHwVer;
        printf("GpioManager: replaced mbHwVer 0x%x\n", mbHwVer);
    }

    switch (mbHwVer) {
    case 0xf: EnvManager::slaveCount = 1; break;
    case 0x1: EnvManager::slaveCount = 4; break;
    default:  EnvManager::slaveCount = SLAVE_COUNT_DFT; break;
    }

    printf("GpioManager: slave MCU count %d\n", EnvManager::slaveCount);
}

void GpioManager::slaveResetByMask(uint32_t mask)
{
    //printf("GpioManager::slaveResetByMask: mask = 0x%02x\n", mask);
    for (int i = 0; i < EnvManager::slaveCount; i++)
    {
        if (mask & (1 << i))
        {
            slaveReset(i);
        }
    }
}

void GpioManager::slaveReset(uint32_t sid)
{
    printf("GpioManager::slaveReset: sid = %u\n", sid);
    setMuxAddr(sid);

    pinNRst->set(0);
    pinBoot->set(0);

    usleep(200 * 1000);

    pinNRst->set(1);
}

void GpioManager::slaveSelect(uint32_t sid, uint8_t value)
{
    //printf("GpioManager::slaveSelect: sid = %u, value = %u\n", sid, value);
    setMuxAddr(sid);

    pinNss->set(!value);
}
