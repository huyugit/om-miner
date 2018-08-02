/*
 * Contains SocketPair class definition.
 */

#include "SocketPair.h"

#include <sys/socket.h>

#include <stdio.h>
#include <errno.h>
#include <error.h>
#include <assert.h>


SocketPair::SocketPair()
    : m_sock1()
    , m_sock2()
{
    int fds[2];
    if (::socketpair(AF_LOCAL, SOCK_STREAM, 0, fds) == -1)
    {
        ::error(0, errno, "socketpair() failed");
        return;
    }

    assert(fds[0] >= 0);
    assert(fds[1] >= 0);
    
    m_sock1.attach(fds[0]);
    m_sock2.attach(fds[1]);
}
