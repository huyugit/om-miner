#ifndef BASE_EXCEPTION_H
#define BASE_EXCEPTION_H
/*
 * Contains BaseException class declaration.
 */

#include "except/ApplicationException.h"


// Define exception class to report base logic errors.
// E.g. type conversion error, out of range exception, etc.
DEFINE_APP_EXCEPTION(BaseException);

#endif  // BASE_EXCEPTION_H
