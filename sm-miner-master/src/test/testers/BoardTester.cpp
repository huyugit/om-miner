#include "BoardTester.h"

#include "app/Application.h"
#include "config/Config.h"
#include "stats/MasterStat.h"
#include "test/HumanRange.h"


BoardTester::BoardTester(int _slaveId, int _boardId)
    : slaveId(_slaveId), boardId(_boardId)
{}


StringBuffer<1024> errorMsg;
StringBuffer<1024> warnMsg;

bool BoardTester::test()
{
    const TestConfig &testConfig = Application::config()->testConfig;
    SlaveStat &slave = g_masterStat.getSlave(slaveId);
    BoardStat &board = slave.getBoardStat(boardId);

    errorMsg.clear();
    warnMsg.clear();

    bool isOk = testBoardParams();

    if (isOk) {
        slave.setLedsBoardTestDone(boardId);
    }

    if (isOk) {
        printf("OK:  ");
    }
    else {
        printf("err: ");
    }

    printf("BRD %2d: T:%2u/%2u HSE:%u(n=%2u) U:%4.1f %4.1f I:%4.1f %4.1f",
           board.getNum(),
           board.info.boardTemperature[0], board.info.boardTemperature[1],
           board.info.heaterErr, board.info.heaterErrNum,
           (double)board.test.pwrSwTest.items[1].voltage / 1000,
           (double)board.test.pwrSwTest.items[2].voltage / 1000,
           (double)board.test.pwrSwTest.items[1].currents[0] / 1000,
           (double)board.test.pwrSwTest.items[2].currents[0] / 1000);

    if (isOk)
        printf(" OK ");
    else
        printf(" err");

    if (!errorMsg.isEmpty())
    {
        printf(": %s", errorMsg.cdata());
    }
    if (!warnMsg.isEmpty() && testConfig.testPrintWarnings)
    {
        printf(" warn: %s", warnMsg.cdata());
    }

    printf("\n");
    return isOk;
}

