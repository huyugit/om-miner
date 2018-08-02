#include "BoardStat.h"

#include "common.h"
#include "app/Application.h"
#include "config/Config.h"
#include "env/EnvManager.h"
#include "base/BaseUtil.h"
#include "stats/MasterStat.h"
#include "stats/SlaveStat.h"
#include "stats/hist/HistoryStat.h"
#include "stats/hist/HistoryStatCalc.h"


PwcStat BoardStat::dummyPwcChip;


BoardStat::BoardStat()
    : hasData(false)
{}

void BoardStat::init(int boardId, int boardNum)
{
    this->boardId  = boardId;
    this->boardNum = boardNum;

    memset(&info, 0, sizeof(info));
    memset(&spec, 0, sizeof(spec));

    boardCurrent    = 0;
    boardPower      = 0;

    boardTotal.clear();

    for (int spiId = 0; spiId < MAX_SPI_PER_BOARD; spiId++)
    {
        spiTotal[spiId].clear();
    }

    for (int spiId = 0; spiId < MAX_SPI_PER_BOARD; spiId++)
    {
        for (int chipId = 0; chipId < MAX_PWC_PER_SPI; chipId++)
        {
            pwcChips[spiId][chipId].init(this, spiId, chipId);
        }
    }

    dummyPwcChip.init(&SlaveStat::dummyBoard, 0, 0);
}

PwcStat &BoardStat::getPwcStat(int spiId, int spiSeq)
{
    if (spiId < MAX_SPI_PER_BOARD &&
        spiSeq < MAX_PWC_PER_SPI)
    {
        return pwcChips[spiId][spiSeq];
    }
    else {
        printf("ERROR: getPwcStat(%d/%d): out of range!\n", spiId, spiSeq);
        return dummyPwcChip;
    }
}

void BoardStat::updateData(const BoardData &boardData)
{
    hasData = true;

    if (spec.isMissingCurrent())
    {
        if (spec.isTypeA())
        {
            BoardStat* boardB = g_masterStat.findBoardB(boardNum);
            if (boardB) {
                // copy currents: new boardData.info => boardB->info
                memcpy(boardB->info.currents, boardData.info.currents, sizeof(info.currents));
                boardB->onNewCurrents();
            }
        }
        if (spec.isTypeB())
        {
            // hack: override currents with old info
            memcpy((void*)boardData.info.currents, info.currents, sizeof(info.currents));
        }
    }

    info = boardData.info;
    spec = boardData.spec;

    onNewCurrents();
}

void BoardStat::onNewCurrents()
{
    uint32_t sum = 0;
    for (int i = 0; i < spec.pwrNum; i++)
    {
        sum += info.currents[i];
    }

    boardCurrent = sum;
    boardPower = (double)info.voltage/1000.0 * sum/1000.0;
}

void BoardStat::aggregate(TotalStat &total)
{
    if (!isFound()) return;

    boardTotal.clear();
    boardTotal.boards = 1;

    for (int spiId = 0; spiId < spiNum(); spiId++)
    {
        spiTotal[spiId].clear();

        for (int chipId = 0; chipId < spiLen(); chipId++)
        {
            pwcChips[spiId][chipId].aggregate(spiTotal[spiId]);
        }

        boardTotal.aggregate(spiTotal[spiId]);

        if (spiTotal[spiId].solutions.getDiff() <= 0)
            boardTotal.numBrokenSpi++;
    }

    boardTotal.current = boardCurrent;

    boardTotal.currentA = 0;
    if (spec.isTypeA())
    {
        boardTotal.currentA = boardTotal.current;
    }

    boardTotal.power = boardPower;

    total.aggregate(boardTotal);
}

void BoardStat::saveStat()
{
    if (!isFound()) return;

    for (int spiId = 0; spiId < spiNum(); spiId++)
        for (int chipId = 0; chipId < spiLen(); chipId++)
            pwcChips[spiId][chipId].saveStat();

    boardTotal.save();

    for (int spiId = 0; spiId < spiNum(); spiId++)
    {
        spiTotal[spiId].save();
    }
}

