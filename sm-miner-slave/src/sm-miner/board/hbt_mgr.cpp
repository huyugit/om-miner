#include "hbt_mgr.h"

#include "multy_board_mgr.h"
#include "master_gate.h"
#include "mother_board.h"
//#include "board_revisions.h"
//#include "pwr_chip_packet.h"
//#include "exchange_zone.h"
//#include "stm_gpio.h"
//#include "utils.h"
#include "polling_timer.h"
#include "i2c_board_access.h"


HbtMgr g_hbtMgr;


HbtMgr::HbtMgr()
    : boardMaskTypeA(0),
      boardMaskTypeB(0)
{}

void HbtMgr::init()
{
    boardMaskTypeA = 0;
    boardMaskTypeB = 0;

    for (int boardId = 0; boardId < MAX_BOARD_PER_SLAVE; boardId++)
    {
        BoardMgr &board = getBoard(boardId);
        if (!board.info.boardFound) continue;

        if (board.spec.isTypeA()) boardMaskTypeA |= (1 << boardId);
        if (board.spec.isTypeB()) boardMaskTypeB |= (1 << boardId);
    }
}

BoardMgr &HbtMgr::getBoard(int boardId)
{
    return g_multyBoardMgr.getBoard(boardId);
}

void HbtMgr::storePwrSwTest(int index)
{
    g_multyBoardMgr.updateBoardsInfo();

    for (int id = 0; id < MAX_BOARD_PER_SLAVE; id++)
    {
        getBoard(id).storePwrSwTest(index);
    }
}

void HbtMgr::setPower(uint32_t mask, bool on)
{
    for (int boardId = 0; boardId < MAX_BOARD_PER_SLAVE; boardId++)
    {
        if (mask & (1 << boardId))
        {
            g_motherBoard.pwrSwitch[boardId].set(on);
        }
    }
}

void HbtMgr::testOcp(uint32_t boardMask)
{
    log("TEST OCP: boardMask=0x%02x\n", boardMask);
    if (boardMask == 0) return;

    for (int boardId = 0; boardId < MAX_BOARD_PER_SLAVE; boardId++)
    {
        if (boardMask & (1 << boardId))
        {
            I2CBoardAccess access(boardId);
            access.ocpReg().write(0x7f);
        }
    }

    for (int ocp = 0x40/*0x7f*/; ocp > 0; ocp--)
    {
        if (!boardMask) {
            break;
        }

        for (int boardId = 0; boardId < MAX_BOARD_PER_SLAVE; boardId++)
        {
            if (boardMask & (1 << boardId))
            {
                I2CBoardAccess access(boardId);
                access.ocpReg().write(ocp);
            }
        }

        mysleep(2);

        g_motherBoard.loadBoardsAdc();

        for (int boardId = 0; boardId < MAX_BOARD_PER_SLAVE; boardId++)
        {
            if (boardMask & (1 << boardId))
            {
                BoardMgr &board = getBoard(boardId);

                uint32_t currU = board.getBoardVoltage();
                uint32_t currI = board.getBoardCurrent(0);

                bool off = false;
                if (currU < 1000) off = true;
                if (currI < 1000) off = true;

                if (off)
                {
                    board.test.testOcpOff = ocp;
                    boardMask &= ~(1 << boardId);
                }
            }
        }
    }

    g_multyBoardMgr.writeOcp();
}

void HbtMgr::testTmpAlert(uint32_t boardMask)
{
    log("TEST TMP ALERT: boardMask=0x%02x\n", boardMask);
    for (int boardId = 0; boardId < MAX_BOARD_PER_SLAVE; boardId++)
    {
        BoardMgr &board = getBoard(boardId);

        if (!(boardMask & (1 << boardId)))
            continue;

        if (!board.spec.isTmpAlert()) {
            continue;
        }

        for (int i = 0; i < board.spec.getNumTmp(); i++)
        {
            testBoardTmpAlert(boardId, i);
        }
    }

    g_multyBoardMgr.writeTmpAlert();
}

void HbtMgr::testBoardTmpAlert(uint8_t boardId, uint8_t tmpId)
{
    BoardMgr &board = getBoard(boardId);
    board.test.tmpAlertStatus[tmpId] = TAS_NA;

    log("---------------------------------------------\n");
    log("TEST TMP ALERT: board %d, tmp %d\n", boardId, tmpId);
    log("---------------------------------------------\n");

    // turn on board power
    g_motherBoard.pwrSwitch[boardId].set(true);

    // ensure power is on
    bool pwrOn = false;

    PollingTimer pollTimer(1000);
    while (pollTimer.inProgress())
    {
        g_multyBoardMgr.updateBoardsInfo();
        log("BOARD INFO 1: %u mV, %u mA\n", board.getBoardVoltage(), board.getBoardCurrent(0));

        pwrOn = (board.getBoardVoltage() > 1000 && board.getBoardCurrent(0) > 1000);
        if (pwrOn) break;

        mysleep(50);
    }

    if (!pwrOn)
    {
        board.test.tmpAlertStatus[tmpId] = TAS_ERR_BEGIN_I;
    }
    else {
        // write low temp threshold
        if (!board.writeTmpAlert(tmpId, 5))
        {
            board.test.tmpAlertStatus[tmpId] = TAS_ERR_WRITE;
        }
        else {
            // ensure power is off
            bool pwrOff = false;

            PollingTimer pollTimerLocal(1000);
            while (pollTimerLocal.inProgress())
            {
                g_multyBoardMgr.updateBoardsInfo();
                log("BOARD INFO 2: %u mV, %u mA\n", board.getBoardVoltage(), board.getBoardCurrent(0));

                pwrOff = !(board.getBoardVoltage() > 1000 && board.getBoardCurrent(0) > 1000);
                if (pwrOff) break;

                mysleep(50);
            }

            if (!pwrOff)
            {
                board.test.tmpAlertStatus[tmpId] = TAS_ERR_OFF_I;
            }
            else {
                board.test.tmpAlertStatus[tmpId] = TAS_OK;
            }
        }
    }

    // turn off board power
    g_motherBoard.pwrSwitch[boardId].set(false);

    if (!board.writeTmpAlert(tmpId)) {
        log("ERROR: can not restore alert\n");
    }

    // delay for tmp75 (adc convertion) to update alert pin
    mysleep(100);

    log("TEST TMP ALERT: board %d, tmp %d, status %u\n",
        boardId, tmpId, board.test.tmpAlertStatus[tmpId]);
}

