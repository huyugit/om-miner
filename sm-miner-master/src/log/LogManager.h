#ifndef LOG_MANAGER_H
#define LOG_MANAGER_H
/*
 * Contains LogManager class declaration.
 */

#include "base/NonCopyable.h"

#include "log/Logger.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>


// Forward declaration.
struct LoggerNode;

// A singleton class managing the logging facility for the application.
class LogManager
    : private NonCopyable  // Prevent copy and assignment.
{
// Public constants.
public:
    // Default log category (= "main").
    static const char* const c_mainCategory;
    
    // Default log level.
    static const LogLevel::Type c_defaultLogLevel = LogLevel::LL_INFO;

// Construction/destruction (private).
private:
    // Default constructor.
    LogManager() throw();

    // Destructor.
    ~LogManager() throw();

// Public interface.
public:
    // Returns LogManager singleton instance.
    static LogManager& instance() throw();
    
    // Creates a new logger associated with either default (main) of the specified category.
    Logger& createLogger(LogLevel::Type logLevel = c_defaultLogLevel);
    Logger& createLogger(const char* logCategory, LogLevel::Type logLevel = c_defaultLogLevel);
    
    // Configures all registered loggers of either default (main) or the specified category
    // to use the given log level.
    LogManager& configure(LogLevel::Type logLevel);
    LogManager& configure(const char* logCategory, LogLevel::Type logLevel);

    // Writes raw data pointed by "data" of size "len" to the log.
    void write(const void* data, size_t len) throw();
    
    // Performs the formatted log output similar to the standard printf().
    void printf(const char* format, ...) throw();

    // Performs the formatted log output similar to the standard vprintf().
    void vprintf(const char* format, va_list arglist) throw();
    
    // Flushes buffered log output.
    void sync() throw();

// Member variables.
private:
    // The standard stream to output.
    FILE* m_out;
    
    // A linked list of created loggers.
    LoggerNode* m_loggers;
};

#endif  // LOG_MANAGER_H
