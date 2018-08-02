/*
 * Contains Logger class definition.
 */

#include "Logger.h"

#include "base/VarArgs.h"

#include "log/LogManager.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>


// Define Logger static manipulators.
const Logger::Flush Logger::s_flush;


Logger::Logger(LogLevel::Type logLevel) throw()
    : m_logLevel(logLevel)
{
}

Logger::~Logger() throw()
{
    sync();
}

Logger& Logger::write(const void* data, size_t len) throw()
{
    LogManager::instance().write(data, len);
    return *this;
}

Logger& Logger::print(const char* str) throw()
{
    return (str != nullptr) ? write(str, ::strlen(str)) : *this;
}

Logger& Logger::println(const char* str /*= nullptr*/) throw()
{
    return print(str).print("\n");
}

Logger& Logger::printf(const char* format, ...) throw()
{
    VarArgs arglist;
    VA_START(arglist, format);
    LogManager::instance().vprintf(format, arglist);
    return *this;
}

Logger& Logger::vprintf(const char* format, va_list arglist) throw()
{
    LogManager::instance().vprintf(format, arglist);
    return *this;
}

Logger& Logger::hexdump(const void* data, size_t len,
    bool hexCapital /*= false*/, bool spaceSeparated /*= false*/,
    size_t bytesPerLine /*= 0*/, bool finalEol /*= true*/) throw()
{
    if (data == nullptr)
        return *this;
    
    const char* const syms = hexCapital ? "0123456789ABCDEF" : "0123456789abcdef";
    
    for (size_t i = 0; i < len; ++i)
    {
        if (i == 0)
            ;  // do nothing
        else if (bytesPerLine > 0 && i % bytesPerLine == 0)
            println();
        else if (spaceSeparated)
            print(" ");
        
        char str[3];
        str[0] = syms[(reinterpret_cast<const uint8_t*>(data)[i] >> 4) & 0xf];
        str[1] = syms[reinterpret_cast<const uint8_t*>(data)[i] & 0xf];
        str[2] = 0;
        print(str);
    }
    
    if (finalEol)
        println();
    
    return *this;
}

void Logger::sync() throw()
{
    LogManager::instance().sync();
}

const Logger::Format Logger::format(const char* format, ...) throw()
{
    VarArgs arglist;
    VA_START(arglist, format);
    return Format(format, arglist);
}

Logger::Format::Format(const char* format, va_list arglist) throw()
{
    ::vsnprintf(m_buf, sizeof(m_buf), format, arglist);
}
