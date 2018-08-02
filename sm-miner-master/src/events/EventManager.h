#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H
/*
* Contains EventManager class declaration.
*/

#include "base/StringBuffer.h"
#include "app/AppComponent.h"
#include "config/Config.h"
#include "events/EventType.h"
#include "stats/StatCounter.h"

#include <stdarg.h>


// Class used to report application events.
//
class EventManager
    : protected AppComponent
{
friend class ApplicationImpl;

// Construction/destruction.
public:
    EventManager(AppRegistry& appRegistry);
    ~EventManager() throw();

// AppComponent implementation (partial).
protected:
    virtual void doInit(const Config& config);
    virtual void doDone() throw();

// Public interface.
public:
    void reportEvent(EventType::Type event) throw();
    void reportEvent(EventType::Type event, const char* format, ...) throw();

    void saveStat();
    
    const StatCounter32& getCounter(EventType::Type event) const throw();

// Implementation methods.
private:
    void doReportEvent(EventType::Type event, const char* format, va_list arglist);
    void writeEventData(const char *fileName, const char* data, size_t len);

// Member variables.
private:
    // Full path of the event output file.
    StringBuffer<256> m_eventFilePath;
};

#endif // EVENT_MANAGER_H
