#include "MasterStat.h"

#include "app/Application.h"
#include "base/DateTimeStr.h"
#include "config/Config.h"
#include "events/EventManager.h"
#include "pool/StratumPool.h"
#include "env/EnvManager.h"
#include "hw/GpioManager.h"
#include "ms-protocol/ms_packet.h"
#include "board_revisions.h"
#include "slave-gate/SlaveGate.h"
#include "stats/hist/HistoryStat.h"
#include "stats/hist/HistoryStatMgr.h"
#include "stats/hist/HistoryStatCalc.h"
#include "sys/NetInfoUtil.h"
#include "base/TablePrinter.h"
#include "version.h"

MasterStat g_masterStat;
PollTimer g_upTimer;

double percent(double x, double y) {
    return y > 0 ? x / y * 100 : 0;
}

SlaveStat MasterStat::dummySlave;


MasterStat::MasterStat()
    : statIntervalTotal(0),
      statStartTimer(),
      statLastTimer()
{
}

void MasterStat::init()
{
    for (int slaveId = 0; slaveId < MAX_SLAVE_COUNT; slaveId++)
        slaves[slaveId].init(slaveId);

    dummySlave.init(0);
}

SlaveStat& MasterStat::getSlave(int slaveId)
{
    if (slaveId < MAX_SLAVE_COUNT)
    {
        return slaves[slaveId];
    }
    else {
        printf("ERROR: getSlave(%d): out of range!\n", slaveId);
        return dummySlave;
    }
}

BoardStat* MasterStat::findBoardB(int startBoardNum)
{
    for (int slaveId = 0; slaveId < MAX_SLAVE_COUNT; slaveId++)
    {
        for (int boardId = 0; boardId < MAX_BOARD_PER_SLAVE; boardId++)
        {
            BoardStat &board = getSlave(slaveId).getBoardStat(boardId);

            if (board.getNum() <= startBoardNum) {
                continue;
            }

            if (!board.info.boardFound) {
                continue;
            }

            if (board.spec.isTypeB()) {
                return &board;
            }

            printf("WARNING: can not find board B starting from board %d (wrong type)\n",
                   startBoardNum);
            return nullptr;
        }
    }

    printf("WARNING: can not find board B starting from board %d (end is reached)\n",
           startBoardNum);
    return nullptr;
}


void MasterStat::aggregate()
{
    // time handling
    statIntervalTotal = static_cast<int>(statStartTimer.elapsedSec());
    
    const uint64_t statIntervalDeltaMs = static_cast<int>(statLastTimer.elapsedMs());
    statLastTimer.start();

    // aggregate all distributed data
    total.clear();
    for (int slaveId = -1; EnvManager::nextSlaveId(slaveId); )
        slaves[slaveId].aggregate(total);

    const StratumStat poolStat = Application::pool().cutOffStat();

    // collect all deltas into one container
    HistoryStat newItem;
    newItem.intervalMs = statIntervalDeltaMs;

    newItem.loadTotalStat(total, false);
    newItem.loadEventDiffs();

    newItem.poolTotalTime = poolStat.totalPoolTimeMs;
    newItem.poolInService = poolStat.inServiceTimeMs;

    newItem.poolReceivedJobs = poolStat.receivedJobs;
    newItem.poolReceivedJobsWithClean = poolStat.receivedJobsWithClean;

    newItem.poolSentShares = poolStat.sentShares;
    newItem.poolAcceptedShares = poolStat.acceptedShares;
    newItem.poolRejectedShares = poolStat.rejectedShares;
    newItem.poolAcceptedSolutions = poolStat.acceptedSolutions;

    // add history element
    historyStatMgr.pushItem(newItem);
}

void MasterStat::saveStat()
{
    for (int slaveId = -1; EnvManager::nextSlaveId(slaveId); )
        slaves[slaveId].saveStat();

    Application::events().saveStat();
}

