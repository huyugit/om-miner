#include "multy_board_mgr.h"
#include "master_gate.h"
#include "tests.h"
#include "mother_board.h"
#include "static_allocator.h"
#include "board_revisions.h"
#include "pwr_chip_packet.h"
#include "exchange_zone.h"
#include "stm_gpio.h"
#include "utils.h"
#include "polling_timer.h"
#include "i2c_board_access.h"
#include "hbt_mgr.h"
#include "eltek_mgr.h"
#include "power_mgr.h"
#include "pwrmodule_mgr.h"

MultyBoardMgr g_multyBoardMgr;
MultyBoardMgr::MultyBoardMgr()
    : cmnConfigBroadcasts(0),
      miningDataBroadcasts(0),
      totalSolutions(0),
      totalSolutionsByDiff(0),
      hashBoardPowerDownFlag(0)
{
    for (int boardId = 0; boardId < MAX_BOARD_PER_SLAVE; boardId++)
    {
        boards[boardId].init(boardId);
    }

	maxHSBMaxTemp = getHSBMaxTemp();
}

PwrChip& MultyBoardMgr::getChip(uint32_t index)
{
    return chips[index];
}

PwrChip &MultyBoardMgr::getBoardChip(uint8_t boardId, uint32_t index)
{
    return chips[boardStartIndex[boardId] + index];
}

PwrChip& MultyBoardMgr::getSpiGridChip(uint8_t boardId, uint8_t spiLine, uint8_t spiSeq)
{
    uint8_t spiLenLocal = boards[boardId].spec.spiLen;
    return chips[boardStartIndex[boardId] + spiLine*spiLenLocal + spiSeq];
}

PwrChip& MultyBoardMgr::getPwrGridChip(uint8_t boardId, uint8_t pwrLine, uint8_t pwrSeq)
{
    uint8_t pwrLen = boards[boardId].spec.pwrLen;
    return chips[boardStartIndex[boardId] + pwrLine*pwrLen + pwrSeq];
}

void MultyBoardMgr::configureBoardsAuto()
{
    for (int boardId = 0; boardId < MAX_BOARD_PER_SLAVE; boardId++)
    {
        boards[boardId].autoConfigure();
    }

    reconfigureGrid();
}

void MultyBoardMgr::configureBoardsBySpec(uint8_t slotMask, const BoardSpec &spec)
{
    for (int boardId = 0; boardId < MAX_BOARD_PER_SLAVE; boardId++)
    {
        if (slotMask & (1 << boardId))
        {
            boards[boardId].configure(spec);
        }
        else {
            boards[boardId].configureNone();
        }
    }

    reconfigureGrid();
}

