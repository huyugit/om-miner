#ifndef STATCOMMON_H
#define STATCOMMON_H

#include <stdint.h>
#include "cmn_block.h"
#include "base/TablePrinter.h"
#include "stats/StatCounter.h"
#include "stats/TotalStat.h"


enum StatLevel
{
    STAT_LEVEL_CHIP,
    STAT_LEVEL_PWC,
    STAT_LEVEL_SPI,
    STAT_LEVEL_BOARD_SYSTEM,
    STAT_LEVEL_BOARD,
};

#define USE_TOTAL true
#define USE_DELTA false

struct PrintStatOpt
{
    StatLevel statLevel;
    bool printHeader;
    bool useTotal;

    TablePrinter tp;

    PrintStatOpt(StatLevel _statLevel, bool _useTotal)
        : statLevel(_statLevel), printHeader(true), useTotal(_useTotal)
    {}
};


// Forward declarations
class SlaveStat;
class BoardStat;
class PwcStat;
class BtcStat;


struct SlaveIterator
{
    SlaveIterator();
    bool next();
    SlaveStat &get() const;

private:
    int slaveId;
};


struct BoardIterator
{
    BoardIterator();
    bool next();
    BoardStat& get() const;

private:
    SlaveIterator slaveIt;
    int boardId;
};


struct PwcIterator
{
    PwcIterator();
    bool next();
    PwcStat& get() const;

private:
    BoardIterator boardIt;
    int pwcId;
};


struct BtcIterator
{
    BtcIterator();
    bool next();
    ChipStat &get() const;

private:
    PwcIterator pwcIt;
    int btcId, mask;
};

#endif // STATCOMMON_H
