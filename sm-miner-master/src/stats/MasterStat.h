#ifndef MASTERSTAT_H
#define MASTERSTAT_H

#include "base/PollTimer.h"

#include "stats/StatCommon.h"
#include "stats/SlaveStat.h"
#include "stats/hist/HistoryStatMgr.h"
#include "stats/hist/HistoryStat.h"
#include "stats/FanStat.h"


class MasterStat
{
public:
    MasterStat();
    void init();

    SlaveStat& getSlave(int slaveId);
    BoardStat *findBoardB(int startBoardNum);

    void aggregate();
    void saveStat();

    void printStat(Writer &wr);
    void printStat(Writer &wr, StatLevel statLevel, bool useTotal);

    void exportStat();

    int statIntervalTotal;

    TotalStat total;

public:
    HistoryStatMgr& getHistory() { return historyStatMgr; }

private:
    PollTimer statStartTimer;
    PollTimer statLastTimer;

    static SlaveStat dummySlave;
    SlaveStat slaves[MAX_SLAVE_COUNT];

    HistoryStatMgr historyStatMgr;


public:
    void printBtcStat(Writer &wr);
    void printPwcStat(Writer &wr);
    void printMasterStat(Writer &wr);
    void printPoolStat(Writer &wr);
    void printEventStat(Writer &wr);
    void printSystemStat(Writer &wr);
    void printSlaveMcuStat(Writer &wr);
};

extern MasterStat g_masterStat;
extern PollTimer g_upTimer;

#endif // MASTERSTAT_H
