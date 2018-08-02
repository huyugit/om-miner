#include "TotalStat.h"

#include "stats/SlaveStat.h"
#include "stats/ChipStat.h"


TotalStat::TotalStat()
    : chips(0)
    , boards(0)
    , numBrokenSpi(0)
    , numBrokenPwc(0)
    , psuCurrent(0)
    , psuPower(0)
    , current(0)
    , currentA(0)
    , power(0)
{
}

TotalStat::TotalStat(const ChipStat &chip)
    : chips(1)
    , boards(0)
    , numBrokenSpi(0)
    , numBrokenPwc(0)
    , solutions(chip.solutions)
    , errors(chip.errors)
    , jobsDone(chip.jobsDone)
    , restarts(chip.restarts)
    , current(0)
    , currentA(0)
    , power(0)
{
}

void TotalStat::clear()
{
    *this = TotalStat();
}

void TotalStat::aggregate(const TotalStat &other)
{
    chips += other.chips;
    boards += other.boards;
    numBrokenSpi += other.numBrokenSpi;
    numBrokenPwc += other.numBrokenPwc;
    solutions += other.solutions;
    errors += other.errors;
    jobsDone += other.jobsDone;
    restarts += other.restarts;

    pwcSolByDiff += other.pwcSolByDiff;
    pwcSharesSent += other.pwcSharesSent;
    pwcSharesDropped += other.pwcSharesDropped;

    psuCurrent += other.psuCurrent;
    psuPower += other.psuPower;

    current += other.current;
    currentA += other.currentA;
    power += other.power;
}

void TotalStat::aggregate(const ChipStat &chip)
{
    chips += 1;
    solutions += chip.solutions;
    errors += chip.errors;
    jobsDone += chip.jobsDone;
    restarts += chip.restarts;
}

void TotalStat::save()
{
    solutions.save();
    errors.save();
    jobsDone.save();
    restarts.save();
    pwcSolByDiff.save();
    pwcSharesSent.save();
    pwcSharesDropped.save();
}