void MultyBoardMgr::reconfigureGrid()
{
    log("Reconfiguring SPI grid:\n");

    numChips = 0;

    spiNum = 0;
    spiLen = 0;
    spiMask = 0;

    for (int boardId = 0; boardId < MAX_BOARD_PER_SLAVE; boardId++)
    {
        BoardMgr &board = boards[boardId];
        if (board.info.boardFound)
        {
            log("BOARD[%d]:\n", boardId); board.dump();

            boardStartIndex[boardId] = numChips;

            // validating board
            BoardSpec &bs = board.spec;

            uint32_t numChipsNew = numChips + bs.spiNum * bs.spiLen;

            if (numChipsNew > MAX_PWC_PER_SLAVE)
            {
                log("ERROR: too many total chips %d (max %d), ignoring board\n",
                    numChipsNew, MAX_PWC_PER_SLAVE);

                board.info.boardFound = false;
                continue;
            }

            uint8_t spiNumNew = spiNum + bs.spiNum;
            uint8_t spiLenNew = MAX(spiLen, bs.spiLen);

            if (spiNumNew > MAX_SPI_PER_SLAVE)
            {
                log("ERROR: too many total spi %d (max %d), ignoring board\n",
                    spiNumNew, MAX_SPI_PER_SLAVE);

                board.info.boardFound = false;
                continue;
            }

            if (numSetBits(bs.spiMask) != bs.spiNum)
            {
                log("ERROR: wrong board's spi mask, ignoring board\n");

                board.info.boardFound = false;
                continue;
            }


            numChips = numChipsNew;

            spiNum = spiNumNew;
            spiLen = spiLenNew;

            spiMask |= bs.spiMask << (MAX_SPI_PER_BOARD * boardId);

//            log("board.spiMask: 0x%02x\n", board.spiMask);
//            log("spiMask:       0x%02x\n", spiMask);
        }
    }


    ASSERT(numChips <= MAX_PWC_PER_SLAVE, "");
    ASSERT(spiNum <= MAX_SPI_PER_SLAVE, "");
    ASSERT(spiLen <= MAX_PWC_PER_SPI, "");
    ASSERT(numSetBits(spiMask) == spiNum, "");


    log("Resulted SPI grid:\n");
    log("  spi x len: %d x %d\n", spiNum, spiLen);
    log("  spi mask:  0x%04x\n", spiMask);
    log("\n");

    g_spiExchange.init(spiMask);


    for (uint32_t i = 0; i < numChips; i++)
    {
        chips[i].onSlaveId = i;
    }


    uint32_t spiMaskTemp = spiMask;
    uint8_t spi = 0;

    for (uint8_t boardId = 0; boardId < MAX_BOARD_PER_SLAVE; boardId++)
    {
        uint8_t boardSpiId = 0;

        for (uint8_t boardSpiLine = 0; boardSpiLine < MAX_SPI_PER_BOARD; boardSpiLine++)
        {
            if (spiMaskTemp & 1)
            {
                uint8_t spiLength = boards[boardId].spec.spiLen;
                getChain(spi).init(spi, spiLength, boardId, boardSpiId);

                spi++;
                boardSpiId++;
            }

            spiMaskTemp >>= 1;
        }
    }

    for (uint8_t boardId = 0; boardId < MAX_BOARD_PER_SLAVE; boardId++)
    {
        BoardSpec &spec = boards[boardId].spec;

        for (int onBoardId = 0; onBoardId < spec.pwrNum * spec.pwrLen; onBoardId++)
        {
            PwrChip &chip = getBoardChip(boardId, onBoardId);

            chip.boardId    = boardId;
            chip.onBoardId  = onBoardId;

            chip.pwrId      = onBoardId / spec.pwrLen;
            chip.pwrSeq     = onBoardId % spec.pwrLen;
        }
    }
}

void MultyBoardMgr::activateCan()
{
	log("activateCan: slaveId = %d\n",g_masterGate.masterData.slaveId);
	g_ModuleMgr.setEnable();
}

void MultyBoardMgr::updateBoardsInfo()
{
    g_motherBoard.loadBoardsAdc();

    for (int id = 0; id < MAX_BOARD_PER_SLAVE; id++)
    {
        getBoard(id).updateInfo();
		getBoard(id).getBoardOSCconfig();  //chenbo add 20180109
    }

	maxHSBMaxTemp = getHSBMaxTemp();
}

void MultyBoardMgr::processPwrSw()
{
	if(g_multyBoardMgr.hashBoardPowerDownFlag == 0)
	{
	    for (int id = 0; id < MAX_BOARD_PER_SLAVE; id++)
	    {
	        getBoard(id).processPwrSw();
	    }
	}else if(g_multyBoardMgr.hashBoardPowerDownFlag == 1)
	{
		for (int id = 0; id < MAX_BOARD_PER_SLAVE; id++)
		{
			g_motherBoard.pwrSwitch[id].set(false);
		}
	}
}

void MultyBoardMgr::setOSCconfig()
{
	for (int id = 0; id < MAX_BOARD_PER_SLAVE; id++)
    {
    	getBoard(id).getBoardOSCconfig();
    }
}

