#ifndef GPIOPOLLING_H
#define GPIOPOLLING_H

#include "app/AppComponent.h"
#include "base/PollTimer.h"
#include "config/Config.h"
#include "env/GpioKey.h"
#include "env/GpioLed.h"
#include "sys/ThreadRunner.h"


class GpioPolling
    : protected AppComponent
{
friend class ApplicationImpl;

// Construction/destruction.
private:
    GpioPolling(AppRegistry& appRegistry);
    ~GpioPolling() throw();

// AppComponent implementation.
protected:
    virtual void doInit(const Config& config);
    virtual void doStart();
    virtual void doStop() throw();
    virtual void doDone() throw();

// Implementation methods.
private:
    void pollingThread();
    void setLeds();

    // Member variables.
private:
    // The member object responsible for running controlling thread.
    ThreadRunner<GpioPolling, &GpioPolling::pollingThread> m_pollingThread;

    GpioLed ledG;
    GpioLed ledR;

    GpioKey key0;
    GpioKey key1;

    PollTimer kbdTimer;
};

#endif // GPIOPOLLING_H
