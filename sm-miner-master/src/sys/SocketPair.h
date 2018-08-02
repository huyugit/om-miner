#ifndef SOCKET_PAIR_H
#define SOCKET_PAIR_H
/*
 * Contains SocketPair class declaration.
 */

#include "base/NonCopyable.h"
#include "sys/FileDescriptor.h"


// This is a wrapper class over a pair of sockets connected to each other
// (see socketpair() function defined in <sys/socket.h>).
//
class SocketPair
    : private NonCopyable  // Prevent copy and assignment.
{
// Construction/destruction.
public:
    // Constructs the object creating a pair of sockets by calling socketpair().
    SocketPair();
    
    // Constructs the object from a pair of the raw file descriptors.
    SocketPair(int fd1, int fd2)
        : m_sock1(fd1)
        , m_sock2(fd2)
    {}

// Public interface.
public:
    // Returns the first file descriptor of the pair.
    FileDescriptor& getSock1()  { return m_sock1; }
    const FileDescriptor& getSock1() const  { return m_sock1; }

    // Returns the second file descriptor of the pair.
    FileDescriptor& getSock2()  { return m_sock2; }
    const FileDescriptor& getSock2() const  { return m_sock2; }

    // Exchanges the content of this object with "other".
    void swap(SocketPair& other) throw()
    {
        m_sock1.swap(other.m_sock1);
        m_sock2.swap(other.m_sock2);
    }

// Member variables.
private:
    FileDescriptor m_sock1, m_sock2;
};

#endif  // SOCKET_PAIR_H
