#include "ServerTest.h"

#include <math.h>

#include "base/PollTimer.h"
#include "app/Application.h"
#include "config/Config.h"
#include "hw/GpioManager.h"
#include "pool/StratumPool.h"
#include "env/EnvManager.h"
#include "slave-gate/SlaveGate.h"
#include "stats/MasterStat.h"
#include "test/HumanRange.h"
#include "test/testers/BoardTester.h"


ServerTest g_serverTest;


ServerTest::ServerTest()
    : cmdId(0)
{
    clearTestStat();
}

void ServerTest::clearTestStat()
{
    numSlave = 0;
    numSlaveOk = 0;
    numBoard = 0;
    numBoardOk = 0;
}

void ServerTest::run()
{
    // Set LEDs to Server Test mode
    for (int slaveId = -1; EnvManager::nextSlaveId(slaveId); )
    {
        SlaveStat &slave = g_masterStat.getSlave(slaveId);
        slave.setLedsServerTest();
    }

    waitPowerOn();

    runTest();
}

void ServerTest::waitPowerOn()
{
    PsuMgrInfo &mi = g_masterStat.getSlave(0).psuMgrInfo;

    PollTimer timer, logTimer;
    while (true)
    {
        bool done = (mi.state == STATE_FULL_POWER);

        if (logTimer.isElapsedSec(1) || done)
        {
            printf("pwr: state %s, time %u sec, %.1fV\n",
                   powerStateToStr(mi.getState()), mi.stateSec / 1000, mi.getSetVoltage());
            logTimer.start();
        }

        if (done)
            break;

        if (timer.isElapsedSec(20))
            throw ApplicationException("Unable to start PSU");

        g_slaveGate.runPollingIteration();
        g_envManager.runPollingIteration();
    }
}

void ServerTest::waitFanOn()
{
    TestConfig &testConfig = Application::configRW().testConfig;
    FanInfo &fi = g_masterStat.getSlave(0).cmnInfo.mbInfo.fan;

    PollTimer timer, logTimer;
    while (true)
    {
        bool done = (fi.getFanI() > testConfig.testFanWaitI);

        if (logTimer.isElapsedSec(2) || done)
        {
            printf("wait fan on: %.1fV, %.1fA\n",
                   fi.getFanU(), fi.getFanI());
            logTimer.start();
        }

        if (done)
            break;

        if (timer.isElapsedSec(20))
            throw ApplicationException("Unable to start FANs");

        g_slaveGate.runPollingIteration();
        g_envManager.runPollingIteration();
    }

}

void ServerTest::runTest()
{
    TestConfig &testConfig = Application::configRW().testConfig;

    uint32_t testNum = testConfig.testNum;

    testType = 0;
    testStep = 0;

    flagMasterTest =
            (testConfig.testMode == TEST_MODE_SERVER ||
             testConfig.testMode == TEST_MODE_MOTHER_BOARD);
    flagSlaveTest =
            (testConfig.testMode == TEST_MODE_SERVER ||
             testConfig.testMode == TEST_MODE_MOTHER_BOARD);
    flagFanTest =
            (testConfig.testMode == TEST_MODE_SERVER ||
             testConfig.testMode == TEST_MODE_MOTHER_BOARD ||
             testConfig.testMode == TEST_MODE_FAN_BOARD);
    flagHashBoardTest =
            (testConfig.testMode == TEST_MODE_SERVER ||
             testConfig.testMode == TEST_MODE_HASH_BOARD ||
             testConfig.testMode == TEST_MODE_MOTHER_BOARD);

    if (flagFanTest)
    {
        printf("STARTING TEST...\n");
        execCmd(HBT_CMD_FAN_TEST);
        checkResults();
    }

    if (flagHashBoardTest)
    {
        waitFanOn();

        for (testStep = 0; testStep < testNum; testStep++)
        {
            printf("STARTING TEST...\n");
            if (!execCmd(HBT_CMD_A_OFF | HBT_CMD_B_ON)) break;
            if (!execCmd(HBT_CMD_A_TEST)) break;
            if (!execCmd(HBT_CMD_A_OFF | HBT_CMD_B_OFF)) break;

            testType = 0;
            if (checkResults()) break;
            doCooling();

            if (Application::configRW().boardMaskB == 0)
                continue;

            printf("STARTING TEST...\n");
            if (!execCmd(HBT_CMD_A_ON | HBT_CMD_B_OFF)) break;
            if (!execCmd(HBT_CMD_B_TEST)) break;
            if (!execCmd(HBT_CMD_A_OFF | HBT_CMD_B_OFF)) break;

            testType = 1;
            if (checkResults()) break;
            doCooling();
        }
    }

    // 5 seconds pause to propgate board LED status to slaves
    PollTimer finishTimer;
    while (!finishTimer.isElapsedSec(5))
    {
        g_slaveGate.runPollingIteration();
        g_envManager.runPollingIteration();
    }
}

