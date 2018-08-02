#ifndef HISTORYSTATMGR_H
#define HISTORYSTATMGR_H

#include <stdint.h>
#include <stddef.h>

#include "stats/hist/HistoryStat.h"


class HistoryStatMgr
{
public:
    static const size_t DELTA_SECONDS   = 30;
    static const size_t MAX_ITEMS       = 120;

    enum Periods {
        PERIOD_DELTA = 0,
        PERIOD_5_MIN,
        PERIOD_15_MIN,
        PERIOD_60_MIN,
        PERIOD_TOTAL,
        MAX_PERIODS,
    };

public:
    void pushItem(const HistoryStat &newItem);

    const HistoryStat& getByPeriod(size_t period) const;

private:
    HistoryStat items[MAX_ITEMS];
    HistoryStat collectedByPeriod[MAX_PERIODS];
};

#endif // HISTORYSTATMGR_H
