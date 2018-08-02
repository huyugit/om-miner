#ifndef VAR_ARGS_H
#define VAR_ARGS_H
/*
 * Contains VarArgs class declaration.
 */

#include "base/NonCopyable.h"

#include <stdarg.h>


// Initializes a given VarArgs instance similar to the standard "va_start" macro.
#define VA_START(VARARGS, ARG) \
    va_start((VARARGS).init(), (ARG))

// Returns the value of the next optional argument similar to "va_end".
#define VA_ARG(VARARGS, T) \
    va_arg((VARARGS).operator va_list&(), T)


// Class holding a standard argument pointer used in variadic functions.
// The purpose is to call "va_end" macro for that pointer automatically.
// In GCC and many other compilers "va_end" does nothing, however,
// it's nice to have for portability reason.
// 
// Use this class object with VA_START and VA_ARG macros instead of
// the standard "va_start" and "va_end" analogues. There is not need
// to explicitly call "va_end".
// 
class VarArgs
    : private NonCopyable  // Prevent copy and assignment.
{
// Construction/destruction.
public:
    // Default constructor.
    VarArgs()
        : m_initialized(false)
    {}
    
    // Destructor.
    ~VarArgs() throw()
    {
        done();
    }

// Operators.
public:
    // Converts this to the standard argument pointer.
    operator va_list&() throw()
    {
        return m_ap;
    }

// Public interface.
public:
    // Marks the stored argument pointer as "initialized" and also
    // returns a reference to it.
    va_list& init() throw()
    {
        m_initialized = true;
        return m_ap;
    }
    
    // Uninitializes the stored argument pointer.
    void done() throw()
    {
        if (m_initialized)
        {
            va_end(m_ap);
            m_initialized = false;
        }
    }

// Member variables.
private:
    // The standard argument pointer variable.
    va_list m_ap;
    
    // Whether the argument pointer was initialized.
    bool m_initialized;
};

#endif  // VAR_ARGS_H
