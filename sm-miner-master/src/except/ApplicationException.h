#ifndef APPLICATION_EXCEPTION_H
#define APPLICATION_EXCEPTION_H
/*
 * Contains ApplicationException class declaration.
 */

#include "except/Exception.h"

#include "base/BaseUtil.h"
#include "base/StringBuffer.h"
#include "base/VarArgs.h"

#include <stdarg.h>


// A macro to define ApplicationException-based exception.
#define DEFINE_APP_EXCEPTION(APP_EXCEPTION) \
    class APP_EXCEPTION \
        : public ApplicationException \
    { \
    public: \
        explicit APP_EXCEPTION(const char* format, ...) \
            : ApplicationException() \
        { \
            VarArgs arglist; \
            VA_START(arglist, format); \
            m_message.vprintf(format, arglist); \
        } \
    }

// A macro to define ApplicationException-based exception with prefix.
#define DEFINE_APP_EXCEPTION_EX(APP_EXCEPTION_EX, PREFIX) \
    class APP_EXCEPTION_EX \
        : public ApplicationException \
    { \
    public: \
        explicit APP_EXCEPTION_EX(const char* format, ...) \
            : ApplicationException() \
        { \
            VarArgs arglist; \
            VA_START(arglist, format); \
            m_message.print((PREFIX)); \
            m_message.vprintf(format, arglist); \
        } \
    }


// General application exception class derived from Exception.
//
class ApplicationException
    : public Exception
{
// Construction/destruction.
public:
    // Default constructor.
    ApplicationException()
        : Exception()
        , m_message()
    {}

    // Constructs the object formatting an error message
    // based on the given format string and arguments.
    explicit ApplicationException(const char* format, ...)
        : Exception()
        , m_message()
    {
        VarArgs arglist;
        VA_START(arglist, format);
        m_message.vprintf(format, arglist);
    }
    
    // Copy constructor.
    ApplicationException(const ApplicationException& src)
        : Exception()
        , m_message(src.m_message)
    {}

// Operators.
public:
    // Assignment operator.
    ApplicationException& operator=(const ApplicationException& right)
    {
        ApplicationException(right).swap(*this);
        return *this;
    }

// Public interface.
public:
    // Returns a C-style character string describing the error.
    virtual const char* what() const
    {
        return m_message.cdata();
    }

    // Exchanges the content of this object with "other".
    void swap(ApplicationException& other) throw()
    {
        m_message.swap(other.m_message);
    }

// Member variables.
protected:
    // Error message string buffer.
    //
    // Fixed size message buffer implies an additional overhead when copying
    // exception objects. However, exceptions are not designed to be used
    // on a critical path from the performance perspective, so this overhead
    // is acceptable.
    StringBuffer<1024> m_message;
};

#endif  // APPLICATION_EXCEPTION_H