bool BoardTester::testBoardParams()
{
    bool ok = true;

    BoardStat &board = g_masterStat.getSlave(slaveId).getBoardStat(boardId);
    const SlaveConfig &slaveConfig = Application::config()->slaveConfig;
    const TestConfig &testConfig = Application::config()->testConfig;

    if (!board.hasData)
    {
        errorMsg.printf("no data");
        return false;
    }

    uint32_t testExpRev = testConfig.testExpRev;
    if (Application::config()->boardMaskB & (1 << board.getNum()))
    {
        testExpRev++;
    }

    if (board.spec.revisionId != testExpRev)
    {
        errorMsg.csvprintf("REV(%d)!=%d (ADC %u)",
            board.spec.revisionId, testExpRev, board.info.revAdc);
        ok = false;
    }

    for (int i = 0; i < board.spec.getNumTmp(); i++)
    {
        if (board.info.boardTemperature[i] < testConfig.testTempMin)
        {
            errorMsg.csvprintf("T[%d]<%d", i, testConfig.testTempMin);
            ok = false;
        }

        if (board.info.boardTemperature[i] > testConfig.testTempMax)
        {
            errorMsg.csvprintf("T[%d]>%d", i, testConfig.testTempMax);
            ok = false;
        }
    }

    if (board.spec.isHeatSinkErrPin())
    {
        bool hsOk = true;

        if (testConfig.testNoHeatSink)
        {
            // ensure heatsink err pin is low
            hsOk = (board.info.heaterErrNum <= testConfig.testNoHsNumMax);
        }
        else {
            // ensure heatsink err pin detected both levels
            hsOk = (board.info.heaterErrNum >= testConfig.testHsNumMin);
        }

        if (!hsOk) {
            errorMsg.csvprintf("Heatsink ERR (%u, n=%u)",
                board.info.heaterErr, board.info.heaterErrNum);
            ok = false;
        }
    }

    if (board.info.overCurrentProtection != slaveConfig.overCurrentProtection)
    {
        errorMsg.csvprintf("OCP(0x%02x)!=0x%02x",
            board.info.overCurrentProtection, slaveConfig.overCurrentProtection);
        ok = false;
    }

    if (! (board.spec.isTypeB() && board.spec.isMissingCurrent()))
    {
        if (board.test.testOcpOff < testConfig.testOcpOffMin)
        {
            errorMsg.csvprintf("OCP OFF 0x%02x < 0x%02x", board.test.testOcpOff, testConfig.testOcpOffMin);
            ok = false;
        }

        if (board.test.testOcpOff > testConfig.testOcpOffMax)
        {
            errorMsg.csvprintf("OCP OFF 0x%02x > 0x%02x", board.test.testOcpOff, testConfig.testOcpOffMax);
            ok = false;
        }
    }

    if (board.spec.isTmpAlert())
    {
        for (int i = 0; i < board.spec.getNumTmp(); i++)
        {
            switch (board.test.tmpAlertStatus[i])
            {
            case TAS_NA:            errorMsg.csvprintf("TMP %u ALERT: N/A", i);                 ok = false; break;
            case TAS_ERR_BEGIN_I:   errorMsg.csvprintf("TMP %u ALERT: low begin current", i);   ok = false; break;
            case TAS_ERR_WRITE:     errorMsg.csvprintf("TMP %u ALERT: write err", i);           ok = false; break;
            case TAS_ERR_OFF_I:     errorMsg.csvprintf("TMP %u ALERT: power still on", i);      ok = false; break;
            }
        }
    }

    for (int t = 0; t < board.test.pwrSwTest.NUM; t++)
    {
        BoardPwrSwTest &pwr = board.test.pwrSwTest.items[t];

        uint32_t voltageMin = 0;
        uint32_t voltageMax = 0;
        uint32_t currentMin = 0;
        uint32_t currentMax = 0;

        switch (t)
        {
        case 0:
        case 3:
            currentMin = 0;
            currentMax = 1000;
            break;
        case 1:
            voltageMin = testConfig.testVoltage2Min;
            voltageMax = testConfig.testVoltage2Max;
            currentMin = testConfig.testCurrent2Min;
            currentMax = testConfig.testCurrent2Max;
            break;
        case 2:
            voltageMin = testConfig.testVoltageMin;
            voltageMax = testConfig.testVoltageMax;
            currentMin = testConfig.testCurrentMin;
            currentMax = testConfig.testCurrentMax;
            break;
        }


        bool testV = true;
        bool testI = true;

        if (t == 0 || t == 3)
        {
            testV = false;
        }

        if (board.spec.isTypeB() && board.spec.isMissingCurrent())
        {
            testI = false;
        }

        if (testV)
        {
            if (pwr.voltage < voltageMin) {
                errorMsg.csvprintf("SW=%d U %d<%d", t, pwr.voltage, voltageMin);
                ok = false;
            }
            if (pwr.voltage > voltageMax) {
                errorMsg.csvprintf("SW=%d U %d>%d", t, pwr.voltage, voltageMax);
                ok = false;
            }
        }

        if (testI)
        {
            for (int i = 0; i < board.spec.pwrNum; i++)
            {
                if (pwr.currents[i] < currentMin) {
                    errorMsg.csvprintf("SW=%d I%d %d<%d", t, i, pwr.currents[i], currentMin);
                    ok = false;
                }
                if (pwr.currents[i] > currentMax) {
                    errorMsg.csvprintf("SW=%d I%d %d>%d", t, i, pwr.currents[i], currentMax);
                    ok = false;
                }
            }
        }
    }


    // scan each power chip
    int numErrMem = 0;
    int numErrNa = 0;

    StringBuffer<1024> strErrMem;
    StringBuffer<1024> strErrNa;

    for (int spiId = 0; spiId < board.spiNum(); spiId++)
    {
        for (int spiSeq = 0; spiSeq < board.spiLen(); spiSeq++)
        {
            PwcStat &pwc = board.getPwcStat(spiId, spiSeq);

            // OLD: (pwc.spiData.pwcStatOk == 0 || pwc.spiData.memOk == false)
            if (pwc.quickTest.pwcResult != PWC_TEST_OK)
            {
                if (pwc.quickTest.pwcResult == PWC_TEST_MEM_ERR)
                {
                    numErrMem++;
                    strErrMem.printf(" %d", spiSeq);
                }
                else {
                    numErrNa++;
                    strErrNa.printf(" %d", spiSeq);
                }
            }
		}
    }

    if (numErrMem > 0)
    {
        errorMsg.csvprintf("PWC MEM ERROR:%s", strErrMem.cdata());
        ok = false;
    }
    if (numErrNa > 0)
    {
        errorMsg.csvprintf("PWC RETEST:%s", strErrNa.cdata());
        ok = false;
    }


    if (testConfig.testMode != TEST_MODE_HASH_BOARD) {
        return ok;
    }


    uint32_t numErr[4];
    StringBuffer<1024> strErr[4];

    memset(numErr, 0, sizeof(numErr));

    // scan each chip
    for (int spiId = 0; spiId < board.spiNum(); spiId++)
    {
        for (int spiSeq = 0; spiSeq < board.spiLen(); spiSeq++)
        {
            HumanRange brokenChips[4];
            PwcStat &pwc = board.getPwcStat(spiId, spiSeq);

            for (PwcBtcIterator it(pwc); it.next(); )
            {
                ChipStat &chip = it.getChipStat();
                // OLD: chip.solutions <= 0
				
                uint32_t r = (pwc.quickTest.btcResults >> (chip.pwcSeq * 2)) & 0x3;

                if (r < BTC_TEST_OK)
                {
                    numErr[r]++;
                    brokenChips[r].push(chip.pwcSeq);
                }
            }

            for (int i = 0; i < 4; i++)
            {
                if (brokenChips[i].size() > 0)
                {
                    strErr[i].printf(" %d/", spiSeq);
                    brokenChips[i].printTo(strErr[i]);
                }
            }
        }
    }

    const char* errTypes[4] =
    {
        "NO RESPONSE ERROR",
        "NO JS ERROR",
        "NO SOL ERROR",
        "OK",
    };
    const unsigned int maxErr[4] =
    {
        testConfig.testMaxBtcNoResp,
        testConfig.testMaxBtcNoJs,
        testConfig.testMaxBtcNoSol,
        1000000,
    };

    for (int i = 0; i < 3; i++)
    {
        if (numErr[i] == 0)
            continue;

        if (numErr[i] > maxErr[i])
        {
            errorMsg.csvprintf("BTC %s (%d chips):%s", errTypes[i], numErr[i], strErr[i].cdata());
            ok = false;
        }
        else {
            warnMsg.csvprintf("BTC %s (%d chips):%s", errTypes[i], numErr[i], strErr[i].cdata());
        }
    }

    return ok;
}
