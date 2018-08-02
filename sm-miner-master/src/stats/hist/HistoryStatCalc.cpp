#include "HistoryStatCalc.h"

#include <stdio.h>

#include "stats/MasterStat.h"
#include "stats/hist/HistoryStatMgr.h"


HistoryStatCalc::HistoryStatCalc(size_t period)
    : stat( g_masterStat.getHistory().getByPeriod(period) )
{
}

HistoryStatCalc::HistoryStatCalc(const TotalStat &total, bool useTotal)
{
    stat = g_masterStat.getHistory().getByPeriod(
                useTotal ? HistoryStatMgr::PERIOD_TOTAL : HistoryStatMgr::PERIOD_DELTA );

    stat.loadTotalStat(total, useTotal);
}


int HistoryStatCalc::getSeconds() const {
    return stat.getSeconds();
}

uint64_t HistoryStatCalc::getSol() const {
    return stat.solutions;
}

uint64_t HistoryStatCalc::getErr() const {
    return stat.errors;
}

uint64_t HistoryStatCalc::getChipRestarts() const {
    return stat.restarts;
}


double HistoryStatCalc::getGHs() const {
    if (stat.intervalMs > 0)
        return (double)stat.solutions / stat.getSeconds() * 4.295;
    else
        return 0;
}

double HistoryStatCalc::getGHsByDiff() const {
    if (stat.intervalMs > 0)
        return (double)stat.pwcSolByDiff / stat.getSeconds() * 4.295;
    else
        return 0;
}

// 2^32 = 4.295G

// BTC55_V1: cores matrix = 21 x 36 = 756 (3 from 24 columns are broken)
// BTC55_V1: 1 job = 4.295GH / 1024 * 756 = 3.171GH
static const double GH_PER_JOB_BTC55_V1 = 3.171;

// BTC55_V2: cores matrix = 24 x 36 = 864
// BTC55_V2: 1 job = 4.295G / 1024 * 864 = 3.624GH
static const double GH_PER_JOB_BTC55_V2 = 3.624;

// BTC28: cores matrix = 4016
// BTC28: 1 job = 4.295G / 4096 * 4016 = 4.211GH
static const double GH_PER_JOB_BTC28 = 4.211;

// BTC16: cores matrix = 8162
// BTC16: 1 job = 4.295G / 8192 * 8162 = 4.279GH
static const double GH_PER_JOB_BTC16 = 4.279;

double HistoryStatCalc::getGHsByJobs() const {

    if (stat.intervalMs > 0)
        return (double)stat.jobsDone / stat.getSeconds() * GH_PER_JOB_BTC16;
    else
        return 0;
}

double HistoryStatCalc::getGHsByPool() const {
    if (stat.intervalMs > 0)
        return (double)stat.poolAcceptedSolutions / stat.getSeconds() * 4.295;
    else
        return 0;
}

double HistoryStatCalc::getErrToSol() const {
    uint64_t total = stat.solutions + stat.errors;

    if (total > 0)
        return (double)100 * stat.errors / total;
    else
        return 0;
}

double HistoryStatCalc::getChipGHs() const {
    if (stat.intervalMs > 0 && stat.chips > 0)
        return (double)stat.solutions / stat.chips / stat.getSeconds() * 4.295;
    else
        return 0;
}
