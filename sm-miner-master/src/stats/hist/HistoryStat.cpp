#include "HistoryStat.h"

#include "app/Application.h"
#include "stats/TotalStat.h"
#include "events/EventManager.h"


HistoryStat::HistoryStat()
    : intervalName("n/a")
    , intervalExpected(0)
    // --
    , intervalMs(0)
    // -- total stat
    , solutions(0)
    , errors(0)
    , jobsDone(0)
    , restarts(0)
    // --
    , pwcSolByDiff(0)
    , pwcSharesSent(0)
    , pwcSharesDropped(0)
    // -- static stat
    , staticSet(0)
    , chips(0)
    , boards(0)
    // -- pool stat
    , poolTotalTime(0)
    , poolInService(0)
    // --
    , poolReceivedJobs(0)
    , poolReceivedJobsWithClean(0)
    // --
    , poolSentShares(0)
    , poolAcceptedShares(0)
    , poolRejectedShares(0)
    , poolAcceptedSolutions(0)
    // -- event stat
    , poolSubscribeError(0)
    , poolDiffChanges(0)
    , poolReconnections(0)
    , poolReconnectionsOnError(0)
    , poolDefaultJobShares(0)
    , poolStaleJobShares(0)
    , poolDuplicateShares(0)
    , poolLowDifficultyShares(0)
{
}

void HistoryStat::aggregate(const HistoryStat &other)
{
    intervalMs += other.intervalMs;

    // total stat
    solutions += other.solutions;
    errors += other.errors;
    jobsDone += other.jobsDone;
    restarts += other.restarts;

    pwcSolByDiff += other.pwcSolByDiff;
    pwcSharesSent += other.pwcSharesSent;
    pwcSharesDropped += other.pwcSharesDropped;

    // static stat
    if (!staticSet && other.staticSet)
    {
        // WARNING: we cannot simply aggregate those data, so we
        // assume that aggregation is done from past to now, so that
        // other is more important!

        chips = other.chips;
        boards = other.boards;
    }

    // pool stat
    poolTotalTime += other.poolTotalTime;
    poolInService += other.poolInService;
    
    poolReceivedJobs += other.poolReceivedJobs;
    poolReceivedJobsWithClean += other.poolReceivedJobsWithClean;
    
    poolSentShares += other.poolSentShares;
    poolAcceptedShares += other.poolAcceptedShares;
    poolRejectedShares += other.poolRejectedShares;
    poolAcceptedSolutions += other.poolAcceptedSolutions;

    // event stat
    poolSubscribeError += other.poolSubscribeError;
    poolDiffChanges += other.poolDiffChanges;
    poolReconnections += other.poolReconnections;
    poolReconnectionsOnError += other.poolReconnectionsOnError;
    poolDefaultJobShares += other.poolDefaultJobShares;
    poolStaleJobShares += other.poolStaleJobShares;
    poolDuplicateShares += other.poolDuplicateShares;
    poolLowDifficultyShares += other.poolLowDifficultyShares;
}

void HistoryStat::loadTotalStat(const TotalStat &total, bool useTotal)
{
    if (useTotal)
    {
        solutions = total.solutions.get();
        errors = total.errors.get();
        jobsDone = total.jobsDone.get();
        restarts = total.restarts.get();

        pwcSolByDiff = total.pwcSolByDiff.get();
        pwcSharesSent = total.pwcSharesSent.get();
        pwcSharesDropped = total.pwcSharesDropped.get();
    }
    else
    {
        solutions = total.solutions.getDiff();
        errors = total.errors.getDiff();
        jobsDone = total.jobsDone.getDiff();
        restarts = total.restarts.getDiff();

        pwcSolByDiff = total.pwcSolByDiff.getDiff();
        pwcSharesSent = total.pwcSharesSent.getDiff();
        pwcSharesDropped = total.pwcSharesDropped.getDiff();
    }

    staticSet = true;
    chips = total.chips;
    boards = total.boards;
}

void HistoryStat::loadEventDiffs()
{
    const EventManager& eventManager = Application::events();

    poolSubscribeError          = eventManager.getCounter(EventType::SUBSCRIBE_ERROR).getDiff();
    poolDiffChanges             = eventManager.getCounter(EventType::DIFF_CHANGE).getDiff();
    poolReconnections           = eventManager.getCounter(EventType::RECONNECTION).getDiff();
    poolReconnectionsOnError    = eventManager.getCounter(EventType::RECONNECTION_ON_ERROR).getDiff();
    poolDefaultJobShares        = eventManager.getCounter(EventType::DEFAULT_JOB_SHARE).getDiff();
    poolStaleJobShares          = eventManager.getCounter(EventType::STALE_JOB_SHARE).getDiff();
    poolDuplicateShares         = eventManager.getCounter(EventType::DUPLICATE_SHARE).getDiff();
    poolLowDifficultyShares     = eventManager.getCounter(EventType::LOW_DIFF_SHARE).getDiff();
}

double HistoryStat::getShareLoss() const
{
    if (poolSentShares == 0)
        return 0.0;
    else if (poolSentShares <= (uint64_t)poolAcceptedShares + poolRejectedShares)
        return 0.0;
    else
        return 100.0 * (poolSentShares - poolAcceptedShares - poolRejectedShares) / poolSentShares;
}

unsigned int HistoryStat::getSeconds() const
{
    return static_cast<unsigned int>(intervalMs / 1000);
}
