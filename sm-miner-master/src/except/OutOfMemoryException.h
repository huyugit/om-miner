#ifndef OUT_OF_MEMORY_EXCEPTION_H
#define OUT_OF_MEMORY_EXCEPTION_H
/*
 * Contains OutOfMemoryException class declaration.
 */

#include "except/Exception.h"


// Exception class to report Out-of-Memory condition.
//
class OutOfMemoryException
    : public Exception
{
// Construction/destruction.
public:
    // Default constructor.
    OutOfMemoryException()  {}

// Public interface.
public:
    // Returns a C-style character string describing the error.
    virtual const char* what() const  { return "Out of memory"; }
};

#endif  // OUT_OF_MEMORY_EXCEPTION_H
