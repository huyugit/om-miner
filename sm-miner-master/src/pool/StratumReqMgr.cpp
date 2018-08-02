#include "StratumReqMgr.h"

#include <time.h>

#include <app/Application.h>
#include <events/EventManager.h>


// Shortcut for event reporting call.
#define REPORT_EVENT \
    Application::events().reportEvent


StratumReqMgr::StratumReqMgr()
{
}

bool StratumReqMgr::isDuplicate(const StratumShare &share)
{
    Mutex::Lock lock(m_mutex);

    for (RecordsMap::iterator it = records.begin(); it != records.end(); it++)
    {
        if (it->second.share == share)
        {
            return true;
        }
    }
    return false;
}

void StratumReqMgr::shareSent(int reqId, const StratumShare &share)
{
    Mutex::Lock lock(m_mutex);

    clenupOldRecords();

    ReqRecord r;
    r.time = getSeconds();
    r.share = share;

    records[reqId] = r;
}

void StratumReqMgr::remove(int reqId)
{
    Mutex::Lock lock(m_mutex);

    RecordsMap::iterator it = records.find(reqId);
    if (it != records.end())
    {
        records.erase(it);
    }

    clenupOldRecords();
}

bool StratumReqMgr::getShare(int reqId, StratumShare &share, int &age)
{
    bool result = false;
    Mutex::Lock lock(m_mutex);

    RecordsMap::iterator it = records.find(reqId);
    if (it != records.end())
    {
        share = it->second.share;
        age = getSeconds() - it->second.time;

        result = true;
    }

    clenupOldRecords();
    return result;
}

int StratumReqMgr::getSeconds()
{
    struct timespec ts;
    if (::clock_gettime(CLOCK_MONOTONIC_RAW, &ts) < 0)
        printf("StratumReqMgr: ERROR: clock_gettime() failed!\n");

    return ts.tv_sec;
}

void StratumReqMgr::clenupOldRecords()
{
    while (true)
    {
        RecordsMap::iterator it = records.begin();
        if (it == records.end()) break;

        int age = getSeconds() - it->second.time;

        char shareStr[512];
        it->second.share.toString(shareStr, sizeof(shareStr));

        if (records.size() > 128 || age > 36)
        {
//            printf("StratumReqMgr: cleanup OLD record: reqId[%d]: age %d sec, SHARE: %s\n",
//                   it->first, age, shareStr);

            records.erase(it);
        }

        break;
    }
}