void MasterStat::printStat(Writer &wr)
{
    wr.printf("------------------------------------------------------------\n");
    wr.printf("-----                  STATISTIC                       -----\n");
    wr.printf("------------------------------------------------------------\n");

    if (Application::config()->logChipStat)
    {
        printBtcStat(wr);
        printPwcStat(wr);
    }

    //printStat(wr, STAT_LEVEL_SPI, USE_DELTA);

    printStat(wr, STAT_LEVEL_BOARD_SYSTEM, USE_TOTAL);

    printStat(wr, STAT_LEVEL_BOARD, USE_DELTA);
    printStat(wr, STAT_LEVEL_BOARD, USE_TOTAL);

    printMasterStat(wr);

    printPoolStat(wr);

    printEventStat(wr);

    printSystemStat(wr);

    printSlaveMcuStat(wr);
}

void MasterStat::printStat(Writer &wr, StatLevel statLevel, bool useTotal)
{
    PrintStatOpt printOpt(statLevel, useTotal);

    for (int slaveId = -1; EnvManager::nextSlaveId(slaveId); )
    {
        slaves[slaveId].printStat(wr, printOpt);
    }

    printOpt.tp.printTable(wr);
}

void MasterStat::printBtcStat(Writer &wr)
{
    printStat(wr, STAT_LEVEL_CHIP, USE_TOTAL);
}

void MasterStat::printPwcStat(Writer &wr)
{
    printStat(wr, STAT_LEVEL_PWC,  USE_TOTAL);
}

void MasterStat::printMasterStat(Writer &wr)
{
    wr.printf("*** MASTER STATS PER INTERVAL:\n");

    TablePrinter tp;
    tp.writeCell("INTERVAL");
    tp.writeCell("sec");

    tp.writeCell("bySol");
    tp.writeCell("byDiff");
    tp.writeCell("byPool");
    tp.writeCell("byJobs");

    tp.writeCell("CHIP GHs");
    tp.writeCell("W/GHs");
    tp.writeCell("SOL");
    tp.writeCell("ERR");
    tp.writeCell("ERR(%%)");
    tp.writeCell("CR");
    tp.newLine();

    for (size_t period = 0; period < HistoryStatMgr::MAX_PERIODS; period++)
    {
        HistoryStatCalc calc(period);
        tp.writeCell("%s",      calc.getStat().intervalName);
        tp.writeCell("%us",     calc.getStat().getSeconds());

        tp.writeCell("%.1f",    calc.getGHs());
        tp.writeCell("%.1f",    calc.getGHsByDiff());
        tp.writeCell("%.1f",    calc.getGHsByPool());
        tp.writeCell("%.1f",    calc.getGHsByJobs());

        tp.writeCell("%.2f",    calc.getChipGHs());
        tp.writeCell("%.3f",    util::safeDiv(total.psuPower, calc.getGHs()));
        tp.writeCell("%llu",    calc.getSol());
        tp.writeCell("%llu",    calc.getErr());
        tp.writeCell("%.1f%%",  calc.getErrToSol());
        tp.writeCell("%llu",    calc.getChipRestarts());
        tp.newLine();
    }

    tp.printTable(wr);


    wr.printf("*** MASTER STATS:\n");

    wr.printf("Date: %s, UpTime: %d secs, mbHwVer: 0x%x, osc: %u\n",
              DateTimeStr().str, statIntervalTotal,
              g_gpioManager.mbHwVer, Application::config()->pwcConfig.osc);

    wr.printf("Found boards:   %d\n", total.boards);
    wr.printf("Broken SPI:     %d\n", total.numBrokenSpi);
    wr.printf("\n");
}

