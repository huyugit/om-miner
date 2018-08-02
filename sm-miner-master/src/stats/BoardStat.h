#ifndef BOARDSTAT_H
#define BOARDSTAT_H

#include "ms-protocol/ms_defines.h"
#include "ms-protocol/ms_data.h"
#include "stats/StatCommon.h"
#include "stats/PwcStat.h"


class BoardStat
{
public:
    BoardStat();
    void init(int boardId, int boardNum);

    inline uint8_t spiNum() const { return spec.spiNum; }
    inline uint8_t spiLen() const { return spec.spiLen; }
    inline uint8_t btcNum() const { return spec.btcNum; }
    inline uint16_t btcMask() const { return spec.btcMask; }

    inline bool isFound() const { return info.boardFound; }
    inline int getNum() const { return boardNum; }

    PwcStat& getPwcStat(int spiId, int spiSeq);

    TotalStat& getBoardTotal() { return boardTotal; }

    void updateData(const BoardData &boardData);
    void onNewCurrents();

    void updateData(const BoardTest &boardTest);

    void aggregate(TotalStat &total);
    void saveStat();

    void printStat(Writer &wr, PrintStatOpt &printOpt);

    bool hasData;

    BoardInfo info;
    BoardSpec spec;
    BoardTest test;

    uint32_t boardCurrent;
    uint32_t boardPower;

private:
    int boardId, boardNum;

    TotalStat boardTotal;
    TotalStat spiTotal[MAX_SPI_PER_BOARD];

    static PwcStat dummyPwcChip;
    PwcStat pwcChips[MAX_SPI_PER_BOARD][MAX_PWC_PER_SPI];
};

#endif // BOARDSTAT_H
