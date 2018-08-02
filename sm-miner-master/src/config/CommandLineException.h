#ifndef COMMAND_LINE_EXCEPTION_H
#define COMMAND_LINE_EXCEPTION_H
/*
 * Contains CommandLineException class declaration.
 */

#include "except/ApplicationException.h"


// Define exception class to report command line errors.
DEFINE_APP_EXCEPTION_EX(CommandLineException, "Invalid command line:\n");

#endif  // COMMAND_LINE_EXCEPTION_H
