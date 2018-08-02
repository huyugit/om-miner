#include "ChipStat.h"

#include "precomp.h"
#include "base/MinMax.h"
#include "stats/BoardStat.h"
#include "stats/MasterStat.h"
#include "stats/hist/HistoryStatCalc.h"


uint32_t calcPositiveDiff(uint32_t x, uint32_t y)
{
    return y > x ? y - x : 0;
}


ChipStat::ChipStat()
{
}

void ChipStat::init(BoardStat *_parentBoard, int _spiId, int _spiSeq, int _pwcSeq)
{
    parentBoard   = _parentBoard;
    spiId         = _spiId;
    spiSeq        = _spiSeq;
    pwcSeq        = _pwcSeq;

    hasData = false;

    osc             = 0;
    reads           = 0;
    lastJobTime     = 0;

    solutions.clear();
    errors.clear();
    jobsDone.clear();
    restarts.clear();
}

void ChipStat::onStatData(const Btc16StatData &d, bool resetDetected)
{
    osc            = d.osc;
    reads          = d.reads;
    lastJobTime    = d.lastJobTime;

    const Btc16StatData &d1 = statData;
    const Btc16StatData &d2 = d;

    if (hasData && !resetDetected)
    {
        // accumulate positive deltas
        solutions      += calcPositiveDiff(d1.solutions, d2.solutions);
        errors         += calcPositiveDiff(d1.errors,    d2.errors);
        jobsDone       += calcPositiveDiff(d1.jobsDone,  d2.jobsDone);
        restarts       += calcPositiveDiff(d1.restarts,  d2.restarts);

        // check for overflow for uint16 data
        if (d2.restarts < d1.restarts)
            restarts += 0x10000;

        const BtcExtStat &s1 = d1.stat;
        const BtcExtStat &s2 = d2.stat;

        stat.serErrors      += s2.serErrors  - s1.serErrors;
        stat.jsErrors       += s2.jsErrors   - s1.jsErrors;

        const BtcFixesStat &f1 = d1.fixes;
        const BtcFixesStat &f2 = d2.fixes;

        fixes.rejected  += (f2.rejected     - f1.rejected);
        fixes.processed += (f2.processed    - f1.processed);
        fixes.ok        += (f2.ok           - f1.ok);
        fixes.hwErr     += (f2.hwErr        - f1.hwErr);
    }

    hasData = true;
    statData = d;
}

void ChipStat::aggregate(TotalStat &total)
{
    total.aggregate(*this);
}

void ChipStat::saveStat()
{
    solutions.save();
    errors.save();
    jobsDone.save();
    restarts.save();
}

void ChipStat::printStat(Writer &wr, PrintStatOpt &printOpt)
{
    TablePrinter &tp = printOpt.tp;

    if (printOpt.statLevel == STAT_LEVEL_CHIP)
    {
        TotalStat totalStat(*this);
        HistoryStatCalc calc(totalStat, printOpt.useTotal);

        if (printOpt.printHeader)
        {
            wr.printf("*** CHIPS STAT %s (%d sec):\n",
                      printOpt.useTotal ? "TOTAL" : "DELTA",
                      calc.getSeconds());
            printOpt.printHeader = false;

            tp.writeCell("BRD");
            tp.writeCell("SPI");
            tp.writeCell("CHIP");
            tp.writeCell("");

            tp.writeCell("OSC");
            tp.writeCell("READ");
            tp.writeCell("");

            tp.writeCell("SOL");
            tp.writeCell("ERR");
            tp.writeCell("E/S");
            tp.writeCell("SPEED");
            tp.writeCell("EFF");
            tp.writeCell("");

            tp.writeCell("REJ");
            tp.writeCell("TST");
            tp.writeCell("OK");
            tp.writeCell("R/J");
            tp.writeCell("HWE");
            tp.writeCell("");

            tp.writeCell("SE");
            tp.writeCell("JSE");
            tp.writeCell("");

            tp.writeCell("JN");
            tp.writeCell("JT");
            tp.writeCell("SPEED");
            tp.writeCell("CR");
            tp.newLine();
        }

        double bySol = calc.getGHs();
        double byJobs = calc.getGHsByJobs();

        tp.writeCell("%d",          parentBoard->getNum());
        tp.writeCell("%d",          spiId);
        tp.writeCell("%d/%d",       spiSeq, pwcSeq);
        tp.writeCell("|");

        tp.writeCell("%d",          osc);
        tp.writeCell("%d",          reads);
        tp.writeCell("|");

        tp.writeCell("%llu",        calc.getSol());
        tp.writeCell("%llu",        calc.getErr());
        tp.writeCell("%.0f%%",      calc.getErrToSol());
        tp.writeCell("%.2f GHs",    bySol);
        tp.writeCell("%.0f%%",      util::safeDiv(bySol, byJobs) * 100);
        tp.writeCell("|");

        tp.writeCell("%u",          fixes.rejected);
        tp.writeCell("%u",          fixes.processed);
        tp.writeCell("%u",          fixes.ok);
        tp.writeCell("%.0f%%",      util::safeDiv(fixes.ok, jobsDone.get()) * 100);
        tp.writeCell("%u",          fixes.hwErr);
        tp.writeCell("|");

        tp.writeCell("%d",          stat.serErrors);
        tp.writeCell("%d",          stat.jsErrors);
        tp.writeCell("|");

        tp.writeCell("%llu",        jobsDone.get());
        tp.writeCell("%d",          lastJobTime);
        tp.writeCell("%.2f GHs",    byJobs);
        tp.writeCell("%llu",        restarts.get());
        tp.newLine();
    }
}
