/*
 * Contains Application class definition.
 */

#include "Application.h"

#include "app/ApplicationImpl.h"

#include "except/OutOfMemoryException.h"

#include <new>


Application::Application()
    : m_implPtr(new(std::nothrow) ApplicationImpl())
{
    if (m_implPtr == nullptr)
        throw OutOfMemoryException();
}

Application::~Application() throw()
{
    delete m_implPtr;
}

const Config& Application::defaults() throw()
{
    return ApplicationImpl::defaults();
}

const LockedConfig Application::config()
{
    return instance().m_implPtr->config();
}

Config &Application::configRW()
{
    return instance().m_implPtr->configRW();
}

void Application::parseCommandLine(int argc, const char* argv[])
{
    instance().m_implPtr->parseCommandLine(argc, argv);
}

void Application::configPostProcessing()
{
    instance().m_implPtr->configPostProcessing();
}

void Application::init()
{
    instance().m_implPtr->init();
}

void Application::run()
{
    instance().m_implPtr->run();
}

void Application::requestExit(int rc)
{
    instance().m_implPtr->requestExit(rc);
}

void Application::done() throw()
{
    instance().m_implPtr->done();
}

EventManager& Application::events() throw()
{
    return instance().m_implPtr->events();
}

GpioPolling& Application::gpioPolling() throw()
{
    return instance().m_implPtr->gpioPolling();
}

StratumPool& Application::pool() throw()
{
    return instance().m_implPtr->pool();
}

Application& Application::instance() throw()
{
    static Application s_instance;
    return s_instance;
}
