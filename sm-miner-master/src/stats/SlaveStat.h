#ifndef SLAVESTAT_H
#define SLAVESTAT_H

#include "pool/StratumShare.h"
#include "ms-protocol/ms_data.h"
#include "cmn_block.h"

#include "stats/StatCommon.h"
#include "stats/BoardStat.h"
#include "stats/ChipStat.h"


class SlaveStat
{
public:
    SlaveStat();
    void init(int _slaveId);

    BoardStat& getBoardStat(int boardId);
    PwcStat &getPwcStat(int boardId, int spiId, int spiSeq);

    void updateData(const SlaveData &msg);
    void updateData(const SlaveHbtData &msg);

    void aggregate(TotalStat &total);
    void saveStat();

    void printStat(Writer &wr, PrintStatOpt &printOpt);

    BoardConfig& getBoardConfig(int boardId);

    LedStates ledStates;
    void setLedsNormalMinig();
    void setLedsServerTest();
    void setLedsBoardTestDone(int boardId);

    StatCounter32 msSpiTx;
    StatCounter32 msSpiRxOk;
    StatCounter32 msSpiRxError;

    bool hasData;

    SlaveCmnData cmnInfo;
    SlaveTestInfo testInfo;
    uint32_t    totalTime;

    NonceContainer noncesContainer;

    PsuMgrInfo  psuMgrInfo;
    uint8_t     psuNum;
    PsuInfo     psuInfo[MAX_PSU_PER_SLAVE];
    PsuSpec     psuSpec[MAX_PSU_PER_SLAVE];

    static BoardStat dummyBoard;

    int getPingTime();

private:
    int slaveId;

    BoardStat boards[MAX_BOARD_PER_SLAVE];
};

#endif // SLAVESTAT_H