void MultyBoardMgr::runLoop()
{
    if (!g_powerMgr.isPowerDetected())
    {
        static PollingTimer timer( 500 );
        if (timer.testAndRestart())
        {
            log("runLoop: waiting power...\n");
        }
        return;
    }

    uint32_t now = getMiliSeconds();

    // setup chain
    uint32_t mask = 0;
    for (uint32_t spi = 0; spi < spiNum; spi++)
    {
        PwrChipChain &chain = getChain(spi);
        BoardMgr &board = getBoard(chain.getBoardId());

        if (board.info.boardFound &&
            board.config.powerOn)
        {
            mask |= 1 << spi;
        }
        else {
            chain.chainSetupDone = false;
        }
    }

    if (mask == 0)
    {
        static PollingTimer timer( 500 );
        if (timer.testAndRestart())
        {
            log("runLoop: waiting boards...\n");
        }
        return;
    }

    // setup chain
    for (uint32_t spi = 0; spi < spiNum; spi++)
    {
        if (mask & (1 << spi))
        {
            getChain(spi).setupChain();
        }
    }

    // broadcast IMAGE
    {
        static uint32_t lastTime = 0;
        if (lastTime == 0 || now - lastTime > 30*1000)
        {
            lastTime = now;
            PwrChipChain::broadcastMinerImage();
        }
    }

    // broadcast CMN CONFIG  
    {
        static uint32_t lastTime = 0;

        if (lastTime == 0 || now - lastTime > 2*1000 ||
            cmnConfigBroadcasts < 3)
        {
            lastTime = now;
            bool changed = (cmnConfigBroadcasts == 0);

			for (int id = 0; id < spiNum; id++)
			{
				PwrChipChain::broadcastCmnConfig( changed , id);
			}

            cmnConfigBroadcasts++;
        }
    }

    // broadcast JOB
    {
        static uint32_t lastTime = 0;

        if (lastTime == 0 || now - lastTime > 2*1000 ||
            miningDataBroadcasts < 3)
        {
            lastTime = now;
            bool changed = (miningDataBroadcasts == 0);

            PwrChipChain::broadcastJob( changed );

            miningDataBroadcasts++;
        }
    }

    // broadcast TIME
    {
        static uint32_t lastTime = 0;

        if (lastTime == 0 || now - lastTime > 2*1000)
        {
            lastTime = now;
            PwrChipChain::broadcastTime();
        }
    }

    // run individual chip processing
    for (uint32_t spi = 0; spi < spiNum; spi++)
    {
        if (mask & (1 << spi))
        {
            PwrChipChain &chain = getChain(spi);
            chain.downloadNonces();
            chain.serviceNextLevel();
        }
    }

    log("Time stats (spi x chips: %d x %d): %d ms\n",
        spiNum, spiLen, getMiliSeconds() - now);
}

void MultyBoardMgr::manageOcp()
{
    static PollingTimer timer( 5000 );
    if (timer.testAndRestart())
    {
        writeOcp();
        writeTmpAlert();
    }
}

void MultyBoardMgr::writeOcp()
{
    for (int boardId = 0; boardId < MAX_BOARD_PER_SLAVE; boardId++)
    {
        BoardMgr &board = getBoard(boardId);
        if (!board.info.boardFound) continue;

        uint8_t ocp = g_slaveCfg.overCurrentProtection;
        if (board.config.ocp > 0) ocp = board.config.ocp;

        log("BRD[%u]: SET OCP 0x%02x\n", boardId, ocp);
        I2CBoardAccess boardI2C(boardId);

        boardI2C.ocpReg().write( ocp );

        uint8_t actualOcp = 0xff;
        bool readOk = boardI2C.ocpReg().read( actualOcp );

        if (readOk)
        {
            board.info.overCurrentProtection = actualOcp;
        }
    }
}

void MultyBoardMgr::writeTmpAlert()
{
    for (int boardId = 0; boardId < MAX_BOARD_PER_SLAVE; boardId++)
    {
        BoardMgr &board = getBoard(boardId);
        if (!board.info.boardFound) continue;

        for (int i = 0; i < board.spec.getNumTmp(); i++)
        {
            board.writeTmpAlert(i);
        }
    }
}

