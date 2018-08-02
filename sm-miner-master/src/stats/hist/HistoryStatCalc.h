#ifndef HISTORYSTATCALC_H
#define HISTORYSTATCALC_H

#include "stats/hist/HistoryStat.h"
#include "stats/TotalStat.h"


class HistoryStatCalc
{
public:
    HistoryStatCalc(size_t period);
    HistoryStatCalc(const TotalStat &total, bool useTotal);

    const HistoryStat& getStat() const { return stat; }

    int getSeconds() const;
    uint64_t getSol() const;
    uint64_t getErr() const;
    uint64_t getChipRestarts() const;

    double getGHs() const;
    double getGHsByDiff() const;
    double getGHsByJobs() const;
    double getGHsByPool() const;

    double getErrToSol() const;

    double getChipGHs() const;

private:
    HistoryStat stat;
};

#endif // HISTORYSTATCALC_H
