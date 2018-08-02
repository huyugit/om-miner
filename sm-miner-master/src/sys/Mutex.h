#ifndef MUTEX_H
#define MUTEX_H
/*
 * Contains Mutex class declaration.
 */

#include "base/NonCopyable.h"
#include "except/SystemException.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <error.h>
#include <pthread.h>


// This is a wrapping class for pthread recursive mutex.
// Destroys the wrapped mutex on destruction.
//
class Mutex
    : private NonCopyable  // Prevent copy and assignment.
{
// Construction/destruction.
public:
    // Constructs the Mutex object .
    Mutex()
    {
        pthread_mutexattr_t attr;
        if (int rc = ::pthread_mutexattr_init(&attr))
            throw SystemException(rc, "pthread_mutexattr_init() failed");
        
        if (int rc = ::pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE))
            throw SystemException(rc, "pthread_mutexattr_settype() failed");
        
        if (int rc = ::pthread_mutex_init(&m_handle, &attr))
            throw SystemException(rc, "pthread_mutex_init() failed");
        
        if (int rc = ::pthread_mutexattr_destroy(&attr))
            ::error(0, rc, "ERROR: pthread_mutexattr_destroy() failed");
    }

    // Destroys the Mutex object.
    ~Mutex() throw()
    {
        if (int rc = ::pthread_mutex_destroy(&m_handle))
            ::error(0, rc, "ERROR: pthread_mutex_destroy() failed");
    }

// Public interface.
public:
    // Locks the mutex. Aborts the program on failure.
    void lock()
    {
        if (int rc = ::pthread_mutex_lock(&m_handle))
            throw SystemException(rc, "pthread_mutex_lock() failed in thread 0x%08lx",
                static_cast<unsigned long int>(::pthread_self()));
    }

    // Tries locking the mutex. Returns "true" if the lock has been acquired and
    // "false" if the lock is already locked by the current or other thread.
    bool tryLock()
    {
        if (int rc = ::pthread_mutex_trylock(&m_handle))
        {
            if (rc == EBUSY)
                return false;
            
            throw SystemException(rc, "pthread_mutex_trylock() failed in thread 0x%08lx",
                static_cast<unsigned long int>(::pthread_self()));
        }
        
        return true;
    }

    // Unlocks the mutex.
    void unlock() throw()
    {
        if (int rc = ::pthread_mutex_unlock(&m_handle))
            ::error(0, rc, "ERROR: pthread_mutex_unlock() failed");
    }
    
    // Returns Pthread mutex object.
    pthread_mutex_t* getHandle() throw()
    {
        return &m_handle;
    }

// Lock/Unlock facility.
public:
    // This is RAII class to auto lock/unlock a Mutex object.
    // Locks the given Mutex in constructor and unlocks in destructor.
    class Lock
        : private NonCopyable  // Prevent copy and assignment.
    {
    // Construction/destruction.
    public:
        // Constructs the Mutex::Lock object .
        explicit Lock(Mutex& mutex, bool locked = false)
            : m_mutex(mutex)
            , m_locked(locked)
        {
            lock();
        }

        // Destroys the Mutex::Lock object.
        ~Lock() throw()
        {
            unlock();
        }

    // Public interface.
    public:
        // Locks the Mutex::Lock object.
        void lock()
        {
            if (!m_locked)
            {
                m_mutex.lock();
                m_locked = true;
            }
        }
        
        // Unlocks the Mutex::Lock object.
        void unlock() throw()
        {
            if (m_locked)
            {
                m_mutex.unlock();
                m_locked = false;
            }
        }
        
    // Member variables.
    private:
        // Attached mutex object to lock/unlock.
        Mutex& m_mutex;
        bool m_locked;
    };

// Member variables.
private:
    // Pthread mutex handle.
    pthread_mutex_t m_handle;
};

#endif  // MUTEX_H
