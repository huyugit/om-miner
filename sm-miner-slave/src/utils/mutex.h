#ifndef MUTEX_H
#define MUTEX_H

#include "atomic_block.hpp"

class Mutex
{
public:
    Mutex()
        : busy(false)
    {}

    void lock()
    {
        AtomicBlock b;
        while (busy) {}
        busy = true;
    }

    bool trylock()
    {
        AtomicBlock b;
        if (busy) {
            return false;
        }
        else {
            busy = true;
            return true;
        }
    }

    void unlock()
    {
        AtomicBlock b;
        busy = false;
    }

private:
    bool busy;
};


class MutexLocker
{
public:
    MutexLocker(Mutex &_mutex, bool doLock = true)
        : mutex(_mutex)
    {
        if (doLock)
        {
            mutex.lock();
        }
    }

    ~MutexLocker()
    {
        mutex.unlock();
    }

private:
    Mutex &mutex;
};

#endif // MUTEX_H
