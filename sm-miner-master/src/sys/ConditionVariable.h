#ifndef CONDITION_VARIABLE_H
#define CONDITION_VARIABLE_H
/*
 * Contains ConditionVariable class declaration.
 */

#include "base/NonCopyable.h"
#include "except/SystemException.h"
#include "sys/Mutex.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <error.h>
#include <assert.h>
#include <pthread.h>
#include <time.h>


// This is a wrapping class for a system file descriptor.
// Closes the owned descriptor on destruction.
class ConditionVariable
    : private NonCopyable  // Prevent copy and assignment.
{
// Construction/destruction.
public:
    // Constructs the object .
    ConditionVariable()
    {
        // Use default condition variable attributes
        if (int rc = ::pthread_cond_init(&m_cond, NULL))
            throw SystemException(rc, "pthread_cond_init() failed in thread 0x%08lx",
                static_cast<unsigned long int>(::pthread_self()));
    }

    // Destroys the object.
    ~ConditionVariable() throw()
    {
        if (int rc = ::pthread_cond_destroy(&m_cond))
            ::error(0, rc, "pthread_cond_destroy() failed in thread 0x%08lx",
                static_cast<unsigned long int>(::pthread_self()));
    }

// Public interface.
public:
    // Waits for a Condition.
    void wait(Mutex& mutex)
    {
        if (int rc = ::pthread_cond_wait(&m_cond, mutex.getHandle()))
            throw SystemException(rc, "pthread_cond_wait() failed in thread 0x%08lx",
                static_cast<unsigned long int>(::pthread_self()));
    }
    
    // Timed Wait for a Condition.
    // Returns true if the wait was successful.
    // If the wait timed out without being satisfied, returns false.
    // "false" if the lock is already locked by the current or other thread.
    bool timedWait(Mutex& mutex, int timeoutMs)
    {
        assert(timeoutMs >= 0);
        
        // Get the current hardware-based time
        // (seconds and nanoseconds since the Epoch).
        struct timespec ts;
        if (::clock_gettime(CLOCK_MONOTONIC_RAW, &ts) < 0)
            throw SystemException(errno, "clock_gettime() failed");
        
        // Decompose the given timeoutMs into seconds and milliseconds.
        const int deltaSec = timeoutMs / 1000;
        const int deltaMs = timeoutMs & 1000;
        
        // Adjust ts time with the seconds delta.
        ts.tv_sec += deltaSec;
        
        // Adjust ts time with milli-seconds delta.
        ts.tv_nsec += static_cast<long>(deltaMs) * 1000000;
        if (ts.tv_nsec >= 1000000000)
        {
            ++ts.tv_sec;
            ts.tv_nsec -= 1000000000;
        }
        
        // Wait for the condition.
        if (int rc = ::pthread_cond_timedwait(&m_cond, mutex.getHandle(), &ts))
        {
            if (rc == ETIMEDOUT)
                return false;  // Wait timed out.
            
            throw SystemException(rc, "pthread_cond_timedwait() failed in thread 0x%08lx",
                static_cast<unsigned long int>(::pthread_self()));
        }
        
        return true;  // Wait succeeded.
    }

    // Signal Condition to One Waiter.
    void signal()
    {
        if (int rc = ::pthread_cond_signal(&m_cond))
            throw SystemException(rc, "pthread_cond_signal() failed in thread 0x%08lx",
                static_cast<unsigned long int>(::pthread_self()));
    }

    // Broadcast Condition to All Waiters.
    void broadcast()
    {
        if (int rc = ::pthread_cond_broadcast(&m_cond))
            throw SystemException(rc, "pthread_cond_broadcast() failed in thread 0x%08lx",
                static_cast<unsigned long int>(::pthread_self()));
    }

// Member variables.
private:
    // Pthread condition variable.
    pthread_cond_t m_cond;
};

#endif  // CONDITION_VARIABLE_H
