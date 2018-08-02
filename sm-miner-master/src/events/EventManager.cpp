/*
* Contains EventManager class definition.
*/

#include "EventManager.h"

#include "base/DateTimeStr.h"
#include "base/VarArgs.h"
#include "base/StringBuffer.h"
#include "except/SystemException.h"
#include "app/Application.h"
#include "log/LogManager.h"
#include "log/LogCategories.h"
#include "sys/FileDescriptor.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>


// Anonymous namespace.
namespace
{
    // Create Event logger.
    Logger& logger = LogManager::instance().createLogger(LogCategories::c_eventLogCtg);
    
    // Statistic counters associated with event types.
    StatCounter32 s_eventCounters[EventType::NUM_OF_EVENTS];

}  // End of anonymous namespace.


EventManager::EventManager(AppRegistry& appRegistry)
    : AppComponent(appRegistry)
    , m_eventFilePath()
{
}

EventManager::~EventManager() throw()
{
    done();  // Ensure the object is uninitialized properly.
}

void EventManager::doInit(const Config& config)
{
    LOG_TRACE(logger) << "Initializing Event Manager...\n";

    m_eventFilePath = config.eventFilePath;
}

void EventManager::doDone() throw()
{
    LOG_TRACE(logger) << "Uninitializing Event Manager...\n";
}

void EventManager::saveStat()
{
    for (size_t i = 0; i < EventType::NUM_OF_EVENTS; ++i)
    {
        s_eventCounters[i].save();
    }
}

const StatCounter32 &EventManager::getCounter(EventType::Type event) const throw()
{
    assert(event >= 0 && event < EventType::NUM_OF_EVENTS);
    
    return s_eventCounters[event];
}

void EventManager::reportEvent(EventType::Type event) throw()
{
    reportEvent(event, nullptr);
}

void EventManager::reportEvent(EventType::Type event, const char* format, ...) throw()
{
    try
    {
        VarArgs arglist;
        VA_START(arglist, format);
        doReportEvent(event, format, arglist);
    }
    catch (const Exception& e)
    {
        LOG_ERROR(logger) << "ERROR: can not log event into event file!\n"
            << "  -- " << e.what() << "\n";
    }
}

void EventManager::doReportEvent(EventType::Type event, const char* format, va_list arglist)
{
    assert(event >= 0 && event < EventType::NUM_OF_EVENTS);
    
    // Increment event counter.
    ++s_eventCounters[event];
    
    // Retrieve event info.
    const EventType::EventInfo& eventInfo = EventType::getInfo(event);
    
    // Check if the event logging is needed.
    if (!eventInfo.logEvent)
        return;
    
    // Compose log message:
    // YYYY-MM-DD hh:mm:ss ::: EVENT_NAME ::: <user message>
    StringBuffer<8*1024> message;

    message.printf("%s ::: %s ::: ", DateTimeStr().str, eventInfo.name);
    
    if (format != nullptr)
        message.vprintf(format, arglist);
    
    // Replace new line characters with "|".
    for (size_t i = 0, len = message.length(); i < len; ++i)
    {
        if (message[i] == '\n')
            message[i] = '|';
    }
    
    message.print("\n");
    
    // Log event.
    LOG_INFO(logger) << "EVENT: " << message.cdata();

    // Log event to file.
    writeEventData(m_eventFilePath.cdata(), message.cdata(), message.length());
}

void EventManager::writeEventData(const char* fileName, const char* data, size_t len)
{
    FileDescriptor eventFile(::open(fileName, O_CREAT | O_WRONLY | O_APPEND));
    if (!eventFile)
        throw SystemException(errno, "Cannot open event file: %s", fileName);
    
    if (eventFile.write(data, len) < 0)
        throw SystemException(errno, "Event file write error: %s", fileName);
}
