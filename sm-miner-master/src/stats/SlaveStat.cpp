#include "SlaveStat.h"
#include "common.h"
#include "app/Application.h"
#include "config/Config.h"
#include "stats/MasterStat.h"

BoardStat SlaveStat::dummyBoard;

SlaveStat::SlaveStat()
{
}

void SlaveStat::init(int _slaveId)
{
    slaveId = _slaveId;

    hasData = false;

    msSpiTx.clear();
    msSpiRxOk.clear();
    msSpiRxError.clear();

    noncesContainer.clear();

    psuNum = 0;
    memset(psuInfo, 0, sizeof(psuInfo));
    memset(psuSpec, 0, sizeof(psuSpec));

    setLedsNormalMinig();

    for (int boardId = 0; boardId < MAX_BOARD_PER_SLAVE; boardId++)
    {
        int boardNum = slaveId * MAX_BOARD_PER_SLAVE + boardId;
        boards[boardId].init(boardId, boardNum);
    }

    dummyBoard.init(0, 0);
}


BoardStat& SlaveStat::getBoardStat(int boardId)
{
    if (boardId < MAX_BOARD_PER_SLAVE)
    {
        return boards[boardId];
    }
    else {
        printf("ERROR: getBoardStat(%d): out of range!\n", boardId);
        return dummyBoard;
    }
}

PwcStat &SlaveStat::getPwcStat(int boardId, int spiId, int spiSeq)
{
    return getBoardStat(boardId).getPwcStat(spiId, spiSeq);
}

void SlaveStat::updateData(const SlaveData &msg)
{
    cmnInfo = msg.cmnInfo;

    for (int i = 0; i < MAX_BOARD_PER_SLAVE; i++)
    {
        getBoardStat(i).updateData( msg.boardData[i] );
    }
}

void SlaveStat::updateData(const SlaveHbtData &msg)
{
    testInfo = msg.testInfo;

    for (int i = 0; i < MAX_BOARD_PER_SLAVE; i++)
    {
        getBoardStat(i).test = msg.boardTest[i];
    }
}

void SlaveStat::aggregate(TotalStat &total)
{
    double unitI = 0;
    double unitP = 0;

    for (size_t i = 0; i < MAX_PSU_PER_SLAVE; i++)
    {
        unitI += psuInfo[i].getIOut();
        unitP += psuInfo[i].getPOut();
    }

    //double unitV = util::safeDiv(unitP, unitI);

    TotalStat slaveTotal;
    slaveTotal.psuCurrent = unitI;
    slaveTotal.psuPower   = unitP;

    for (int boardId = 0; boardId < MAX_BOARD_PER_SLAVE; boardId++)
        boards[boardId].aggregate(slaveTotal);

    total.aggregate(slaveTotal);
}

void SlaveStat::saveStat()
{
    for (int boardId = 0; boardId < MAX_BOARD_PER_SLAVE; boardId++)
        boards[boardId].saveStat();

    msSpiTx.save();
    msSpiRxOk.save();
    msSpiRxError.save();
}

BoardConfig& SlaveStat::getBoardConfig(int boardId)
{
    int slotId = slaveId * MAX_BOARD_PER_SLAVE + boardId;
    return Application::configRW().boardConfig[slotId];
}

void SlaveStat::setLedsNormalMinig()
{
    ledStates.greenLedState = LED_BLINK_SLOW;
    ledStates.redLedState = LED_OFF;

    for (int i = 0; i < MAX_BOARD_PER_SLAVE; i++)
    {
        ledStates.boardLedState[i] = LED_BLINK_SLOW;
        ledStates.boardLedColor[i] = BOARD_LED_COLOR_GREEN;
    }
}

void SlaveStat::setLedsServerTest()
{
    ledStates.greenLedState = LED_BLINK_QUICK;
    ledStates.redLedState = LED_BLINK_QUICK;

    for (int i = 0; i < MAX_BOARD_PER_SLAVE; i++)
    {
        ledStates.boardLedState[i] = LED_BLINK_QUICK;
        ledStates.boardLedColor[i] = BOARD_LED_COLOR_RED;
    }
}

void SlaveStat::setLedsBoardTestDone(int boardId)
{
    assert(boardId < MAX_BOARD_PER_SLAVE);

    ledStates.boardLedState[boardId] = LED_ON;
    ledStates.boardLedColor[boardId] = BOARD_LED_COLOR_GREEN;
}

int SlaveStat::getPingTime()
{
    return (cmnInfo.loopbackTime > 0 ? g_upTimer.elapsedMs() - cmnInfo.loopbackTime : 666666);
}

void SlaveStat::printStat(Writer &wr, PrintStatOpt &printOpt)
{
    for (int boardId = 0; boardId < MAX_BOARD_PER_SLAVE; boardId++)
    {
        boards[boardId].printStat(wr, printOpt);
    }
}

