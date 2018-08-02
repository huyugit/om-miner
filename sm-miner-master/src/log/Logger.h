#ifndef LOGGER_H
#define LOGGER_H
/*
 * Contains Logger class declaration.
 */

#include "base/NonCopyable.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>


// A set of the logging macros.
// 
// Each macro requires a Logger instance to be passed as an argument.
// Loggers are created via "LogManager::instance().createLogger(<logCategory>)" call.
// <logCategory> here is an optional log category name, configured in advance.
// 
// The basic macro is LOG_IF(LOGGER, CONDITION), used as following:
// LOG_IF(LOGGER, CONDITION) << "Sample log message\n";
// If the specified CONDITION is true, it outputs "Sample log message\n" string
// using the given LOGGER.
//
// LOG(LOGGER) - logs unconditionally.
// LOG_INFO(LOGGER) - logs informational message (if permitted by the current log level setting).
// LOG_TRACE(LOGGER) - logs tracing message (if permitted by the current log level setting).
// LOG_ERROR(LOGGER) - identical to LOG_INFO, but the message is prefixed with "ERROR: ".
// LOG_WARN(LOGGER) - identical to LOG_INFO, but the message is prefixed with "WARNING: ";
// 
// LOG_INFO_IF(LOGGER, CONDITION) - if CONDITION is true, identical to LOG_INFO, otherwise doesn't log.
// LOG_TRACE_IF(LOGGER, CONDITION) - if CONDITION is true, identical to LOG_TRACE, otherwise doesn't log.
// 
// Usage example:
// LOG_INFO(LOGGER) << "Sample log message\n";
// 
#define LOG_IF(LOGGER, CONDITION)           if (!(CONDITION)) ; else (LOGGER)
#define LOG_INFO_IF(LOGGER, CONDITION)      LOG_IF((LOGGER), (LOGGER).admitsInfo() && (CONDITION))
#define LOG_TRACE_IF(LOGGER, CONDITION)     LOG_IF((LOGGER), (LOGGER).admitsTrace() && (CONDITION))
//
#define LOG(LOGGER)         LOG_IF((LOGGER), true)
#define LOG_INFO(LOGGER)    LOG_INFO_IF((LOGGER), true)
#define LOG_TRACE(LOGGER)   LOG_TRACE_IF((LOGGER), true)
#define LOG_ERROR(LOGGER)   LOG_INFO_IF((LOGGER), ((LOGGER) << "ERROR: ", true))
#define LOG_WARN(LOGGER)    LOG_INFO_IF((LOGGER), ((LOGGER) << "WARNING: ", true))


// Log level enumeration.
struct LogLevel
{
    enum Type
    {
        LL_NONE,   // No logging output.
        LL_INFO,   // Production log level.
        LL_TRACE   // Log all the details.
    };
};

// Class implementing logging interface.
// Defines a set of overloading "<<" operators for stream output.
// All errors during the log output are simply ignored.
//
class Logger
    : private NonCopyable  // Prevent copy and assignment.
{
friend class LoggerNode;

// Log stream manipulators.
public:
    // Manipulator class used to flush the log stream.
    // See flush() for more information.
    class Flush
    {
    friend class Logger;
    private:
        // Private destructor to prevent instantiation outside of Logger.
        ~Flush() throw()  {}
    };
    
    // Manipulator class used to combine formatting output with the stream output.
    // See format() for more information.
    class Format
    {
    friend class Logger;
    private:
        Format(const char* format, va_list arglist) throw();
        char m_buf[256];
    };
    
// Construction/destruction.
private:
    // Constructor.
    explicit Logger(LogLevel::Type logLevel) throw();

    // Destructor.
    ~Logger() throw();

// Stream output operators.
public:
    Logger& operator<<(const char* value) throw()
    {
        return print(value);
    }
    Logger& operator<<(bool value) throw()
    {
        return print(value ? "true" : "false");
    }
    Logger& operator<<(char value) throw()
    {
        return printf("%c", value);
    }
    Logger& operator<<(int value) throw()
    {
        return printf("%d", value);
    }
    Logger& operator<<(unsigned int value) throw()
    {
        return printf("%u", value);
    }
    Logger& operator<<(long int value) throw()
    {
        return printf("%ld", value);
    }
    Logger& operator<<(long unsigned int value) throw()
    {
        return printf("%lu", value);
    }
    Logger& operator<<(double value) throw()
    {
        return printf("%f", value);
    }
    Logger& operator<<(const Flush&) throw()
    {
        sync();
        return *this;
    }
    Logger& operator<<(const Format& fmt) throw()
    {
        return print(fmt.m_buf);
    }

// Public interface.
public:
    // Sets/gets log level property.
    inline void setLogLevel(LogLevel::Type logLevel) throw()  { m_logLevel = logLevel; }
    inline LogLevel::Type getLogLevel() const throw()  { return m_logLevel; }

    // Checks if the current logger configuration admits logging at the given log level.
    inline bool admitsLevel(LogLevel::Type logLevel) const throw()
    {
        return (m_logLevel >= logLevel);
    }
    
    // Helper functions to check if logging at a particular log level is admitted.
    inline bool admitsInfo() const throw()  { return admitsLevel(LogLevel::LL_INFO); }
    inline bool admitsTrace() const throw()  { return admitsLevel(LogLevel::LL_TRACE); }
    
    inline static LogLevel::Type traceOrInfo(bool traceIf)
    {
        return (traceIf) ? LogLevel::LL_TRACE : LogLevel::LL_INFO;
    }
    
    // Writes the raw data pointed by "data" of size "len" into the log.
    Logger& write(const void* data, size_t len) throw();
    
    // Writes a given string into the log.
    Logger& print(const char* str) throw();
    
    // Writes a given string into the log followed by end-of-line character.
    Logger& println(const char* str = nullptr) throw();
    
    // Performs the formatted log output similar to the standard printf().
    Logger& printf(const char* format, ...) throw();

    // Performs the formatted log output similar to the standard vprintf().
    Logger& vprintf(const char* format, va_list arglist) throw();
    
    // Logs the given binary data as hex string.
    // 
    // data - the binary data to log.
    // len - the length of the binary data.
    // hexCapital - if true,.hexadecimal characters will appear in the upper case.
    // spaceSeparated - whether to output space after logging each data bytes.
    // bytesPerLine - a number of bytes per line (0 - no line separators).
    // lineSeparator - line separator (used if bytesPerLine > 0).
    // finalEol - if true, print end-of-line character at the end.
    // 
    Logger& hexdump(const void* data, size_t len,
        bool hexCapital = false, bool spaceSeparated = false,
        size_t bytesPerLine = 0, bool finalEol = true) throw();
    
    // Flush all buffered stream output.
    void sync() throw();
    
    // Returns Flush manipulator instance, that, if passed
    // to "<<" operator, causes all buffered stream data to flush.
    //
    // E.g.: LOG_INFO(logger) << Logger::flush();
    //
    static const Flush& flush() throw()  { return s_flush; }
    
    // Returns Format manipulator instance containing the data
    // formatted according to the specified format string and arguments.
    static const Format format(const char* format, ...) throw();
    
// Member variables.
private:
    // Log level.
    volatile LogLevel::Type m_logLevel;
    
    // Static manipulator instances.
    static const Flush s_flush;
};

#endif  // LOGGER_H
