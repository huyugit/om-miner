#include "StatCommon.h"

#include "app/Application.h"
#include "config/Config.h"
#include "env/EnvManager.h"
#include "stats/MasterStat.h"



SlaveIterator::SlaveIterator()
    : slaveId(-1)
{}

bool SlaveIterator::next()
{
    const uint32_t slaveMask = Application::config()->slaveMask;

    slaveId++;
    while (slaveId < EnvManager::slaveCount)
    {
        if (slaveMask & (1 << slaveId))
        {
            return true;
        }
        slaveId++;
    }
    return false;
}

SlaveStat& SlaveIterator::get() const
{
    return g_masterStat.getSlave(slaveId);
}



BoardIterator::BoardIterator()
    : boardId(-1)
{}

bool BoardIterator::next()
{
    if (boardId >= 0)
    {
        boardId++;

        if (boardId >= MAX_BOARD_PER_SLAVE)
        {
            boardId = -1;
        }
    }

    if (boardId < 0)
    {
        if (!slaveIt.next())
            return false;

        boardId = 0;
    }

    return true;
}

BoardStat &BoardIterator::get() const
{
    return slaveIt.get().getBoardStat(boardId);
}



PwcIterator::PwcIterator()
    : pwcId(-1)
{}

bool PwcIterator::next()
{
    if (pwcId >= 0)
    {
        pwcId++;

        BoardStat &board = boardIt.get();
        int pwcNum = board.spec.spiNum * board.spec.spiLen;

        if (pwcId >= pwcNum)
        {
            pwcId = -1;
        }
    }

    if (pwcId < 0)
    {
        if (!boardIt.next())
            return false;

        pwcId = 0;
    }

    return true;
}

PwcStat &PwcIterator::get() const
{
    return boardIt.get().getPwcStat(0, pwcId);
}



BtcIterator::BtcIterator()
    : btcId(-1), mask(0)
{}

bool BtcIterator::next()
{
    while (true)
    {
        if (btcId < 0)
        {
            if (!pwcIt.next())
                return false;

            mask = pwcIt.get().btcMask();
            btcId = 0;
        }
        else {
            btcId++;
        }

        if (btcId >= MAX_BTC16_PER_PWC)
        {
            btcId = -1;
            continue;
        }

        if (mask & (1 << btcId))
             break;
    }

    return true;
}

ChipStat &BtcIterator::get() const
{
    return pwcIt.get().getChipStat(btcId);
}
