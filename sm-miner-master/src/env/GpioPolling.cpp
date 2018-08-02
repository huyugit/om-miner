#include "GpioPolling.h"

#include "app/Application.h"

#include "env/EnvManager.h"
#include "hw/GpioManager.h"
#include "pool/StratumPool.h"

#include "log/LogManager.h"
#include "log/LogCategories.h"

#include "base/MiscUtil.h"

#include "version.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>


namespace
{
    // Create manager logger.
    Logger& logger = LogManager::instance().createLogger(LogCategories::c_gpioLogCtg);

}


GpioPolling::GpioPolling(AppRegistry& appRegistry)
    : AppComponent(appRegistry)
    , m_pollingThread()
{}

GpioPolling::~GpioPolling() throw()
{
    done();  // Ensure the object is uninitialized properly.
}

void GpioPolling::doInit(const Config& /*config*/)
{
    return;
    LOG_TRACE(logger) << "Initializing GpioPolling manager...\n";

    ledG.init(g_gpioManager.pinLedG);
    ledR.init(g_gpioManager.pinLedR);

    key0.init(g_gpioManager.pinKey0);
    key1.init(g_gpioManager.pinKey1);
}

void GpioPolling::doStart()
{
    return;
    LOG_TRACE(logger) << "Starting GpioPolling controlling thread...\n";

    try
    {
        m_pollingThread.start(this);
    }
    catch (const Exception& e)
    {
        throw ApplicationException("Unable to start GpioPolling controlling thread: %s", e.what());
    }
}

void GpioPolling::doStop() throw()
{
    return;
    LOG_TRACE(logger) << "Stopping GpioPolling controlling thread...\n";

    m_pollingThread.cancel();
    m_pollingThread.join();
}

void GpioPolling::doDone() throw()
{
    return;
    LOG_TRACE(logger) << "Uninitializing GpioPolling manager...\n";
}

void GpioPolling::pollingThread()
{
    LOG_TRACE(logger) << "Entered GpioPolling controlling thread 0x"
        << Logger::format("%08lx", m_pollingThread.getThreadId()) << ".\n";

    PollTimer updateTimer;
    PollTimer ledTimer;

    setLeds();

    while (!m_pollingThread.isCancelRequested())
    {
        if (kbdTimer.isStopped())
        {
            if (updateTimer.isElapsedMs(1000))
            {
                setLeds();
                updateTimer.start();
            }
        }
        else {
            if (kbdTimer.elapsedMs() > 1000)
            {
                kbdTimer.reset();
            }
        }

        if (ledTimer.isElapsedMs(100))
        {
            GpioLed::timerValue++;

            ledG.onTimerTick();
            ledR.onTimerTick();

            ledTimer = PollTimer();
        }

        key0.poll();
        key1.poll();

        if (key0.shortPress) {
            printf("GpioPolling: key0.shortPress\n");
        }
        if (key0.longPress) {
            printf("GpioPolling: key0.longPress\n");
        }
        if (key1.shortPress) {
            printf("GpioPolling: key1.shortPress\n");
        }
        if (key1.longPress) {
            printf("GpioPolling: key1.longPress\n");
        }

        if (key0.shortPress || key0.longPress ||
            key1.shortPress || key1.longPress)
        {
            kbdTimer.start();
            setLeds();
        }

        ::usleep(10 * 1000);
    }

    LOG_TRACE(logger) << "Leaving GpioPolling controlling thread.\n";
}

void GpioPolling::setLeds()
{
    if (kbdTimer.isStopped())
    {
        ledG.setState(GpioLed::LED_BLINK_SLOW);

        if (Application::pool().getState() == StratumPool::ProtocolState::IN_SERVICE)
        {
            ledR.setState(GpioLed::LED_OFF);
        }
        else {
            ledR.setState(GpioLed::LED_BLINK_SLOW);
        }
    }
    else {
        ledG.setState(GpioLed::LED_BLINK_QUICK);
        ledR.setState(GpioLed::LED_BLINK_QUICK);
    }
}