bool ServerTest::checkResults()
{
    TestConfig &testConfig = Application::configRW().testConfig;

    uint32_t testNum = testConfig.testNum;

    const char* modeStr = "N/A";
    switch (Application::configRW().testConfig.testMode)
    {
    case TEST_MODE_SERVER:          modeStr = "SERVER"; break;
    case TEST_MODE_HASH_BOARD:      modeStr = "HASH BOARD"; break;
    case TEST_MODE_MOTHER_BOARD:    modeStr = "MOTHER BOARDS"; break;
    case TEST_MODE_FAN_BOARD:       modeStr = "FAN BOARD"; break;
    }

    printf("\n");
    printf("---------------------------------------\n");
    printf("----- %s TEST %u/%u (BRD %s)\n", modeStr, testStep+1, testNum, (testType == 0 ? "A" : "B"));
    printf("---------------------------------------\n");
    printf("\n");

    clearTestStat();

    // Board Tests

    bool done = true;

    if (!checkMasterResults())
        done = false;

    if (!checkSlaveResults())
        done = false;

    if (!checkFanResults())
        done = false;

    if (flagHashBoardTest)
    {
        for (int slaveId = -1; EnvManager::nextSlaveId(slaveId); )
        {
            for (int boardId = 0; boardId < MAX_BOARD_PER_SLAVE; boardId++)
            {
                SlaveStat &slave = g_masterStat.getSlave(slaveId);
                BoardStat &board = slave.getBoardStat(boardId);

                if ((Application::configRW().boardMask & (1 << board.getNum())) == 0)
                    continue;

                BoardTester boardValidator(slaveId, boardId);
                numBoard++;

                if (boardValidator.test()) {
                    numBoardOk++;
                }
                else {
                    done = false;
                }
            }
        }
    }

    printf("\n");
    printf("TEST: %s\n", (done ? "PASSED" : "FAILED"));
    return done;
}

bool ServerTest::checkMasterResults()
{
    if (!flagMasterTest)
        return true;

    TestConfig &testConfig = Application::configRW().testConfig;

    bool verOk = (g_gpioManager.mbHwVer == testConfig.testMbHwVer);
    printf("OPI hwVer 0x%x: %s\n", g_gpioManager.mbHwVer, (verOk ? "PASSED" : "FAILED"));

    return verOk;
}

bool ServerTest::checkSlaveResults()
{
    if (!flagSlaveTest)
        return true;

    bool result = true;

    TestConfig &testConfig = Application::configRW().testConfig;

    for (int slaveId = -1; EnvManager::nextSlaveId(slaveId); )
    {
        SlaveMbInfo &mbInfo = g_masterStat.getSlave(slaveId).cmnInfo.mbInfo;

        bool ok = (mbInfo.hwVer == testConfig.testSlaveHwVer);
        printf("SLAVE %d: hwVer 0x%x: %s\n", slaveId, mbInfo.hwVer, (ok ? "PASSED" : "FAILED"));

        if (!ok) {
            result = false;
        }
    }

    {
        int slaveId = 0;
        SlaveMbInfo &mbInfo = g_masterStat.getSlave(0).cmnInfo.mbInfo;

        double expV =  g_masterStat.getSlave(0).psuMgrInfo.getSetVoltage();
        double dev = fabs(mbInfo.getMbVoltage() - expV) / expV;
        bool ok = (dev < 0.05);

        printf("SLAVE %d: U=%.1fV: %s\n", slaveId, mbInfo.getMbVoltage(), (ok ? "PASSED" : "FAILED"));

        if (!ok) {
            result = false;
        }
    }

    return result;
}

