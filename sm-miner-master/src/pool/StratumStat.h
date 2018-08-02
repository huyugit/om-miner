#ifndef STRATUM_STAT_H
#define STRATUM_STAT_H
/*
 * Contains StratumStat class declaration.
 */

#include <stdint.h>


// Class holding Pool statistics deltas.
// 
struct StratumStat
{
    uint64_t totalPoolTimeMs;
    uint64_t inServiceTimeMs;

    unsigned int receivedJobs;
    unsigned int receivedJobsWithClean;

    unsigned int sentShares;
    unsigned int acceptedShares;
    unsigned int rejectedShares;
    uint64_t acceptedSolutions;
    
    // Default constructor.
    StratumStat()
        : totalPoolTimeMs(0)
        , inServiceTimeMs(0)
        //
        , receivedJobs(0)
        , receivedJobsWithClean(0)
        //
        , sentShares(0)
        , acceptedShares(0)
        , rejectedShares(0)
        , acceptedSolutions(0)
    {}

    // Constructs the object using the specified property values.
    StratumStat(
        uint64_t totalPoolTimeMs,
        uint64_t inServiceTimeMs,

        unsigned int receivedJobs,
        unsigned int receivedJobsWithClean,

        unsigned int sentShares,
        unsigned int acceptedShares,
        unsigned int rejectedShares,
        uint64_t acceptedSolutions)
        //
        : totalPoolTimeMs(totalPoolTimeMs)
        , inServiceTimeMs(inServiceTimeMs)
        //
        , receivedJobs(receivedJobs)
        , receivedJobsWithClean(receivedJobsWithClean)
        //
        , sentShares(sentShares)
        , acceptedShares(acceptedShares)
        , rejectedShares(rejectedShares)
        , acceptedSolutions(acceptedSolutions)
    {}
};

#endif  // STRATUM_STAT_H