void MasterStat::printPoolStat(Writer &wr)
{
    StratumPool& pool = Application::pool();

    wr.printf("*** POOL STATS:\n");
    pool.logPoolStat(wr);

    TablePrinter tp;
    tp.writeCell("INTERVAL");
    tp.writeCell("sec");
    tp.writeCell("JOBS");
    tp.writeCell("clean");
    tp.writeCell("SHARES");
    tp.writeCell("ok");
    tp.writeCell("err");
    tp.writeCell("POOL sol");
    tp.writeCell("loss");
    tp.writeCell("INSERVICE");
    tp.writeCell("%%");
    tp.newLine();

    for (size_t period = 0; period < HistoryStatMgr::MAX_PERIODS; period++)
    {
        const HistoryStat& stat = getHistory().getByPeriod(period);

        tp.writeCell("%s", stat.intervalName);
        tp.writeCell("%us", stat.getSeconds());

        tp.writeCell("%u", stat.poolReceivedJobs);
        tp.writeCell("%u", stat.poolReceivedJobsWithClean);

        tp.writeCell("%u", stat.poolSentShares);
        tp.writeCell("%u", stat.poolAcceptedShares);
        tp.writeCell("%u", stat.poolRejectedShares);
        tp.writeCell("%llu", stat.poolAcceptedSolutions);

        tp.writeCell("%.1f%%",stat.getShareLoss());
        tp.writeCell("%.0fs", stat.poolInService / 1000.0);
        tp.writeCell("%.1f%%", percent(stat.poolInService, stat.poolTotalTime));
        tp.newLine();
    }

    tp.printTable(wr);
}

void MasterStat::printEventStat(Writer &wr)
{
    wr.printf("*** EVENT STATS:\n");
    wr.printf("Legend: SE - subbsribe error (initialising issue)\n");
    wr.printf("        DIFF - diff changes, REC - reconnects, RECE - reconnects on error\n");
    wr.printf("        SHARES - sent to pool, PSS - pwc shares sent, PSD - pwc shares dropped\n");
    wr.printf("        DJS - default job shares, SJS - stale job shares, DUP - duplicates, LDS - low diff shares\n");

    TablePrinter tp;
    tp.writeCell("INTERVAL");
    tp.writeCell("sec");

    tp.writeCell("SE");
    tp.writeCell("DIFF");
    tp.writeCell("REC");
    tp.writeCell("RECE");

    tp.writeCell("SHARES");
    tp.writeCell("PSS");
    tp.writeCell("PSD");

    tp.writeCell("DJS");
    tp.writeCell("SJS");
    tp.writeCell("DUP");
    tp.writeCell("LDS");
    tp.newLine();

    for (size_t period = 0; period < HistoryStatMgr::MAX_PERIODS; period++)
    {
        const HistoryStat& stat = getHistory().getByPeriod(period);

        tp.writeCell("%s", stat.intervalName);
        tp.writeCell("%us", stat.getSeconds());

        tp.writeCell("%u", stat.poolSubscribeError);
        tp.writeCell("%u", stat.poolDiffChanges);
        tp.writeCell("%u", stat.poolReconnections);
        tp.writeCell("%u", stat.poolReconnectionsOnError);

        tp.writeCell("%u", stat.poolSentShares);
        tp.writeCell("%u", stat.pwcSharesSent);
        tp.writeCell("%u", stat.pwcSharesDropped);

        tp.writeCell("%u", stat.poolDefaultJobShares);
        tp.writeCell("%u", stat.poolStaleJobShares);
        tp.writeCell("%u", stat.poolDuplicateShares);
        tp.writeCell("%u", stat.poolLowDifficultyShares);

        tp.newLine();
    }

    tp.printTable(wr);
}

