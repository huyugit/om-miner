#ifndef HISTORYSTAT_H
#define HISTORYSTAT_H

#include <stdint.h>
#include <stddef.h>


class TotalStat;

class HistoryStat
{
public:
    const char* intervalName;
    uint32_t intervalExpected;

    uint64_t intervalMs;

    // total stat
    uint64_t solutions;
    uint64_t errors;
    uint64_t jobsDone;
    uint64_t restarts;

    uint64_t pwcSolByDiff;
    uint32_t pwcSharesSent;
    uint32_t pwcSharesDropped;

    // total static stat: can not be aggregated!
    bool staticSet;
    uint32_t chips;
    uint32_t boards;

    // pool stat
    uint64_t poolTotalTime;
    uint64_t poolInService;
    
    uint32_t poolReceivedJobs;
    uint32_t poolReceivedJobsWithClean;
    
    uint32_t poolSentShares;
    uint32_t poolAcceptedShares;
    uint32_t poolRejectedShares;
    uint64_t poolAcceptedSolutions;

    // event stat
    uint32_t poolSubscribeError;
    uint32_t poolDiffChanges;
    uint32_t poolReconnections;
    uint32_t poolReconnectionsOnError;
    uint32_t poolDefaultJobShares;
    uint32_t poolStaleJobShares;
    uint32_t poolDuplicateShares;
    uint32_t poolLowDifficultyShares;

public:
    HistoryStat();

    void aggregate(const HistoryStat &other);

    void loadTotalStat(const TotalStat &total, bool useTotal);
    void loadEventDiffs();
    
    double getShareLoss() const;

    unsigned int getSeconds() const;
};

#endif // HISTORYSTAT_H
