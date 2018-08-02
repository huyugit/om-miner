#ifndef NETWORK_SOCKET_H
#define NETWORK_SOCKET_H
/*
 * Contains NetworkSocket class declaration.
 */

#include "base/NonCopyable.h"

#include "sys/FileDescriptor.h"


// This class provides socket-based interface for network communication.
class NetworkSocket
    : private NonCopyable  // Prevent copy and assignment.
{
    // Construction/destruction.
public:
    // Constructs the socket object.
    NetworkSocket() throw();

    // Destroys the socket object.
    // Ensures the raw socket is closed.
    ~NetworkSocket() throw();

// Public interface.
public:
    // Return the raw socket file descriptor.
    inline int getDescriptor() const  { return m_sock.getDescriptor(); }

    // Returns true if the socket is closed.
    inline bool isClosed() const  { return m_sock.isInvalid(); }

    // Opens TCP/IP connection with the remote host.
    void connect(const char* host, const char* port);

    // Receives up to "size" bytes into the "buffer" from the socket.
    // Returns the number of bytes actually received or -1 in case of an error.
    size_t receive(void* buffer, size_t size, int flags = 0);

    // Sends up to "size" bytes from the "buffer" over the socket.
    // Returns the number of bytes actually sent or -1 in case of an error.
    size_t send(const void* buffer, size_t size, int flags = 0);

    // Closes the network connection.
    void shutdown();
    
    // Closes the raw socket objects cleaning up the resources.
    void close() throw();

// Member variables.
private:
    FileDescriptor m_sock;
};

#endif  // NETWORK_SOCKET_H