void MasterStat::printSystemStat(Writer &wr)
{
    wr.printf("*** SYSTEM STATS:\n");
    {
        TablePrinter tp;
        tp.writeCell("PSU");
        tp.writeCell("Serial");

        tp.writeCell("Part");
        tp.writeCell("Version");
        tp.writeCell("Date");
        tp.writeCell("Desc");
        tp.newLine();

        SlaveStat& slave = g_masterStat.getSlave(0);
        for (size_t i = 0; i < MAX_PSU_PER_SLAVE; i++)
        {
            PsuSpec &ps = slave.psuSpec[i];

            tp.writeCell("%u",  ps.id);
            tp.writeCell("%s",  ps.serial);

            tp.writeCell("%s",  ps.prodPart);
            tp.writeCell("%s",  ps.prodVer);
            tp.writeCell("%04u-%02u-%02u", ps.prodYear, ps.prodMonth, ps.prodDay);
            tp.writeCell("%s",  ps.prodDesc);
            tp.newLine();
        }
        tp.printTable(wr);
    }

    {
        TablePrinter tp;
        tp.writeCell("PSU");
        tp.writeCell("Serial");

        tp.writeCell("State");
        tp.writeCell("Time");
        tp.writeCell("Set,V");

        tp.writeCell("U,V");
        tp.writeCell("I,A");
        tp.writeCell("P,Wt");
        tp.writeCell("Uin,V");

        tp.writeCell("Tin");
        tp.writeCell("Tout");

		//chenbo add begin 20180309
		tp.writeCell("RatedI,A");
		tp.writeCell("RatedP,W");
		//chenbo add end

        tp.writeCell("LED");
        tp.writeCell("UpTime");
        tp.writeCell("FSR");
        tp.writeCell("FS");
        tp.writeCell("Status");
        tp.writeCell("Rx");
        tp.writeCell("Alarm");
        tp.newLine();

        SlaveStat& slave = g_masterStat.getSlave(0);
        for (size_t i = 0; i < MAX_PSU_PER_SLAVE; i++)
        {
            PsuMgrInfo &mi = slave.psuMgrInfo;
            PsuInfo &pi = slave.psuInfo[i];

            tp.writeCell("%u",   pi.id);
            tp.writeCell("%s",   pi.serial);

            tp.writeCell("%s",   powerStateToStr(mi.getState()));
            tp.writeCell("%u",   mi.stateSec / 1000);
            tp.writeCell("%.1f", mi.getSetVoltage());

            tp.writeCell("%.1f", pi.getVOut());
            tp.writeCell("%.1f", pi.getIOut());
            tp.writeCell("%.1f", pi.getPOut());
            tp.writeCell("%.1f", pi.getVIn());
            tp.writeCell("%d",    pi.tempIn);
            tp.writeCell("%d",    pi.tempOut);

			//chenbo add begin 20180329
			tp.writeCell("%d",    pi.ratedCurrent);
			if(pi.ratedCurrent > 60)
			{
				tp.writeCell("4000W");
			}
			else
			{
				tp.writeCell("3000W");
			}
			//chenbo add end

            tp.writeCell("%s",    EltekLedsToStr(pi.greenLed, pi.yellowLed, pi.redLed).str);
            tp.writeCell("%u",    pi.upTime);
            tp.writeCell("%u",    pi.fanSpeedRef);
            tp.writeCell("%u",    pi.fanSpeed);
            tp.writeCell("%s",    eltekConditionToStr(pi.condition));
            tp.writeCell("%u",    pi.numStatus);
            tp.writeCell("%s",    EltekAlarmSetToStr(pi.majorAlarm, pi.minorAlarm).str);
            tp.newLine();
        }
        tp.printTable(wr);
    }

    {
        wr.printf("FAN INFO:\n");

        FanConfig &fanConfig = Application::configRW().slaveConfig.fanConfig;
        SlaveMbInfo &mbInfo = g_masterStat.getSlave(0).cmnInfo.mbInfo;

        TablePrinter tp;
        tp.writeCell("SetU");
        tp.writeCell("|");
        tp.writeCell("GetU");
        tp.writeCell("GetI");
        tp.newLine();

        tp.writeCell("%.3fV",   fanConfig.getFanVoltage());
        tp.writeCell("|");
        tp.writeCell("%.3fV",   mbInfo.fan.getFanU());
        tp.writeCell("%.3fA",   mbInfo.fan.getFanI());
        tp.newLine();

        tp.printTable(wr);
    }

    {
        wr.printf("POWER INFO:\n");
        wr.printf("Set Voltage: %.1f V\n", Application::configRW().psuConfig.getVoltage());

        TotalStat total = g_masterStat.total;
        HistoryStatCalc calc(HistoryStatMgr::PERIOD_TOTAL);

        TablePrinter tp;
        tp.writeCell("");
        tp.writeCell("U, V");
        tp.writeCell("I, A");
        tp.writeCell("P, kW");
        tp.writeCell("W/GHs");
        tp.newLine();

        tp.writeCell("PSU");
        tp.writeCell("%.1f", util::safeDiv(total.psuPower, total.psuCurrent));
        tp.writeCell("%.1f", total.psuCurrent);
        tp.writeCell("%.2f", total.psuPower / 1000);
        tp.writeCell("%.3f", util::safeDiv(total.psuPower, calc.getGHs()));
        tp.newLine();

        tp.writeCell("Board");
        tp.writeCell("%.1f", util::safeDiv(total.power, total.currentA / 1000.0));
        tp.writeCell("%.1f", total.currentA / 1000.0);
        tp.writeCell("%.2f", total.power / 1000.0);
        tp.writeCell("%.3f", util::safeDiv(total.power, calc.getGHs()));
        tp.newLine();

        tp.printTable(wr);
    }

    g_envManager.printEnvStats(wr);
    wr.printf("\n");
}

