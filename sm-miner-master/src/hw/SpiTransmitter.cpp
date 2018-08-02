/*
 * Contains SpiTransmitter class definition.
 */

#include "SpiTransmitter.h"

#include "base/MinMax.h"

#include "except/SystemException.h"

#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h> 

#include "app/Application.h"
#include "base/PollTimer.h"
#include "config/Config.h"
#include "except/ApplicationException.h"
#include "hw/CpuInfo.h"
#include "hw/GpioManager.h"
#include "hw/GpioPinOPi.h"


SpiTransmitter::SpiTransmitter()
    : m_dev()
    , m_mode(SPI_MODE_0)
    , m_bits(8)
    , m_speed(2000000)
{
    m_speed = Application::configRW().slaveSpiSpeed;
}

void SpiTransmitter::transfer(const uint8_t* wrbuf, uint8_t* rdbuf, size_t size)
{
    if (g_gpioManager.spiSwMode) {
        transferSw(wrbuf, rdbuf, size);
    }
    else {
        transferHw(wrbuf, rdbuf, size);
    }
}

void SpiTransmitter::open(const char* spiDev)
{
    assert(spiDev != nullptr);

    FileDescriptor dev(::open(spiDev, O_RDWR));
    if (dev.isInvalid())
        throw SystemException(errno, "Unable to open SPI %s", spiDev);

    if (::ioctl(dev.getDescriptor(), SPI_IOC_WR_MODE, &m_mode) < 0)
        throw SystemException(errno, "Unable to set SPI_IOC_WR_MODE to %d", m_mode);

    if (::ioctl(dev.getDescriptor(), SPI_IOC_RD_MODE, &m_mode) < 0)
        throw SystemException(errno, "Unable to set SPI_IOC_RD_MODE to %d", m_mode);

    if (::ioctl(dev.getDescriptor(), SPI_IOC_WR_BITS_PER_WORD, &m_bits) < 0)
        throw SystemException(errno, "Unable to set SPI_IOC_WR_BITS_PER_WORD to %d", m_bits);

    if (::ioctl(dev.getDescriptor(), SPI_IOC_RD_BITS_PER_WORD, &m_bits) < 0)
        throw SystemException(errno, "Unable to set SPI_IOC_RD_BITS_PER_WORD to %d", m_bits);

    if (::ioctl(dev.getDescriptor(), SPI_IOC_WR_MAX_SPEED_HZ, &m_speed) < 0)
        throw SystemException(errno, "Unable to set SPI_IOC_WR_MAX_SPEED_HZ to %d", m_speed);

    if (::ioctl(dev.getDescriptor(), SPI_IOC_RD_MAX_SPEED_HZ, &m_speed) < 0)
        throw SystemException(errno, "Unable to set SPI_IOC_RD_MAX_SPEED_HZ to %d", m_speed);

    m_dev.swap(dev);
}

void SpiTransmitter::transferHw(const uint8_t* wrbuf, uint8_t* rdbuf, size_t size)
{
    if (!isOpen())
    {
        const char* fileName = "/dev/spidev0.0";

        switch (g_cpuInfo.cpuType)
        {
        case CpuInfo::CPU_RPI: fileName = "/dev/spidev0.0"; break;
        case CpuInfo::CPU_OPI: fileName = "/dev/spidev1.0"; break;
        default:
            throw ApplicationException("GpioManager: unexpected cpu type");
        }

        //printf("SpiTransmitter: open %s\n", fileName);
        open(fileName);
    }

    static const size_t SPI_CHUNK_SIZE = 4096;
    static const int MAX_TR_COUNT = 16;

    struct spi_ioc_transfer tr[MAX_TR_COUNT] = {};

    // Prepare SPI transfer commands.
    int cc = 0;  // A number of SPI transfer commands.
    while (size > 0)
    {
        const size_t len = min(size, SPI_CHUNK_SIZE);
    
        tr[cc].tx_buf = reinterpret_cast<uintptr_t>(wrbuf);
        tr[cc].rx_buf = reinterpret_cast<uintptr_t>(rdbuf);
        tr[cc].len = len;
        tr[cc].delay_usecs = 0;
        tr[cc].speed_hz = m_speed;
        tr[cc].bits_per_word = m_bits;
        tr[cc].cs_change = 0;

        size -= len;
        wrbuf += len;
        rdbuf += len;
        ++cc;
        
        if (cc > MAX_TR_COUNT)
            throw SystemException("SPI transmit failed: Size too big (not enough SPI transfers)");
    }

    // Execute SPI transfers one-by-one.
    for (int i = 0; i < cc; ++i)
    {
        if (::ioctl(m_dev.getDescriptor(), SPI_IOC_MESSAGE(1), reinterpret_cast<intptr_t>(&tr[i])) < 0)
            throw SystemException(errno, "SPI transmit failed");
    }
}

void SpiTransmitter::delay()
{
    static volatile int tmp = 0;
    for (int i = 0; i < 2; i++) tmp += i;
}

void SpiTransmitter::transferSw(const uint8_t *wrbuf, uint8_t *rdbuf, size_t size)
{
    GpioPin &pinMosi = *g_gpioManager.pinSpiMosi;
    GpioPin &pinMiso = *g_gpioManager.pinSpiMiso;
    GpioPin &pinClk  = *g_gpioManager.pinSpiClk;

    PollTimer timer;
    uint32_t bits = 0;

    for (size_t i = 0; i < size; i++)
    {
        rdbuf[i] = 0;
        for (int j = 7; j >= 0; j--)
        {
            uint8_t bo = wrbuf[i] & (1 << j);
            pinMosi.set(bo);

            delay();

            pinClk.set(1);

            delay();

            rdbuf[i] |= (pinMiso.read() << j);
            pinClk.set(0);

            bits++;
        }
    }

    timer.stop();

    uint32_t ms = timer.elapsedMs();
    if (ms == 0) ms = 1;

    //printf("TX: %u bits in %u ms, %u KHz\n", bits, ms, bits / ms);
}
