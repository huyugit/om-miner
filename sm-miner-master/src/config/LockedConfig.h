#ifndef LOCKED_CONFIG_H
#define LOCKED_CONFIG_H
/*
 * Contains LockedConfig class declaration.
 */

#include "base/BaseUtil.h"


// Forward declarations.
class Config;
class Mutex;

// Class locking the application config on construction and
// unlocking on destruction. Defines dereference and member
// access operators to be used as a config proxy.
// 
class LockedConfig
{
friend class ApplicationImpl;

// Construction.
private:
    // Constructor (stores and locks the given config).
    LockedConfig(Config& config, Mutex& configMutex);

// Destruction.
public:
    // Destructor (unlocks the stored config).
    ~LockedConfig() throw();

// Copy and assignment.
public:
    LockedConfig(const LockedConfig& src);
    LockedConfig& operator=(const LockedConfig& right);

// Operators.
public:
    // Dereference operator.
    const Config& operator*() const throw();
    
    // Member access operator.
    const Config* operator->() const throw();

    const Config* getConfig() const throw();
    Config* getConfigRW() throw();

// Public interface.
public:
    // Explicit config lock/unlock.
    void lock();
    void unlock() throw();
    
    // Exchanges the content of this object with "other".
    void swap(LockedConfig& other) throw()
    {
        util::swap(m_configPtr, other.m_configPtr);
        util::swap(m_configMutexPtr, other.m_configMutexPtr);
        util::swap(m_locked, other.m_locked);
    }

// Member variables.
private:
    // Configuration object reference.
    Config* m_configPtr;
    
    // Reference to a mutex for locking/unlocking configuration.
    Mutex* m_configMutexPtr;
    
    // Whether the config is locked.
    bool m_locked;
};

#endif  // LOCKED_CONFIG_H