bool ServerTest::checkFanResults()
{
    if (!flagFanTest)
        return true;

    TestConfig &testConfig = Application::configRW().testConfig;
    bool ok = true;

    FanTestArr &arr = g_masterStat.getSlave(0).testInfo.fanTest;
    for (int i = 0; i < arr.NUM; i++)
    {
        FanConfig &cfg = arr.items[i].cfg;
        FanInfo &info = arr.items[i].info;

        StringBuffer<1024> errorMsg;

        double dev = fabs(info.getFanU() - cfg.getFanVoltage()) / cfg.getFanVoltage();
        if (dev > testConfig.testFanOnVDev) {
            errorMsg.csvprintf("U out of range (> %.2f)", testConfig.testFanOnVDev);
        }
        if (info.getFanI() < testConfig.testFanOnIMin) {
            errorMsg.csvprintf("I < %.1fA", testConfig.testFanOnIMin);
        }
        if (info.getFanI() > testConfig.testFanOnIMax) {
            errorMsg.csvprintf("I > %.1fA", testConfig.testFanOnIMax);
        }

        bool testOk = errorMsg.isEmpty();

        printf("FAN TEST: %6.3fV  =>  %6.3fV %6.3fA: %s\n",
               cfg.getFanVoltage(),
               info.getFanU(), info.getFanI(),
               (testOk ? "PASSED" : errorMsg.cdata()));

        if (!testOk) {
            ok = false;
        }
    }

    printf("FAN TEST: %s\n", (ok ? "PASSED" : "FAILED"));
    return ok;
}

bool ServerTest::execCmd(uint32_t cmdFlags)
{
    cmdId++;

    SlaveHbtConfig &cfg = Application::configRW().slaveConfig.hbtConfig;
    cfg.cmdId = cmdId;
    cfg.cmdFlags = cmdFlags;


    StringBuffer<1024> msg;
    if (cmdFlags & HBT_CMD_A_ON)   msg.csvprintf("BRD A ON");
    if (cmdFlags & HBT_CMD_A_OFF)  msg.csvprintf("BRD A OFF");
    if (cmdFlags & HBT_CMD_A_TEST) msg.csvprintf("BRD A TEST");
    if (cmdFlags & HBT_CMD_B_ON)   msg.csvprintf("BRD B ON");
    if (cmdFlags & HBT_CMD_B_OFF)  msg.csvprintf("BRD B OFF");
    if (cmdFlags & HBT_CMD_B_TEST) msg.csvprintf("BRD B TEST");
    if (cmdFlags & HBT_CMD_FAN_TEST) msg.csvprintf("FAN TEST");

    printf("EXEC: ID %u, CMD: %s\n", cmdId, msg.cdata());


    PollTimer timer, dotTimer;
    while (true)
    {
        bool done = true;
        for (int slaveId = -1; EnvManager::nextSlaveId(slaveId); )
        {
            SlaveTestInfo &testInfo = g_masterStat.getSlave(slaveId).testInfo;
            if (testInfo.ackId != cmdId || testInfo.ackRes != 0)
            {
                done = false;
            }
        }

        if (done) {
            break;
        }

        if (dotTimer.isElapsedSec(2))
        {
            printf("test in progress, %u sec\n", timer.elapsedSec());
            dotTimer.start();
        }

        if (timer.isElapsedSec(Application::configRW().testConfig.testTimeSec))
        {
            for (int slaveId = -1; EnvManager::nextSlaveId(slaveId); )
            {
                SlaveTestInfo &testInfo = g_masterStat.getSlave(slaveId).testInfo;
                if (testInfo.ackId != cmdId) {
                    printf("ERROR: SLAVE %u: no response\n", slaveId);
                }
                if (testInfo.ackRes != 0) {
                    printf("ERROR: SLAVE %u: error %u\n", slaveId, testInfo.ackRes);
                }
            }

            return false;
        }

        g_slaveGate.runPollingIteration();
        g_envManager.runPollingIteration();

        usleep(1000 * Application::configRW().pollingDelayMs);
    }

    return true;
}

void ServerTest::doCooling()
{
    uint32_t sec = Application::configRW().testConfig.testCoolingTimeSec;
    if (sec > 0)
    {
        printf("COOLING %u sec\n", sec);

        PollTimer timer;
        while (!timer.isElapsedSec(sec))
        {
            g_slaveGate.runPollingIteration();
            g_envManager.runPollingIteration();

            usleep(1000 * Application::configRW().pollingDelayMs);
        }
    }
}
