/*
 * Contains LogManager class definition.
 */

#include "LogManager.h"

#include "base/VarArgs.h"

#include <assert.h>
#include <string.h>


// Struct representing a node of a linked list of loggers.
struct LoggerNode
{
    Logger logger;  // A logger instance.
    const char* logCategory;  // The associated log category.

    LoggerNode* nextPtr;  // Pointer to the next node or nullptr if last.

    // Default constructor.
    LoggerNode(const char* logCategory, LogLevel::Type logLevel) throw()
        : logger(logLevel)
        , logCategory(logCategory)
        , nextPtr(nullptr)
    {
    }
};


// Define c_mainCategory constant.
const char* const LogManager::c_mainCategory = "main";

// Constructs Log Manager instance.
LogManager::LogManager() throw()
    : m_out(stdout)
    , m_loggers(nullptr)
{
}

LogManager::~LogManager() throw()
{
    sync();
    
    // Delete all loggers.
    LoggerNode* headPtr = m_loggers;
    while (headPtr)
    {
        LoggerNode* const nodePtr = headPtr;
        headPtr = headPtr->nextPtr;
        delete nodePtr;
    }
}

LogManager& LogManager::instance() throw()
{
    static LogManager s_instance;
    return s_instance;
}

Logger& LogManager::createLogger(LogLevel::Type logLevel)
{
    return createLogger(c_mainCategory, logLevel);
}

Logger& LogManager::createLogger(const char* logCategory, LogLevel::Type logLevel)
{
    assert(logCategory != nullptr);

    // Create the new logger node.
    LoggerNode* const newPtr = new LoggerNode(logCategory, logLevel);

    // Add the created logger node to the end of m_loggers list.
    if (m_loggers != nullptr)
    {
        LoggerNode* tailPtr = m_loggers;
        while (tailPtr->nextPtr)
            tailPtr = tailPtr->nextPtr;

        tailPtr->nextPtr = newPtr;
    }
    else
    {
        m_loggers = newPtr;
    }

    // Return a reference to the created logger.
    return newPtr->logger;
}

LogManager& LogManager::configure(LogLevel::Type logLevel)
{
    return configure(c_mainCategory, logLevel);
}

LogManager& LogManager::configure(const char* logCategory, LogLevel::Type logLevel)
{
    assert(logCategory != nullptr);
    
    // Scan m_loggers list and for each logger of the given category
    // set log level to the specified value.
    for (LoggerNode* nodePtr = m_loggers; nodePtr != nullptr; nodePtr = nodePtr->nextPtr)
    {
        if (::strcmp(nodePtr->logCategory, logCategory) == 0)
            nodePtr->logger.setLogLevel(logLevel);
    }
    
    return *this;
}

void LogManager::write(const void* data, size_t len) throw()
{
    if (data != nullptr)
        ::fwrite(data, 1, len, m_out);
}

void LogManager::printf(const char* format, ...) throw()
{
    VarArgs arglist;
    VA_START(arglist, format);
    vprintf(format, arglist);
}

void LogManager::vprintf(const char* format, va_list arglist) throw()
{
    if (format != nullptr)
        ::vfprintf(m_out, format, arglist);
}

void LogManager::sync() throw()
{
    ::fflush(m_out);
}
