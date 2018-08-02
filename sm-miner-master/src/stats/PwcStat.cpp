#include "PwcStat.h"

#include "precomp.h"
#include "stats/BoardStat.h"
#include "stats/MasterStat.h"
#include "stats/hist/HistoryStatCalc.h"


PwcBtcIterator::PwcBtcIterator(PwcStat &_pwc)
    : pwc(_pwc), pwcSeq(-1), leftMask(_pwc.btcMask())
{
}

ChipStat &PwcBtcIterator::getChipStat() const
{
    if (pwcSeq >= 0) {
        return pwc.getChipStat(pwcSeq);
    }
    else {
        return PwcStat::dummyChip;
    }
}

bool PwcBtcIterator::next()
{
    if (leftMask != 0)
    {
        while ((leftMask & 1) == 0)
        {
            leftMask >>= 1;
            pwcSeq++;
        }

        leftMask >>= 1;
        pwcSeq++;

        return true;
    }

    return false;
}


ChipStat PwcStat::dummyChip;


PwcStat::PwcStat()
{
}

void PwcStat::init(BoardStat *_parentBoard, int _spiId, int _spiSeq)
{
    parentBoard   = _parentBoard;
    spiId         = _spiId;
    spiSeq        = _spiSeq;

    for (int pwcSeq = 0; pwcSeq < MAX_BTC16_PER_PWC; pwcSeq++)
    {
        chips[pwcSeq].init(parentBoard, spiId, spiSeq, pwcSeq);
    }

    dummyChip.init(&SlaveStat::dummyBoard, 0, 0, 0);

    quickTest.clear();
}

uint8_t PwcStat::btcNum() const
{
    return parentBoard->btcNum();
}

uint16_t PwcStat::btcMask() const
{
    return parentBoard->btcMask();
}

PwcBtcIterator PwcStat::btcIterator()
{
    return PwcBtcIterator(*this);
}

ChipStat& PwcStat::getChipStat(int pwcSeq)
{
    if (pwcSeq >= 0 && pwcSeq < MAX_BTC16_PER_PWC)
    {
        return chips[pwcSeq];
    }
    else {
        printf("ERROR: getChipStat(%d): out of range!\n", pwcSeq);
        return dummyChip;
    }
}

void PwcStat::onPwcBlock(const PwcBlock &pb)
{
    const PwcSharedData &psd1 = pwcSharedData;
    const PwcSharedData &psd2 = pb.pwcSharedData;

    bool resetDetected = (psd2.totalTime < psd1.totalTime);

    if (!resetDetected)
    {
        // accumulate positive deltas
        totalSolByDiff  += psd2.totalSolByDiff  - psd1.totalSolByDiff;
        sharesSent      += psd2.sharesSent      - psd1.sharesSent;
        sharesDropped   += psd2.sharesDropped   - psd1.sharesDropped;
    }

    pwcSharedData = pb.pwcSharedData;
    pingTimer.start();

    for (int pwcSeq = 0; pwcSeq < pb.len; pwcSeq++)
    {
        ChipStat &chip = getChipStat(pwcSeq);
        chip.onStatData(pb.btc16StatData[pwcSeq], resetDetected);
    }
}

void PwcStat::aggregate(TotalStat &total)
{
    for (PwcBtcIterator it(*this); it.next(); )
    {
        it.getChipStat().aggregate(total);
    }

    if (pingTimer.elapsedSec() > 120) {
        total.numBrokenPwc++;
    }

    total.pwcSolByDiff      += totalSolByDiff;
    total.pwcSharesSent     += sharesSent;
    total.pwcSharesDropped  += sharesDropped;
}

void PwcStat::saveStat()
{
    for (PwcBtcIterator it(*this); it.next(); )
    {
        it.getChipStat().saveStat();
    }

    totalSolByDiff.save();
    sharesSent.save();
    sharesDropped.save();
}

void PwcStat::printStat(Writer &wr, PrintStatOpt &printOpt)
{
    TablePrinter &tp = printOpt.tp;

    if (printOpt.statLevel == STAT_LEVEL_PWC)
    {
        if (printOpt.printHeader)
        {
            wr.printf("*** POWER CHIP STAT:\n");
            printOpt.printHeader = false;

            tp.writeCell("BRD");
            tp.writeCell("PWC");
            tp.writeCell("");

            tp.writeCell("F");
            tp.writeCell("T");
            tp.writeCell("CFG");
            tp.writeCell("");

            tp.writeCell("BF");
            tp.writeCell("CNST");
            tp.writeCell("UNQ");
            tp.writeCell("TOTAL");
            tp.writeCell("UNIQUE");
            tp.writeCell("");

            tp.writeCell("STAT");
            tp.writeCell("LOAD");
            tp.writeCell("SHIFT");
            tp.writeCell("TIME");
            tp.writeCell("CPU");
            tp.writeCell("");

            tp.writeCell("SS");
            tp.writeCell("SD");
            tp.newLine();
        }

        tp.writeCell("%d",          parentBoard->getNum());
        tp.writeCell("%d/%2d",      spiId, spiSeq);
        tp.writeCell("|");

        tp.writeCell("%d",          spiData.found);
        tp.writeCell("%d",          spiData.pwcTest);
        tp.writeCell("%d",          spiData.uniqConfigLoaded);
        tp.writeCell("|");

        tp.writeCell("%d",          spiData.memVendor);
        tp.writeCell("%d",          spiData.memConst);
        tp.writeCell("%d",          spiData.memUniq);
        tp.writeCell("%d",          spiData.memTotal);
        tp.writeCell("0x%08x",      spiData.memUniqVal);
        tp.writeCell("|");

        tp.writeCell("%d",          spiData.pwcStatOk);
        tp.writeCell("%d",          spiData.pwcStatTotal);
        tp.writeCell("%d",          spiData.pwcStatShift);
        tp.writeCell("%d",          pwcSharedData.totalTime);
        tp.writeCell("%d MHz",      pwcSharedData.cpuFreq);
        tp.writeCell("|");

        tp.writeCell("%d",          pwcSharedData.sharesSent);
        tp.writeCell("%d",          pwcSharedData.sharesDropped);
        tp.newLine();
    }
    else
    {
        for (PwcBtcIterator it(*this); it.next(); )
        {
            it.getChipStat().printStat(wr, printOpt);
        }
    }
}