void MultyBoardMgr::printInfo()
{
    static PollingTimer timer( 5000 );
    if (timer.testAndRestart())
    {
        log("\n");
        printAllChipsInfo();
        log("\n");
        printAllChipsSolInfo();
        log("\n");

        log("---------------------------------------------\n");
        printSummaryInfo();
        printSystemInfo();
        log("---------------------------------------------\n");
        log("\n");

        g_eltekMgr.printInfo();

        if (0) // DEBUG: print stats on slave
        {
            log("BTC STATS:\n");
            PwcDataArray &pda = g_masterGate.pwcDataArray;
            for (int i = 0; i < pda.len; i++)
            {
                PwcDataItem &pdi = pda.items[i];
                PwcBlock &pd = pdi.pwcData;

                for (int j = 0; j < pd.len; j++)
                {
                    Btc16StatData &btc = pd.btc16StatData[j];

                    log("PWC=%u/%u BTC=%d OSC=%2d R=%3d S=%4d E=%4d J=%4d JT=%3d CR=%4d SE=%4d JSE=%4d\n",
                        pdi.spiId, pdi.spiSeq,
                        j, btc.osc, btc.reads, btc.solutions, btc.errors,
                        btc.jobsDone, btc.lastJobTime, btc.restarts,
                        btc.stat.serErrors, btc.stat.jsErrors);
                }
            }
        }
    }
}

void MultyBoardMgr::printSummaryInfo()
{
    uint32_t totalSol = 0;
    uint32_t totalErr = 0;
    uint32_t totalJobs = 0;

    uint32_t deltaTime = getSeconds();

    log("*** POWER LINE STATS (time: %ds) ***\n", deltaTime);

    for (int boardId = 0; boardId < MAX_BOARD_PER_SLAVE; boardId++)
    {
        BoardMgr &board = getBoard(boardId);
        if (!board.info.boardFound) continue;

        for (int plId = 0; plId < board.spec.pwrNum; plId++)
        {
            uint32_t plSol = 0;
            uint32_t plErr = 0;
            uint32_t plJobs = 0;

            for (int plSeq = 0; plSeq < board.spec.pwrLen; plSeq++)
            {
                PwrChip& pwrChip = getPwrGridChip(boardId, plId, plSeq);
                plSol += pwrChip.pwcSharedData.totalSolutions;
                plErr += pwrChip.pwcSharedData.totalErrors;
                plJobs += pwrChip.pwcSharedData.totalJobsDone;
            }

            log("BRD/PWR %d/%d: S/E:%d/%d %dGH/s E:%d%% J:%d\n",
                boardId, plId, plSol, plErr,
                int((double)plSol/deltaTime*4.295),
                plSol > 0 ? int( (double)plErr/plSol*100 ) : 0,
                plJobs);

            totalSol += plSol;
            totalErr += plErr;
            totalJobs += plJobs;
        }
    }

    log("TOTAL: S/E:%d/%d E:%d%% J:%d\n",
        totalSol, totalErr,
        calcPercent(totalErr, totalSol),
        totalJobs);
}

