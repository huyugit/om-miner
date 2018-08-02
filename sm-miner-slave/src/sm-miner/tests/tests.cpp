#include "tests.h"

#include "common.h"
#include "format.hpp"
#include "mother_board.h"
#include "master_gate.h"
#include "multy_board_mgr.h"
#include "i2c_board_access.h"
#include "utils.h"
#include "mytime.h"


void test_JobGenerating()
{
    g_masterGate.emulateOnMiningData();
    log("Active MiningData: "); g_masterGate.miningData.dump();
}

void test_StratumJobQueue()
{
    for (int i = 0; i < 10; i++)
    {
        g_masterGate.emulateOnMiningData();

//        g_stratumPool.stratumJobsPool.dump();

//        log("g_stratumPool.stratumJobsPool:\n");
//        for (int i = 0; i < g_stratumPool.stratumJobsCount; i++)
//        {
//            log("[%d] = id: %d\n", i, g_stratumPool.stratumJobs[i]->id);
//        }
    }
}


void runTests()
{
    if (DEBUG_LEVEL > 0)
    {
        test_JobGenerating();
        test_StratumJobQueue();
        STOP();
    }

    // TEST/DEBUG: board i2c temperature sensor
    if (0) {
        while (1)
        {
            log("-----------------------------------\n");
            int boardId = 4;
            //for (int boardId = 0; boardId < MAX_BOARD_PER_SLAVE; boardId++)
            {
                BoardMgr &board = g_multyBoardMgr.getBoard(boardId);

                I2CBoardAccess access(boardId);
                //access.i2c().scan();

                for (int i = 0; i < board.spec.getNumTmp(); i++)
                {
                    I2CTmp75 tmp75 = access.tmp75(i);
                    if (tmp75.read())
                    {
                        log("tmp75[%d]: %u C\n", i, tmp75.temp);
                    }
                    else {
                        log("tmp75[%d]: read fail\n", i);
                    }
                }
            }

            mysleep(1000);
        }
    }

    // TEST/DEBUG: board OCP (over current protection)
    if (0) {
        while (1)
        {
            log("-----------------------------------\n");
            int boardId = 0;
            //for (int boardId = 0; boardId < MAX_BOARD_PER_SLAVE; boardId++)
            {
                I2CBoardAccess access(boardId);
                uint8_t data;

                access.ocpReg().read(data);
                access.ocpReg().write(0x7f);
            }

            mysleep(1000);
        }
    }

    // TEST/DEBUG: board OCP + power switch + low current detection
    if (0) {
        while (1)
        {
            log("-----------------------------------\n");
            int boardId = 2;
            BoardMgr &board = g_multyBoardMgr.getBoard(boardId);

            {
                I2CBoardAccess access(boardId);
                access.i2c().debug = false;
                access.ocpReg().write(0x7f);
            }

            g_motherBoard.pwrSwitch[boardId].set(true);

            uint32_t prevU = 0;
            uint32_t prevI = 0;

            for (int ocp = 0x7f; ocp > 0; ocp--)
            {
                uint8_t ocp2 = 0;
                {
                    I2CBoardAccess access(boardId);
                    access.ocpReg().write(ocp);
                    access.ocpReg().read(ocp2);
                }

                mysleep(2);

                g_motherBoard.loadBoardsAdc();

                uint32_t currU = board.getBoardVoltage();
                uint32_t currI = board.getBoardCurrent(0);

                bool off = false;
                if (currU < 1000) off = true;
                if (currI < 1000) off = true;

                if (off)
                {
                    log("BOARD[%d]: OCP=0x%02x (%3u), U=%5u mV, I=%5u mA\n",
                        boardId, ocp2, ocp2, prevU, prevI);
                    log("BOARD[%d]: OCP=0x%02x (%3u), U=%5u mV, I=%5u mA\n",
                        boardId, ocp2-1, ocp2-1, currU, currI);
                    break;
                }

                prevU = currU;
                prevI = currI;
            }

        }
    }

    // TEST/DEBUG: ADC
    if (0) {
        g_slaveCfg.debugOpt |= SLAVE_DEBUG_LOG_ADC;
        while (1) {
            g_motherBoard.loadBoardsAdc();

//            {
//                I2CBoardAccess access(2);
//                access.ocpReg().write(0x30);
//            }

            g_multyBoardMgr.updateBoardsInfo();
            //printSystemInfo();

            log("*** SYSTEM STATS ***\n");
            //int boardId = 2;
            for (int boardId = 0; boardId < MAX_BOARD_PER_SLAVE; boardId++)
            {
                BoardMgr &board = g_multyBoardMgr.getBoard(boardId);

                log("BOARD[%d]: F=%d T=%2d/%2d C RevADC=%4d HE=%u (%u) U=%u mv",
                    board.boardId,
                    board.info.boardFound,
                    board.info.boardTemperature[0], board.info.boardTemperature[1],
                    board.info.revAdc,
                    board.info.heaterErr,
                    board.info.heaterErrNum,
                    board.info.voltage);

                for (int i = 0; i < board.spec.pwrNum; i++)
                {
                    log(" I%d=%5dma", i, board.info.currents[i]);
                }
                log("\n");
            }

            mysleep(1000);
        }
    }

    // TEST/DEBUG: power switch on/off
    if (0) {
        bool pwrOn = false;
        while (1) {
            pwrOn = !pwrOn;

            for (int i = 0; i < MAX_BOARD_PER_SLAVE; i++)
            {
                BoardMgr &b = g_multyBoardMgr.getBoard(i);
                if (!b.info.boardFound) continue;

                g_motherBoard.pwrSwitch[i].set(pwrOn);
            }

            for (int i = 0; i < 5; i++)
            {
                g_motherBoard.loadBoardsAdc();

                for (int iBoard = 0; iBoard < MAX_BOARD_PER_SLAVE; iBoard++)
                {
                    BoardMgr &b = g_multyBoardMgr.getBoard(iBoard);
                    if (!b.info.boardFound) continue;

                    log("BOARD[%d]: U=%5u mV, I=%5u mA\n", iBoard, b.getBoardVoltage(), b.getBoardCurrent(0));
                }
                mysleep(200);
            }
        }
    }

    // TEST/DEBUG: fan DAC
    if (0) {
        FanConfig cfg;
        while (1) {
            log("--------------------------------\n");

            for (int i = 0; i <= 0x1000; i += 0x400)
            {
                cfg.fanDac = i < 0x1000 ? i : 0xfff;
                g_motherBoard.setupFan(cfg);

                cfg.dump();
                mysleep(5000);
            }
        }
    }

    // DEBUG: spi to pwc (physical level)
    if (1) {
        g_spiExchange.measureSpeed();
    }
    if (0) {
        g_spiExchange.testSignal();
    }
    if (0) {
        g_slaveCfg.spiResetCycles = 200;
        while (1) {
            log("resetLine\n");
            g_spiExchange.resetLine(0);
            mysleep(10);
        }
    }
}
