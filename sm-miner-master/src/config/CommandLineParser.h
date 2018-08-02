#ifndef COMMAND_LINE_PARSER_H
#define COMMAND_LINE_PARSER_H
/*
 * Contains CommandLineParser class declaration.
 */

#include "base/NonCopyable.h"

#include <stdlib.h>
#include <stdint.h>


// Forward declarations.
class Config;

// Class for parsing command line parameters.
class CommandLineParser
    : private NonCopyable  // Prevent copy and assignment.
{
// Construction (private).
private:
    CommandLineParser(int argc, const char* argv[]);

// Public interface.
public:
    // Prints command line usage.
    static void showUsage();

    // Parses the application command line into the given config variable.
    static void parse(int argc, const char* argv[], Config& config);

// Implementation methods.
private:
    const char* getArgument(int argn) const;
    const char* getParam(int argn, const char* option, const char* param);
    
    const char* getParamAsStr(int argn, const char* option, const char* param);
    long getParamAsLong(int argn, const char* option, const char* param);
    unsigned long getParamAsULong(int argn, const char* option, const char* param);
    int getParamAsInt(int argn, const char* option, const char* param);
    unsigned int getParamAsUInt(int argn, const char* option, const char* param);
    uint8_t getParamAsUInt8(int argn, const char* option, const char* param);
    uint16_t getParamAsUInt16(int argn, const char* option, const char* param);
    uint32_t getParamAsUInt32(int argn, const char* option, const char* param);
    double getParamAsDouble(int argn, const char* option, const char* param);
    
    static bool optionEqual(const char* option, const char* test);
    static char* uint32ToBinStr(uint32_t value, char* binStr, size_t size);
    static uint32_t binStrToUInt32(const char* binStr);

// Member variables.
private:
    int m_argc;
    const char** m_argv;
};

#endif  // COMMAND_LINE_PARSER_H