void MasterStat::printSlaveMcuStat(Writer &wr)
{
    wr.printf("*** MASTER-SLAVE SPI BUS STATS:\n");

    TablePrinter tp;
    tp.writeCell("SLAVE");
    tp.writeCell("UID");
    tp.writeCell("VER");
    tp.writeCell("TIME");
    tp.writeCell("PING");

    tp.writeCell("  M=>S");
    tp.writeCell("rx");
    tp.writeCell("err");
    tp.writeCell("%%");

    tp.writeCell("  S=>M");
    tp.writeCell("rx");
    tp.writeCell("err");
    tp.writeCell("%%");

    tp.writeCell("SS");
    tp.writeCell("SD");
    tp.newLine();

    for (int slaveId = -1; EnvManager::nextSlaveId(slaveId); )
    {
        SlaveStat& slave = g_masterStat.getSlave(slaveId);

        tp.writeCell("%d", slaveId);

        tp.writeCell("%s", McuUIDToStr(slave.cmnInfo.uid).str);

        tp.writeCell("0x%08x", slave.cmnInfo.swVersion);
        tp.writeCell("%u", slave.cmnInfo.totalTime);
        tp.writeCell("%d", slave.getPingTime());

        int numRx = slave.msSpiRxOk + slave.msSpiRxError;

        tp.writeCell("%d", slave.msSpiTx.get());
        tp.writeCell("%d", slave.msSpiRxOk.get());
        tp.writeCell("%d", slave.msSpiRxError.get());
        tp.writeCell("%.1f%%", numRx > 0 ? 100.0 * slave.msSpiRxError / numRx : 0);


        SlaveSpiStat &ss = slave.cmnInfo.slaveSpiStat;
        numRx = ss.spiRxOk + ss.spiRxErr;

        tp.writeCell("%d", ss.spiTxRx);
        tp.writeCell("%d", ss.spiRxOk);
        tp.writeCell("%d", ss.spiRxErr);
        tp.writeCell("%.1f%%", numRx > 0 ? 100.0 * ss.spiRxErr / numRx : 0);

        tp.writeCell("%u", slave.noncesContainer.numSent);
        tp.writeCell("%u", slave.noncesContainer.numDrop);

        tp.newLine();
    }

    tp.printTable(wr);


    wr.printf("Total packets: %u in %u ms, speed: %.2f packets/sec\n",
              g_slaveGate.numTx, g_slaveGate.numTxMs,
              util::safeDiv(g_slaveGate.numTx, g_slaveGate.numTxMs/1000.0));
}
