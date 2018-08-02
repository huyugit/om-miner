#ifndef SIGNAL_PIPE_H
#define SIGNAL_PIPE_H
/*
 * Contains SignalPipe class declaration.
 */

#include "base/NonCopyable.h"
#include "sys/SocketPair.h"


// Control pipe to send wake signal from one end to another.
// The implementation is based on a pair of connected sockets.
//
class SignalPipe
    : private NonCopyable  // Prevent copy and assignment.
{
// Construction/destruction.
public:
    // Constructs the object creating a pair of sockets by calling SignalPipe().
    SignalPipe()
        : m_socketPair()
    {}

// Public interface.
public:
    // Returns raw file descriptor for polling.
    inline int getDescriptor()
    {
        return m_socketPair.getSock2().getDescriptor();
    }
    
    // Send notification to consumer.
    // Upon receiving the notification, the signaled party should check
    // what kind of processing is needed.
    void signal();
    
    // Reads all notification data from the notification queue.
    // Should be called upon receiving the signal to clear that queue.
    void consumeNotificationData();
    
    // Exchanges the content of this object with "other".
    void swap(SignalPipe& other) throw()
    {
        m_socketPair.swap(other.m_socketPair);
    }

// Member variables.
private:
    // A pair of connected sockets to send signals.
    SocketPair m_socketPair;
};

#endif  // SIGNAL_PIPE_H
