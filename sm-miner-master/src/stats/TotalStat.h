#ifndef TOTALSTAT_H
#define TOTALSTAT_H

#include <stdint.h>
#include "stats/StatCounter.h"

class ChipStat;
class SlaveStat;

class TotalStat
{
public:
    TotalStat();
    TotalStat(const ChipStat &chip);

    void clear();
    void aggregate(const TotalStat &other);
    void aggregate(const ChipStat &chip);
    void save();

    int chips;
    int boards;
    int numBrokenSpi;
    int numBrokenPwc;
    StatCounter64 solutions;
    StatCounter64 errors;
    StatCounter64 jobsDone;
    StatCounter64 restarts;   // chip restarts

    StatCounter64 pwcSolByDiff;
    StatCounter64 pwcSharesSent;
    StatCounter64 pwcSharesDropped;

    double psuCurrent;
    double psuPower;

    uint32_t current;
    uint32_t currentA;
    uint32_t power;
};

#endif // TOTALSTAT_H
