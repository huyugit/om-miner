#ifndef EVENT_TYPE_H
#define EVENT_TYPE_H
/*
 * Contains EventType class declaration.
 */

#include "base/BaseUtil.h"

#include <assert.h>


// A wrapper struct for enumeration of the supported event types.
// It is also a repository of the event types associated details.
//
struct EventType
{
    // It's important that the enum values are assigned sequentially
    // starting from zero as they are used as indexes for an array access.
    // 
    // Attention! When adding a new event type, do not forget to add
    // a corresponded event name in toString() static method!
    //
    enum Type
    {
        MINER_START = 0,
        MINER_ALIVE,  // Miner periodic 'keep alive' event.
        POOL_INFO,
        POOL_ERROR,
        SUBSCRIBE_ERROR,
        DIFF_CHANGE,
        RECONNECTION,
        RECONNECTION_ON_ERROR,
        DEFAULT_JOB_SHARE,
        STALE_JOB_SHARE,  // No active job found for the received nonce.
        DUPLICATE_SHARE,
        LOW_DIFF_SHARE,  // Calculated share discarded as low difficulty.
        POOL_SHARE_DISCARDED,
        POOL_JOB,
        POOL_CLEAN_JOB,
        SHARE_SENT,

        // A number of event types listed above.
        NUM_OF_EVENTS
    };

    // Structure holding additional details associated with each event type.
    struct EventInfo
    {
        const char* name;
        bool logEvent;

        EventInfo(const char* name, bool logEvent = true) throw()
            : name(name)
            , logEvent(logEvent)
        {
            assert(name != nullptr);
        }
    };

    // Returns additional details associated with the given event type.
    static const EventInfo& getInfo(Type value) throw()
    {
        static const EventInfo repository[] =
            { EventInfo("MINER_START")
            , EventInfo("MINER_ALIVE")
            , EventInfo("POOL_INFO")
            , EventInfo("POOL_ERROR")
            , EventInfo("SUBSCRIBE_ERROR")
            , EventInfo("DIFF_CHANGE")
            , EventInfo("RECONNECTION")
            , EventInfo("RECONNECTION_ON_ERROR")
            , EventInfo("DEFAULT_JOB_SHARE")
            , EventInfo("STALE_JOB_SHARE")
            , EventInfo("DUPLICATE_SHARE")
            , EventInfo("LOW_DIFF_SHARE")
            , EventInfo("POOL_SHARE_DISCARDED")
            , EventInfo("POOL_JOB")
            , EventInfo("POOL_CLEAN_JOB")
            , EventInfo("SHARE_SENT")
            };

        assert(util::arrayLength(repository) == NUM_OF_EVENTS);
        assert(value >= 0 && value < NUM_OF_EVENTS);
        
        return repository[value];
    }
    
    // Returns name of the given event type.
    static const char* toString(Type value) throw()
    {
        return getInfo(value).name;
    }
};

#endif  // EVENT_TYPE_H
