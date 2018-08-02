#ifndef CHIPSTAT_H
#define CHIPSTAT_H

#include "stats/StatCommon.h"
#include "sys/writer/Writer.h"

class ChipStat
{
public:
    ChipStat();
    void init(BoardStat *_parentBoard, int _spiId, int _spiSeq, int _pwcSeq);

    void onStatData(const Btc16StatData &d, bool resetDetected);

    void aggregate(TotalStat &total);
    void saveStat();

    void printStat(Writer &wr, PrintStatOpt &printOpt);

public: // private:
    BoardStat *parentBoard;
    int spiId, spiSeq, pwcSeq;

    bool hasData;
    Btc16StatData statData;

    uint8_t   osc;
    uint16_t  reads;
    StatCounter64 solutions;
    StatCounter64 errors;
    StatCounter64 jobsDone;
    uint16_t lastJobTime;
    StatCounter64 restarts;

    BtcExtStat stat;
    BtcFixesStat fixes;
};

#endif // CHIPSTAT_H
