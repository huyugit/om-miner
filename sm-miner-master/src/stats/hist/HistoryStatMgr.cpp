#include "HistoryStatMgr.h"

#include <assert.h>
#include "base/BaseUtil.h"
#include "app/Application.h"
#include "pool/StratumPool.h"


struct PeriodInfo
{
    const char* name;
    size_t len;
};

static PeriodInfo g_periodInfo[HistoryStatMgr::MAX_PERIODS] =
{
    { "30 sec",     1 },
    { "5 min",     10 },
    { "15 min",    30 },
    { "1 hour",   120 },
    { "total",      0 },
};


void HistoryStatMgr::pushItem(const HistoryStat &newItem)
{
//    if (items[0].intervalMs + newItem.intervalMs > (DELTA_SECONDS + 3) * 1000)
//    {
        // shift array of item to free first cell
        for (ssize_t i = util::arrayLength(items) - 1; i > 0; i--)
        {
            items[i] = items[i - 1];
        }

        // put new item into the first cell
        items[0] = newItem;
//    }
//    else
//    {
//        items[0].aggregate(newItem);
//    }

    // recalculate collected items
    for (size_t period = 0; period < MAX_PERIODS; period++)
    {
        HistoryStat& collected = collectedByPeriod[period];

        if (period == PERIOD_TOTAL)
        {
            // special case for the total period
            collected.aggregate(newItem);
        }
        else
        {
            collected = HistoryStat();

            // WARNING: aggregate from OLD to NEW items!!!
            for (ssize_t i = g_periodInfo[period].len - 1; i >= 0; i--)
            {
                collected.aggregate(items[i]);
            }
        }

        collected.intervalName = g_periodInfo[period].name;
        collected.intervalExpected = g_periodInfo[period].len * DELTA_SECONDS;
    }
}

const HistoryStat& HistoryStatMgr::getByPeriod(size_t period) const
{
    assert(period < util::arrayLength(collectedByPeriod));
    return collectedByPeriod[period];
}