void MultyBoardMgr::printSystemInfo()
{
    log("*** SYSTEM STATS ***\n");
    log("Sent Nonces:   %d\n", g_masterGate.foundNonces.getTotalTx());

    log("SlaveID: %u (wm: 8), TIME: %u ms\n",
        g_masterGate.masterData.slaveId,
        getMiliSeconds());

    log("UID: %s\n", McuUIDToStr(g_motherBoard.uid).str);

    log("ADC: MB VER 0x%02x, MBV %u mV (%u adc), FAN U/I %4d/%4d\n",
        g_motherBoard.mbInfo.hwVer,
        g_motherBoard.mbInfo.getMbVoltageMV(),
        g_motherBoard.mbInfo.adc50V,
        g_motherBoard.mbInfo.fan.fanAdcU,
        g_motherBoard.mbInfo.fan.fanAdcI);

    log("FAN: DAC %4u\n",
        g_slaveCfg.fanConfig.fanDac);

    for (int boardId = 0; boardId < MAX_BOARD_PER_SLAVE; boardId++)
    {
        BoardMgr &board = getBoard(boardId);

        log("BOARD[%d]: F=%d T=%2d/%2d TA=%2u(%2u)/%2u(%2u) RevADC=%4d HE=%u (%u) ON=%u PWR=%u OH N/T=%u/%u ms LOI=%u U=%u mv",
            board.boardId,
            board.info.boardFound,
            board.info.boardTemperature[0], board.info.boardTemperature[1],
            board.info.taInfo[0].alertHi, board.info.taInfo[0].numWrite,
            board.info.taInfo[1].alertHi, board.info.taInfo[1].numWrite,
            board.info.revAdc,
            board.info.heaterErr,
            board.info.heaterErrNum,
            board.config.powerOn,
            g_motherBoard.pwrSwitch[boardId].isOn,
            board.info.ohNum,
            board.info.ohTime,
            board.info.lowCurrRst,
            board.info.voltage);

        for (uint8_t i = 0; i < board.spec.pwrNum; i++)
        {
            log(" I%d=%5dma", i, board.info.currents[i]);
        }
        log("\n");
    }

    log("*** SPI TO MASTER STATS ***\n");
    log("Master ping: %u ms\n", g_masterGate.getPingTime());

    log("spiTxRx:   %d\n", g_masterGate.spiStat.spiTxRx);
    log("spiRxOk:   %d\n", g_masterGate.spiStat.spiRxOk);
    log("spiRxErr:  %d\n", g_masterGate.spiStat.spiRxErr);
}

void MultyBoardMgr::printAllChipsInfo()
{
    for (uint32_t i = 0; i < numChips; i++)
    {
        chips[i].printPwcInfo();
    }
}

void MultyBoardMgr::printAllChipsSolInfo()
{
    for (uint32_t i = 0; i < numChips; i++)
    {
        chips[i].printPwcSolInfo();
    }
}

void MultyBoardMgr::printAllChipsTestInfo()
{
    for (uint32_t i = 0; i < numChips; i++)
    {
        chips[i].printPwcTestInfo();
    }
}

BoardMgr& MultyBoardMgr::getBoard(int boardId)
{
    ASSERT(boardId < MAX_BOARD_PER_SLAVE, "");
    return boards[boardId];
}

/* max temp in all of the HSB */
int8_t MultyBoardMgr::getHSBMaxTemp(void)
{
	int8_t max = boards[0].getBoardMaxTemp();
	for (uint32_t i = 0; i < MAX_BOARD_PER_SLAVE; i++) {
		if (boards[i].getBoardMaxTemp() > max) {
			max = boards[i].getBoardMaxTemp();
		}
	}
	
    return max;
}

void MultyBoardMgr::testPowerSwitch()
{
    if (g_slaveCfg.debugOpt & SLAVE_DEBUG_TEST_POWER_SW)
    {
        static PollingTimer timer( 10*1000 );
        if (timer.testAndRestart())
        {
            static int powerEnable = 0;
            powerEnable = ((powerEnable + 1) & 1);

            log("\n\n");
            log("BOARD 0: driving power switch to: %d\n", powerEnable);

            g_motherBoard.pwrSwitch[0].set(powerEnable);

            g_slaveCfg.debugOpt |= SLAVE_DEBUG_LOG_ADC;
            g_motherBoard.loadBoardsAdc();
        }
    }
}