void HbtMgr::runTests()
{
    for (uint32_t spi = 0; spi < g_multyBoardMgr.spiNum; spi++)
    {
        g_multyBoardMgr.getChain(spi).setupChain();
    }

    if (g_slaveCfg.testMode == TEST_MODE_HASH_BOARD &&
        g_slaveCfg.hbtConfig.testBtcTime > 0)
    {
        PwrChipChain::broadcastBtcTestImage();

        PollingTimer timer(g_slaveCfg.hbtConfig.testBtcTime);
        while (timer.inProgress())
        {
            // run individual chip processing
            for (uint32_t spi = 0; spi < g_multyBoardMgr.spiNum; spi++)
            {
                g_multyBoardMgr.getChain(spi).downloadTestStats();
            }
        }
    }
}

void HbtMgr::doTest(uint32_t boardMask)
{
    if (boardMask == 0)
        return;

    int pwrSwTest = 0;

    g_multyBoardMgr.writeOcp();

    log("--------------------------------------\n");
    log("TEST\n");
    log("--------------------------------------\n");

    // power on
    storePwrSwTest(pwrSwTest++);
    setPower(boardMask, 1);
    storePwrSwTest(pwrSwTest++);

    // run tests
    uint32_t t0 = getMiliSeconds();
    runTests();
    uint32_t t1 = getMiliSeconds();

    // power off
    storePwrSwTest(pwrSwTest++);
    setPower(boardMask, 0);
    storePwrSwTest(pwrSwTest++);

    // test ocp
    setPower(boardMask, 1);
    testOcp(boardMask);
    setPower(boardMask, 0);

    testTmpAlert(boardMask);

    log("--------------------------------------\n");
    log("POWER OFF (less then %d ms under power)\n", t1 - t0);
    log("--------------------------------------\n");
}

void HbtMgr::doFanTest()
{
    FanTestArr &arr = g_masterGate.testInfo.fanTest;
    //const double voltArr[] = { 8.5, 12.0, 14.0 };
	const double voltArr[] = { 10.8, 12.0, 13.2 };

    {
        FanConfig cfg;

        for (int index = 0; index < 3; index++)
        {
            // work around
            MotherBoard::resetWatchDog();

            cfg.setFanVoltage(voltArr[index]);

            g_motherBoard.setupFan(cfg);
            mysleep(g_slaveCfg.hbtConfig.testFanDelay);
            g_motherBoard.loadBoardsAdc();

            arr.items[index].cfg = cfg;
            arr.items[index].info = g_motherBoard.mbInfo.fan;

            arr.items[index].dump();
        }
    }

    g_motherBoard.setupFan(g_slaveCfg.fanConfig);
}

void HbtMgr::printInfo()
{
    static PollingTimer timer(5000);
    if (timer.testAndRestart())
    {
        g_multyBoardMgr.printAllChipsTestInfo();
        log("\n");
        //g_multyBoardMgr.printSystemInfo();
        g_multyBoardMgr.printInfo();
        log("\n");
    }
}

void HbtMgr::poll()
{
    static PollingTimer timer(500);
    if (timer.testAndRestart())
    {
        log("STATE: CMD REQ/ACK=%u/%u\n",
            g_slaveCfg.hbtConfig.cmdId, g_masterGate.testInfo.ackId);
    }

    {
        static PollingTimer t(500);
        if (t.testAndRestart())
        {
            // update with timer to allow LEDs to blink
            g_multyBoardMgr.updateBoardsInfo();
        }

        printInfo();
    }

    if (g_slaveCfg.hbtConfig.cmdId != g_masterGate.testInfo.ackId)
    {
        uint32_t f = g_slaveCfg.hbtConfig.cmdFlags;

        if (f & HBT_CMD_FAN_TEST)  doFanTest();

        if (f & HBT_CMD_A_ON)      setPower(boardMaskTypeA, 1);
        if (f & HBT_CMD_A_OFF)     setPower(boardMaskTypeA, 0);
        if (f & HBT_CMD_A_TEST)    doTest(boardMaskTypeA);

        if (f & HBT_CMD_B_ON)      setPower(boardMaskTypeB, 1);
        if (f & HBT_CMD_B_OFF)     setPower(boardMaskTypeB, 0);
        if (f & HBT_CMD_B_TEST)    doTest(boardMaskTypeB);

        g_masterGate.testInfo.ackId = g_slaveCfg.hbtConfig.cmdId;
    }
}
