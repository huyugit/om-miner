#ifndef EXIT_EXCEPTION_H
#define EXIT_EXCEPTION_H
/*
 * Contains ExitException class declaration.
 */

#include "except/Exception.h"


// An utility exception class to fall out from the current "try {} catch" block.
// 
// The usage is illustrated by the following code snippet:
// 
// try {
//     // ...
//     ExitException::raise();
//     // ...
// }
// catch {const ExitException& ) {
//   ;  // Exited.
// }
//
class ExitException
    : public Exception
{
// Construction/destruction.
public:
    // Default constructor.
    ExitException()  {}

// Public interface.
public:
    // Returns a C-style character string describing the error.
    virtual const char* what() const  { return "Exit exception"; }
};

#endif  // EXIT_EXCEPTION_H
