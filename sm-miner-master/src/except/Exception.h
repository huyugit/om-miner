#ifndef EXCEPTION_H
#define EXCEPTION_H
/*
 * Contains Exception class declaration.
 */


// Base abstract class for all exceptions thrown.
//
class Exception
{
// Construction/destruction.
public:
    // Default constructor.
    Exception() {}

    // Virtual destructor.
    virtual ~Exception() throw()  {}

// Public interface.
public:
    // Returns a C-style character string describing the error.
    virtual const char* what() const = 0;
};

#endif  // EXCEPTION_H
