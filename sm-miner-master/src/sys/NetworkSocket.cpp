/*
 * Contains NetworkSocket class definition.
 */

#include "NetworkSocket.h"

#include "except/SystemException.h"

#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <netdb.h>

#include <sys/socket.h>


NetworkSocket::NetworkSocket() throw()
    : m_sock()
{}

NetworkSocket::~NetworkSocket() throw()
{
    close();
}

void NetworkSocket::connect(const char* host, const char* port)
{
    assert(host != nullptr);
    assert(port != nullptr);
    
    close();
    
    struct addrinfo hints = {};
    hints.ai_family = AF_UNSPEC;  // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;  // Stream socket
    hints.ai_protocol = 0;  // Any protocol
    hints.ai_flags = 0;
    
    struct addrinfo* result;
    if (int rc = ::getaddrinfo(host, port, &hints, &result))
        throw SystemException("getaddrinfo failed: %s\n", gai_strerror(rc));
    
    // getaddrinfo() returns a list of address structures.
    // Try each address until we successfully connect.
    // If socket (or connect) fails, we close the socket
    // and try the next address.
    for (struct addrinfo* rp = result; rp != nullptr; rp = rp->ai_next)
    {
        FileDescriptor sock(::socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol));
        if (!sock)
            continue;  // Try the next address.

        if (::connect(sock.getDescriptor(), rp->ai_addr, rp->ai_addrlen) == -1)
            continue;
        
        m_sock.swap(sock);
        break;  // Success 
    }
    
    ::freeaddrinfo(result);
    
    if (!m_sock)  // No address succeeded.
        throw SystemException("Unable to connect.");
}

size_t NetworkSocket::receive(void* buffer, size_t size, int flags /*= 0*/)
{
    if (size == 0)
        return 0;
    
    assert(buffer != nullptr);
    const ssize_t result = ::recv(m_sock.getDescriptor(), buffer, size, flags);
    if (result < 0)
        throw SystemException(errno, "Error receiving data");
    else if (result == 0)
        throw SystemException("Connection closed by the other side while receiving data.");
    
    return static_cast<size_t>(result);
}

size_t NetworkSocket::send(const void* buffer, size_t size, int flags /*= 0*/)
{
    if (size == 0)
        return 0;
    
    assert(buffer != nullptr);
    const ssize_t result = ::send(m_sock.getDescriptor(), buffer, size, flags);
    if (result < 0)
        throw SystemException(errno, "Error sending data");
    else if (result == 0)
        throw SystemException("Connection closed by the other side while sending data.");

    return static_cast<size_t>(result);
}

void NetworkSocket::shutdown()
{
    if (!m_sock)
        return;
    
    if (::shutdown(m_sock.getDescriptor(), SHUT_RDWR) == -1)
        ::perror("Socket shutdown error");
}

void NetworkSocket::close() throw()
{
    shutdown();
    m_sock.close();
}
