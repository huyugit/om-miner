#ifndef POLL_TIMER_H
#define POLL_TIMER_H
/*
 * Contains PollTimer class declaration.
 */

#include "base/BaseUtil.h"
#include "except/SystemException.h"

#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <error.h>
#include <time.h>


// Class to measure wall clock elapsed time.
class PollTimer
{
// Construction/destruction.
public:
    // Default constructor.
    // If createStarted flag indicated, the timer is started
    // immediately at construction.
    explicit PollTimer(bool createStarted = true) throw()
        : m_startTime()
        , m_stopped(true)
        , m_elapsedExtraMs(0)
    {
        if (createStarted)
            start();
    }

    // Copy constructor.
    PollTimer(const PollTimer& src) throw()
        : m_startTime(src.m_startTime)
        , m_stopped(src.m_stopped)
        , m_elapsedExtraMs(src.m_elapsedExtraMs)
    {}

// Operators.
public:
    // Assignment operator.
    PollTimer& operator=(const PollTimer& right) throw()
    {
        if (&right != this)
        {
            m_startTime = right.m_startTime;
            m_stopped = right.m_stopped;
            m_elapsedExtraMs = right.m_elapsedExtraMs;
        }
        
        return *this;
    }

// Public interface.
public:
    // Begins accumulating elapsed time.
    void start()
    {
        if (::clock_gettime(CLOCK_MONOTONIC_RAW, &m_startTime) < 0)
            throw SystemException(errno, "clock_gettime() failed");

        m_stopped = false;
        m_elapsedExtraMs = 0;
    }
    
    // Stops accumulating elapsed time.
    void stop() throw()
    {
        if (!m_stopped)
        {
            m_elapsedExtraMs = elapsedMs();
            m_stopped = true;
        }
    }
    
    // Resumes the stopped timer.
    void resume()
    {
        if (m_stopped)
        {
            if (::clock_gettime(CLOCK_MONOTONIC_RAW, &m_startTime) < 0)
                throw SystemException(errno, "clock_gettime() failed");
            
            m_stopped = false;
        }
    }
    
    // Stops timer and clears accumulated elapsed time.
    void reset() throw()
    {
        m_stopped = true;
        m_elapsedExtraMs = 0;
    }
    
    // Returns whether the timer is currently stopped.
    inline bool isStopped() const throw()
    {
        return m_stopped;
    }
    
    // Returns a number of milliseconds since the time the timer was
    // started not accounting the total time when it was stopped.
    uint64_t elapsedMs() const
    {
        if (m_stopped)
            return m_elapsedExtraMs;
        
        struct timespec currentTime;
        if (::clock_gettime(CLOCK_MONOTONIC_RAW, &currentTime) < 0)
            throw SystemException(errno, "clock_gettime() failed");
        
        const uint64_t diffMs = m_elapsedExtraMs
            + int64_t(currentTime.tv_sec - m_startTime.tv_sec) * 1000
            + int64_t(currentTime.tv_nsec - m_startTime.tv_nsec) / 1000000;
        
        return diffMs;
    }
    
    // Returns elapsed time in seconds (see elapsedMs).
    // For checking if the interval is elapsed use isElapsedSec().
    unsigned int elapsedSec() const
    {
        return static_cast<unsigned int>(elapsedMs() / 1000);
    }
    
    // Returns true if the given interval in milliseconds is elapsed.
    inline bool isElapsedMs(uint64_t intervalMs) const throw()
    {
        return (elapsedMs() >= intervalMs);
    }

    // Returns true if the given interval in seconds is elapsed.
    inline bool isElapsedSec(uint64_t intervalSec) const throw()
    {
        return (elapsedMs() >= intervalSec * 1000);
    }

    // Exchanges the content of this object with "other".
    void swap(PollTimer& other) throw()
    {
        util::swap(m_startTime, other.m_startTime);
        util::swap(m_stopped, other.m_stopped);
        util::swap(m_elapsedExtraMs, other.m_elapsedExtraMs);
    }

// Member variables.
private:
    // Timer starting/resuming timestamp (seconds, microseconds).
    struct timespec m_startTime;

    // Whether the time is stopped.
    bool m_stopped;
    
    // A number of milliseconds elapsed as of the previous stop.
    uint64_t m_elapsedExtraMs;
};

#endif  // POLL_TIMER_H
