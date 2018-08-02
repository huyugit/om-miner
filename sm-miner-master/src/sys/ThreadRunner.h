#ifndef THREAD_RUNNER_H
#define THREAD_RUNNER_H
/*
 * Contains ThreadRunner class declaration.
 */

#include "base/NonCopyable.h"
#include "except/Exception.h"
#include "except/SystemException.h"
#include "sys/Mutex.h"
#include "sys/ConditionVariable.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <error.h>
#include <pthread.h>
#include <cxxabi.h>


// A template class to run a specified method of class T in a different thread.
template<class T, void (T::*exec)()>
class ThreadRunner
    : private NonCopyable  // Prevent copy and assignment.
{
// Construction.
public:
    // Constructs the object.
    ThreadRunner()
        : m_invalidId(::pthread_self())
        , m_threadId(m_invalidId)
        , m_cancelRequsted(false)
        , m_startedCondition()
        , m_startWaitMutex()
        , m_worker(nullptr)
    {
    }
    
    // Joins the spawned thread on destruction.
    ~ThreadRunner() throw()
    {
        join();
    }

// Public interface.
public:
    // Spawns a new thread and from that thread calls T::*exec() on the given work object.
    void start(T* worker, size_t stackSize = 1024 * 1024)  // 1Mb
    {
        assert(worker != nullptr);
        
        // Join to current running thread if any.
        join();
        
        // Reset thread cancellation flag.
        m_cancelRequsted = false;
        
        // Set work object to call the execution method.
        m_worker = worker;
        
        pthread_attr_t attr;
        if (int rc = ::pthread_attr_init(&attr))
            throw SystemException(rc, "pthread_attr_init() failed.");
        
        try
        {
            if (int rc = ::pthread_attr_setstacksize(&attr, stackSize))
                throw SystemException(rc, "pthread_attr_setstacksize() failed.");
            
            Mutex::Lock lock(m_startWaitMutex);
            
            pthread_t threadId;
            if (int rc = ::pthread_create(&threadId, &attr, &threadRoutine, this))
                throw SystemException(rc, "pthread_create() failed.");
            
            // Wait until the spawned thread is started.
            while (!isRunning())
                m_startedCondition.wait(m_startWaitMutex);
        }
        catch (...)
        {
            if (int rc = ::pthread_attr_destroy(&attr))
                ::error(0, rc, "ERROR: pthread_attr_destroy() failed.");
            
            throw;
        }
    }
    
    // Returns if there is a spawned thread running.
    inline bool isRunning() const throw()
    {
        return (!::pthread_equal(m_threadId, m_invalidId));
    }
    
    inline unsigned long int getThreadId() const throw()
    {
        return static_cast<unsigned long int>(m_threadId);
    }
    
    // Cancel the current running thread (if any).
    // If 'force' argument is true, also calls pthread_cancel().
    void cancel(bool force = false) throw()
    {
        if (!isRunning())
            return;  // No thread running.
        
        m_cancelRequsted = true;
        if (!force)
            return;
        
        if (int rc = ::pthread_cancel(m_threadId))
        {
            if (isRunning())
                ::error(0, rc, "ERROR: pthread_cancel() failed (thread ID = 0x%08lx from 0x%08lx)",
                    getThreadId(), static_cast<unsigned long int>(::pthread_self()));
                
        }
    }

    // Return true if the cancellation has been requested by calling cancel().
    bool isCancelRequested() const throw()
    {
        return m_cancelRequsted;
    }

    // Waits until the spawned thread finishes.
    void join() throw()
    {
        if (!isRunning())
            return;  // No thread running.

        if (::pthread_equal(m_threadId, ::pthread_self()))
            return;  // No sense to join from the current thread.

        if (int rc = ::pthread_join(m_threadId, nullptr))
        {
            if (isRunning())
            {
                ::error(0, rc, "ERROR: pthread_join() failed (thread ID = 0x%08lx from 0x%08lx)",
                    getThreadId(), static_cast<unsigned long int>(::pthread_self()));
                m_threadId = m_invalidId;
            }
        }
    }

// Implementation methods.
private:
    // A start routine for the new thread.
    // Invokes T::exec(), specified as a template argument.
    static void* threadRoutine(void* arg)
    {
        assert(arg != nullptr);
        
        ThreadRunner* obj = static_cast<ThreadRunner*>(arg);
        
        // Store the ID of the spawned thread.
        const pthread_t threadId = ::pthread_self();
        obj->m_threadId = threadId;
        
        // Signal the parent thread that this thread has been started.
        Mutex::Lock lock(obj->m_startWaitMutex);
        obj->m_startedCondition.signal();
        lock.unlock();
        
        try
        {
            (obj->m_worker->*exec)();
        }
        catch (const abi::__forced_unwind& )
        {
            // Mark thread as non-running by resetting the thread ID.
            obj->m_threadId = obj->m_invalidId;
            
            // Thread has been canceled, re-throwing to avoid "exception not rethrown" error.
            throw;
        }
        catch (const Exception& e)
        {
            ::fprintf(stderr, "FATAL: Unhandled exception in thread 0x%08lx:\n%s\n",
                static_cast<unsigned long int>(threadId), e.what());
            assert(false);  // Should not normally get here.
            throw;  // This will terminate the application.
        }
        catch (...)
        {
            ::fprintf(stderr, "FATAL: Unhandled exception in thread 0x%08lx.\n",
                static_cast<unsigned long int>(threadId));
            assert(false);  // Should not normally get here.
            throw;  // This will terminate the application.
        }
        
        // Mark thread as non-running by resetting the thread ID.
        obj->m_threadId = obj->m_invalidId;
        
        return nullptr;
    }

// Member variables.
private:
    // Invalid thread ID - initialized to the thread creating the object.
    const pthread_t m_invalidId;
    
    // The ID of the spawned thread (set to m_invalidId if not running).
    volatile pthread_t m_threadId;
    
    // A flag requesting cancellation of the current running thread.
    volatile bool m_cancelRequsted;
    
    // Condition variable and mutex to wait until the spawned thread starts.
    ConditionVariable m_startedCondition;
    Mutex m_startWaitMutex;
    
    // A work object to run the execution method on.
    T* m_worker;
};

#endif  // THREAD_RUNNER_H
