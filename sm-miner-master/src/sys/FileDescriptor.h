#ifndef FILE_DESCRIPTOR_H
#define FILE_DESCRIPTOR_H
/*
 * Contains FileDescriptor class declaration.
 */

#include "base/NonCopyable.h"
#include "base/BaseUtil.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>



// This is a wrapping class for a system file descriptor.
// Closes the owned descriptor on destruction.
class FileDescriptor
    : private NonCopyable  // Prevent copy and assignment.
{
// Static constants.
public:
    // Invalid file descriptor.
    static const int INVALID_FD = -1;

// Construction/destruction.
public:
    // Constructs the empty (invalid) FileDescriptor object.
    FileDescriptor() throw() : m_fd(INVALID_FD)  {}

    // Constructs the object from the provided raw file descriptor.
    explicit FileDescriptor(int fd) throw() : m_fd(fd)  {}
    
    // Destroys the object. Closes the owned file descriptor if open.
    ~FileDescriptor() throw()  { close(); }

// Operators.
public:
    // Test if the owned file descriptor is invalid.
    inline bool operator!() throw()  { return isInvalid(); }

// Public interface.
public:
    // Returns the raw file descriptor value.
    inline int getDescriptor() const throw()  { return m_fd; }

    // Returns true if the owned file descriptor is invalid.
    inline bool isInvalid() const throw()  { return (m_fd < 0); }

    // Attaches a given file descriptor to this object.
    // The previously owned file descriptor is closed.
    void attach(int fd) throw()
    {
        close();
        m_fd = fd;
    }

    // Attaches a given file descriptor to this object.
    // The previously owned file descriptor is closed.
    int detach() throw()
    {
        const int fd = m_fd;
        m_fd = INVALID_FD;
        return fd;
    }

    // Reads up to "size" bytes from the current file into the "buffer".
    // Returns the number of bytes actually read or -1 in case of an error.
    inline ssize_t read(void* buffer, size_t size) throw()
    {
        return ::read(m_fd, buffer, size);
    }

    // Writes up to "size" bytes from the "buffer" into the current file.
    // Returns the number of bytes actually written or -1 in case of an error.
    inline ssize_t write(const void* buffer, size_t size) throw()
    {
        return ::write(m_fd, buffer, size);
    }

    // Ensures all data associated with the current file descriptor is written.
    // Returns 0 if succeeded, otherwise returns -1.
    int fsync() throw()
    {
        return ::fsync(m_fd);
    }

    // Duplicates the owned file descriptor (mimics dup2 from unistd.h).
    int dup(int fd = INVALID_FD) const  throw()
    {
        return ::dup2(m_fd, fd);
    }

    // Closes the open file descriptor. If it is closed already, does nothing.
    bool close() throw()
    {
        bool succeeded = true;
        if (m_fd != INVALID_FD)
        {
            if (::close(m_fd) == -1)
                succeeded = false;

            m_fd = INVALID_FD;
        }

        return succeeded;
    }

    // Exchanges the content of this object with "other".
    void swap(FileDescriptor& other) throw()
    {
        util::swap(m_fd, other.m_fd);
    }

// Member variables.
private:
    // File descriptor.
    int m_fd;
};

#endif  // FILE_DESCRIPTOR_H