void BoardStat::printStat(Writer &wr, PrintStatOpt &printOpt)
{
    TablePrinter &tp = printOpt.tp;
    if (!isFound() && !Application::configRW().logEmptyBoards) return;

    if (printOpt.statLevel == STAT_LEVEL_BOARD_SYSTEM)
    {
        if (printOpt.printHeader)
        {
            printOpt.printHeader = false;
            wr.printf("*** BOARDS SYS INFO (%d boards, %d chips):\n",
                      g_masterStat.total.boards, g_masterStat.total.chips);

            tp.writeCell("BRD");
            tp.writeCell("FOUND");
            tp.writeCell("REV");
            tp.writeCell("SPI");
            tp.writeCell("PWR");
            tp.writeCell("CHIPS");
            tp.writeCell("OhN");
            tp.writeCell("OhT");
            tp.writeCell("loI");
            tp.writeCell("T, C");
            tp.writeCell("TA");
            tp.writeCell("RevADC");
            tp.writeCell("OCP");
            tp.writeCell("HE");
            tp.writeCell("U, mV");
            tp.writeCell("I, mA");
            tp.writeCell("P, Wt");
            tp.newLine();
        }

        StringBuffer<128> sbTmp;
        for (int i = 0; i < spec.getNumTmp(); i++)
        {
            sbTmp.printf(" %2d", info.boardTemperature[i]);
        }

        StringBuffer<128> sbTmpAlert;
        for (int i = 0; i < spec.getNumTmp(); i++)
        {
            sbTmpAlert.printf(" %2u(%2u)", info.taInfo[i].alertHi, info.taInfo[i].numWrite);
        }

        StringBuffer<128> sb;
        for (int i = 0; i < spec.pwrNum; i++)
        {
            sb.printf(" I%d:%5d", i, info.currents[i]);
        }

        tp.writeCell("%d",          boardNum);
        tp.writeCell("%d",          info.boardFound);
        tp.writeCell("BREV:%d",     spec.revisionId);
        tp.writeCell("%dx%2d",      spec.spiNum, spec.spiLen);
        tp.writeCell("%dx%dx%d",    spec.pwrNum, spec.pwrLen, spec.btcNum);
        tp.writeCell("%d",          boardTotal.chips);
        tp.writeCell("%d",          info.ohNum);
        tp.writeCell("%d",          info.ohTime / 1000);
        tp.writeCell("%d",          info.lowCurrRst);
        tp.writeCell("%s",          sbTmp.cdata());
        tp.writeCell("%s",          sbTmpAlert.cdata());
        tp.writeCell("%d",          info.revAdc);
        tp.writeCell("0x%02x",      info.overCurrentProtection);
        tp.writeCell("%u (%u)",     info.heaterErr, info.heaterErrNum);
        tp.writeCell("%d",          info.voltage);
        tp.writeCell("%s",          sb.cdata());
        tp.writeCell("%d",          boardPower);
        tp.newLine();
    }

    else if (printOpt.statLevel == STAT_LEVEL_BOARD)
    {
        HistoryStatCalc calc(boardTotal, printOpt.useTotal);

        if (printOpt.printHeader)
        {
            printOpt.printHeader = false;

            wr.printf("*** BOARDS STAT %s (%u secs, %d boards, %d chips):\n",
                      printOpt.useTotal ? "TOTAL" : "DELTA",
                      calc.getSeconds(), g_masterStat.total.boards, g_masterStat.total.chips);

            tp.writeCell("BRD");
            tp.writeCell("SOL");
            tp.writeCell("ERR");
            tp.writeCell("bySol");
            tp.writeCell("E/S");
            tp.writeCell("JOBS");
            tp.writeCell("CR");
            tp.writeCell("W/GHs");
            tp.writeCell("EPWC");
            tp.newLine();
        }

        tp.writeCell("%d", boardNum);
        tp.writeCell("%llu", calc.getSol());
        tp.writeCell("%d", calc.getErr());
        tp.writeCell("%.0f GH/s", calc.getGHs());
        tp.writeCell("%.0f%%", calc.getErrToSol());
        tp.writeCell("%d", boardTotal.jobsDone.get( printOpt.useTotal ));
        tp.writeCell("%d", boardTotal.restarts.get( printOpt.useTotal ));
        tp.writeCell("%.3f", util::safeDiv(boardTotal.power, calc.getGHs()));
        tp.writeCell("%d", boardTotal.numBrokenPwc);
        tp.newLine();
    }

    else if (printOpt.statLevel == STAT_LEVEL_SPI)
    {
        for (int spiId = 0; spiId < spiNum(); spiId++)
        {
            HistoryStatCalc calc(spiTotal[spiId], printOpt.useTotal);
        
            if (printOpt.printHeader)
            {
                printOpt.printHeader = false;

                wr.printf("*** SPI STAT %s (%u secs):\n",
                          printOpt.useTotal ? "TOTAL" : "DELTA",
                          calc.getSeconds());

                tp.writeCell("BRD");
                tp.writeCell("SPI");
                tp.writeCell("LEN");
                tp.writeCell("SOL");
                tp.writeCell("ERR");
                tp.writeCell("bySol");
                tp.writeCell("E/S");
                tp.writeCell("JOBS");
                tp.writeCell("CR");
                tp.newLine();
            }

            tp.writeCell("%d", boardNum);
            tp.writeCell("%d", spiId);
            tp.writeCell("%d", spiLen());

            tp.writeCell("%llu", calc.getSol());
            tp.writeCell("%llu", calc.getErr());
            tp.writeCell("%.0f GH/s", calc.getGHs());
            tp.writeCell("%.0f%%", calc.getErrToSol());
            tp.writeCell("%d", spiTotal[spiId].jobsDone.get());
            tp.writeCell("%d", spiTotal[spiId].restarts.get());
            tp.newLine();
        }
    }

    else
    {
        for (int spiId = 0; spiId < spiNum(); spiId++)
        {
            for (int chipId = 0; chipId < spiLen(); chipId++)
            {
                pwcChips[spiId][chipId].printStat(wr, printOpt);
            }
        }
    }
}
