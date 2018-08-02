#ifndef PWCSTAT_H
#define PWCSTAT_H

#include "ms-protocol/ms_data.h"
#include "stats/StatCommon.h"
#include "stats/ChipStat.h"
#include "base/PollTimer.h"
#include "sys/writer/Writer.h"


class PwcStat;

class PwcBtcIterator
{
public:
    PwcBtcIterator(PwcStat &_pwc);

    ChipStat& getChipStat() const;
    bool next();

private:
    PwcStat &pwc;
    int pwcSeq;
    uint32_t leftMask;
};


class PwcStat
{
public:
    PwcStat();
    void init(BoardStat *_parentBoard, int _spiId, int _spiSeq);

    uint8_t btcNum() const;
    uint16_t btcMask() const;

    PwcBtcIterator btcIterator();
    ChipStat &getChipStat(int pwcSeq);

    void onPwcBlock(const PwcBlock &pb);

    void aggregate(TotalStat &total);
    void saveStat();

    void printStat(Writer &wr, PrintStatOpt &printOpt);

public: // private:
    BoardStat *parentBoard;
    int spiId, spiSeq;

    PwcSpiData spiData;
    PwcSharedData pwcSharedData;
    PollTimer pingTimer;

    PwcQuickTestRes quickTest;

    StatCounter64 totalSolByDiff;
    StatCounter64 sharesSent;
    StatCounter64 sharesDropped;

    static ChipStat dummyChip;
    ChipStat chips[MAX_BTC16_PER_PWC];
};

#endif // PWCSTAT_H