void MultyBoardMgr::minerRun()
{
    if (!ExchangeZone::validate()) {
        STOP();
    }

    if (1) {
        log("Memory distribution (spi x chips = %dx%d):\n", MAX_SPI_PER_SLAVE, MAX_PWC_PER_SPI);
        log("PwcBlock           = %d bytes\n", sizeof(PwcBlock));
        log("PwrChip            = %d bytes\n", sizeof(PwrChip));
        log("MultyBoardMgr      = %d bytes: %d x PwrChip\n", sizeof(MultyBoardMgr), MAX_PWC_PER_SLAVE);
        log("Btc16StatData      = %d bytes\n", sizeof(Btc16StatData));
        log("MasterGate         = %d bytes\n", sizeof(MasterGate));
        log("  tx/rx buffers    = %d bytes\n", MasterGate::getTxRxBuffersSize());
        log("  miningData       = %d bytes\n", sizeof(MasterGate::miningData));
        log("  foundNonces      = %d bytes\n", sizeof(MasterGate::foundNoncesContainer));
        log("  pwcDataArray     = %d bytes\n", sizeof(MasterGate::pwcDataArray));
        log("\n");

        log("master => slave:\n");
        log("  MasterData       = %4d bytes\n",
            sizeof(MasterData));
        log("  - SlaveConfig    = %4d bytes\n",
            sizeof(SlaveConfig));
        log("  - BoardConfig    = %4d bytes (%d x %d)\n",
            sizeof(MasterData::boardConfig), MAX_BOARD_PER_SLAVE, sizeof(BoardConfig));
        log("  - LedStates      = %4d bytes\n",
            sizeof(LedStates));
        log("  PwcMiningData    = %4d bytes\n",
            sizeof(PwcMiningData));
        log("\n");

        log("slave => master:\n");
        log("  SlaveData        = %4d bytes\n",
            sizeof(SlaveData));
        log("  PwcDataArray     = %4d bytes (%d x %d)\n",
            sizeof(PwcDataArray), PwcDataArray::MAX_LEN, sizeof(PwcDataItem));
        log("  NonceContainer   = %4d bytes (%d x %d)\n",
            sizeof(NonceContainer), NonceContainer::SIZE, sizeof(PwcNonceData));
        log("  PwcSpiArray      = %4d bytes (%d x %d)\n",
            sizeof(PwcSpiArray), PwcSpiArray::MAX_LEN, sizeof(PwcSpiData));

        size_t total = sizeof(SlaveData) + sizeof(PwcDataArray) +
                       sizeof(NonceContainer) + sizeof(PwcSpiArray);

        log("total              = %4d bytes\n", total);
        log("free               = %4d bytes\n", MS_FRAME_SIZE - total);
        log("\n");

        log("HBT master => slave:\n");
        log("  MasterData       = %4d bytes\n",
            sizeof(MasterData));
        log("  - SlaveConfig    = %4d bytes\n",
            sizeof(SlaveConfig));
        log("  - LedStates      = %4d bytes\n",
            sizeof(LedStates));
        log("\n");

        log("HBT slave => master:\n");
        log("  SlaveData        = %4d bytes\n", sizeof(SlaveData));
        log("  PwcQuickTestArr  = %4d bytes\n", sizeof(PwcQuickTestArr));

        total = sizeof(SlaveData) + sizeof(PwcQuickTestArr);

        log("total              = %4d bytes\n", total);
        log("free               = %4d bytes\n", MS_FRAME_SIZE - total);
        log("\n");
    }

	g_ModuleMgr.init();
    g_masterGate.initSpi();
    configureBoardsAuto();

    g_hbtMgr.init();

    log("Configuration:\n");
    log("  -grid size (spi x len):  %d x %d\n", spiNum, spiLen);
    log("\n");

    runTests();

    g_staticAllocator.dump();
    g_staticAllocatorCCM.dump();

    log("MINER: started");

    while(true)
    {
        //log("running main loop...\n");
        MotherBoard::resetWatchDog();

        g_masterGate.checkDataTransfer();
        activateCan();

        if (g_slaveCfg.testMode != TEST_MODE_NONE)
        {
            g_hbtMgr.poll();
        }
        else {
            manageOcp();
            updateBoardsInfo();
            processPwrSw();

            runLoop();

            testPowerSwitch();
            printInfo();
        }
    }
}
