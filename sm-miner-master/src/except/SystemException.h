#ifndef SYSTEM_EXCEPTION_H
#define SYSTEM_EXCEPTION_H
/*
 * Contains SystemException class declaration.
 */

#include "except/Exception.h"

#include "base/BaseUtil.h"
#include "base/StringBuffer.h"
#include "base/VarArgs.h"

#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <error.h>


// Class representing a UNIX error description.
//
class UnixErrorDescription
{
// Construction/destruction.
public:
    explicit UnixErrorDescription(int errnum)
        : m_description(true)  // zeroFill
    {
        init(errnum);
    }

// Public interface.
public:
    // Retrieves and stores in the internal buffer a description
    // of the given UNIX error code.
    void init(int errnum)
    {
        static const size_t bufSize = m_description.capacity() - 1;
        const char* const str = ::strerror_r(errnum, m_description.cdata(), bufSize);
        
        // GNU version of strerror_r() function doesn't usually modify
        // the provided buffer so we must use the function return value.
        // See http://www.club.cc.cmu.edu/~cmccabe/blog_strerror.html for details.
        if (str != m_description.cdata())
        {
            ::strncpy(m_description.cdata(), str, bufSize);
        }
    }
    
    // Returns a pointer to the stored description C-string.
    const char* cstr() const
    {
        return m_description.cdata();
    }

// Member variables.
private:
    // A buffer holding an error description.
    StringBuffer<100> m_description;
};

// Exception class to report system errors.
//
class SystemException
    : public Exception
{
// Constants.
private:
    // Error code indicating custom error described by the user error message.
    static const int c_customError = 0;
    
// Construction/destruction.
public:
    // Default constructor.
    SystemException()
        : Exception()
        , m_message()
        , m_errno(c_customError)
    {}

    // Constructs the object from the given error code, format string and arguments.
    SystemException(int errnum, const char* format, ...)
        : Exception()
        , m_message()
        , m_errno(errnum)
    {
        VarArgs arglist;
        VA_START(arglist, format);
        formatMessage(format, arglist);
    }

    // Constructs the error object as a custom error described by the given message.
    SystemException(const char* format, ...)
        : Exception()
        , m_message()
        , m_errno(c_customError)
    {
        VarArgs arglist;
        VA_START(arglist, format);
        formatMessage(format, arglist);
    }

    // Copy constructor.
    SystemException(const SystemException& src)
        : Exception()
        , m_message(src.m_message)
        , m_errno(src.m_errno)
    {}

// Operators.
public:
    // Assignment operator.
    SystemException& operator=(const SystemException& right)
    {
        SystemException(right).swap(*this);
        return *this;
    }

// Getters/setters.
public:
    int getErrno() const throw()  { return m_errno; }

// Public interface.
public:
    // Returns a C-style character string describing the error.
    virtual const char* what() const
    {
        return m_message.cdata();
    }
    
    // Exchanges the content of this object with "other".
    void swap(SystemException& other) throw()
    {
        m_message.swap(other.m_message);
        util::swap(m_errno, other.m_errno);
    }
    
// Implementation methods.
private:
    // Formats the error message based on the current m_errno,
    // the given format string (same as for printf) and arguments.
    void formatMessage(const char* format, va_list arglist)
    {
        m_message.clear();
        m_message.vprintf(format, arglist);
        
        if (m_errno != c_customError)
        {
            m_message.printf(": %s (%d)", UnixErrorDescription(m_errno).cstr(), m_errno);
        }
    }

// Member variables.
private:
    // Error message string buffer.
    //
    // Fixed size message buffer implies an additional overhead when copying
    // exception objects. However, exceptions are not designed to be used
    // on a critical path from the performance perspective, so this overhead
    // is acceptable.
    StringBuffer<1024> m_message;
    
    // Error code (errno).
    int m_errno;
};

#endif  // SYSTEM_EXCEPTION_H
