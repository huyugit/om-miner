/*
 * Contains LockedConfig class definition.
 */

#include "LockedConfig.h"

#include "config/Config.h"

#include "sys/Mutex.h"

#include <assert.h>


LockedConfig::LockedConfig(Config& config, Mutex& configMutex)
    : m_configPtr(&config)
    , m_configMutexPtr(&configMutex)
    , m_locked(false)
{
    lock();
}

LockedConfig::~LockedConfig() throw()
{
    unlock();
}

LockedConfig::LockedConfig(const LockedConfig& src)
    : m_configPtr(src.m_configPtr)
    , m_configMutexPtr(src.m_configMutexPtr)
    , m_locked(false)
{
    lock();
}

LockedConfig& LockedConfig::operator=(const LockedConfig& right)
{
    LockedConfig(right).swap(*this);
    return *this;
}

const Config& LockedConfig::operator*() const throw()
{
    return *m_configPtr;
}

const Config* LockedConfig::operator->() const throw()
{
    return m_configPtr;
}

const Config* LockedConfig::getConfig() const throw()
{
    return m_configPtr;
}

Config *LockedConfig::getConfigRW() throw()
{
    return m_configPtr;
}

void LockedConfig::lock()
{
    assert(!m_locked);
    m_configMutexPtr->lock();
    m_locked = true;
}

void LockedConfig::unlock() throw()
{
    if (m_locked)
    {
        m_configMutexPtr->unlock();
        m_locked = false;
    }
}
