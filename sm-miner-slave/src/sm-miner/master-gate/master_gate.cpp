#include "master_gate.h"
#include "format.hpp"
#include "utils.h"
#include "spi.hpp"
#include "multy_board_mgr.h"
#include "ms_data.h"
#include "ms_packet.h"
#include "mother_board.h"
#include "board_revisions.h"
#include "sm-miner/version.h"
#include "psu/eltek_mgr.h"
#include "psu/power_mgr.h"
#include "i2c_board_access.h"
#include "pwrmodule_mgr.h"

MasterGate g_masterGate;
PsuConfig g_psuConfig;
/* lxj add begin 20180328 */
SlaveErrorArray g_slaveError;
/* lxj add end */


MasterGate::MasterGate()
    : txId(0),
      lastRxTime(0),
      foundNonces(foundNoncesContainer.nonces, foundNoncesContainer.SIZE)

{
    masterData.slaveId = 250;
    foundNoncesContainer.clear();
    miningData.initDefault();
    pwcDataArray.clear();
}

void MasterGate::initSpi()
{
    log("Init SPI to Master...\n");
    spiToMaster.init();
    log("done\n");
}

void MasterGate::checkDataTransfer()
{
    if (spiToMaster.isTransferComplete())
    {
//        log("TX: "); hexdumpBeginEnd(txBuffer, MAX_BUFFER_SIZE);
//        log("RX: "); hexdumpBeginEnd(rxBuffer, MAX_BUFFER_SIZE);

        MsPacket rxPacket(rxBuffer, sizeof(rxBuffer));
        MsPacket txPacket(txBuffer, sizeof(txBuffer));

        if (rxPacket.isTest())
        {
            txPacket.fillTestSlave(rxPacket);
        }
        else {
            onMasterBlockReceived(rxPacket);
            fillTx(txPacket);
        }
    }

    if (!spiToMaster.isTransferActive())
    {
//        log("SPI to Master: prepare for new communication\n");
        spiToMaster.txrx(rxBuffer, txBuffer, sizeof(rxBuffer));
    }
}


void MasterGate::onMasterBlockReceived(MsPacket &packet)
{
    spiStat.spiTxRx++;

    if (processRx(packet)) {
        spiStat.spiRxOk++;
    }
    else {
        log("PACKET ERROR from MASTER:\n");
        hexdumpBeginEnd((uint8_t*)packet.ptrSrc, packet.sizeSrc);

        spiStat.spiRxErr++;
    }
}

bool MasterGate::processRx(MsPacket &packet)
{
    MS_DISPATCH_BEGIN(packet)
    MS_DISPATCH_MSG(packet, MasterData)
    MS_DISPATCH_MSG(packet, PsuConfig)
    MS_DISPATCH_MSG(packet, PwcMiningData)
    MS_DISPATCH_END(packet)
}

void MasterGate::fillTx(MsPacket &packet)
{
    txId++;

    // --- Slave & Boards
    pushSlaveData(packet);

	/* lxj add begin 20180328 */
	{
		SlaveErrorArray slaveError; 
		fillSlaveErrorArray(slaveError);
		packet.pushMsg(slaveError);
	}
	/* lxj add end */

    // --- PSU info
   	if(g_ModuleMgr.pModuleMgr != nullptr)
	{
	    PsuInfoArray pia;
	    fillPsuInfo(pia);
	    packet.pushMsg(pia);
	}

    if (g_slaveCfg.testMode != TEST_MODE_NONE)
    {
        pushSlaveTestData(packet);
        pushPwcQuickTestArr(packet);
        packet.pushEnd();
        return;
    }

    // --- PWC stats

    // adjust data
    for (int i = 0; i < pwcDataArray.len; i++)
    {
        PwcDataItem &item = pwcDataArray.items[i];
        PwrChip &chip = g_multyBoardMgr.getSpiGridChip(item.boardId, item.spiId, item.spiSeq);

        // mark chip as stat data were sent
        chip.toMasterTxId = txId;
    }

    packet.pushMsg(pwcDataArray);

    // --- Nonces
    packet.pushMsg(foundNoncesContainer);

    // --- PSU spec
	if(g_ModuleMgr.pModuleMgr != nullptr)
	{
		if (g_ModuleMgr.pModuleMgr->getNumRectifiers() > 0)
	    {
	        static uint8_t id = 0xff;
	        id++;
	        if (id >= g_ModuleMgr.pModuleMgr->getNumRectifiers()) id = 0;
			PsuSpec psuSpec;
			PsuSpec *pPsuSpec = g_ModuleMgr.pModuleMgr->getRectifierSpecByIndex(id);
			memcpy((char *)(&psuSpec), (char *)pPsuSpec, sizeof(PsuSpec));
			packet.pushMsg(psuSpec);
	    }
	}

    // --- SPI stats
    PwcSpiArray pwcSpiArray;
    fillPwcSpiArray(pwcSpiArray);
    packet.pushMsg(pwcSpiArray);

    packet.pushEnd();
}

void MasterGate::processMsg(MasterData &md)
{
    lastRxTime = getMiliSeconds();
	
    MasterData oldMasterData = masterData;
    masterData = md;

    // Slave Config
    if (memcmp(&g_slaveCfg, &masterData.slaveConfig, sizeof(masterData.slaveConfig)) != 0)
    {
        //log("New slaveConfig received: "); masterData.slaveConfig.dump();

        // save current config for changes detection
        SlaveConfig oldConfig = g_slaveCfg;
        g_slaveCfg = masterData.slaveConfig;

        if (g_slaveCfg.userSlotMask   != oldConfig.userSlotMask ||
            g_slaveCfg.userBrdRev     != oldConfig.userBrdRev ||
            g_slaveCfg.userBrdIfFound != oldConfig.userBrdIfFound ||
            g_slaveCfg.userBrdSpiNum  != oldConfig.userBrdSpiNum ||
            g_slaveCfg.userBrdSpiLen  != oldConfig.userBrdSpiLen ||
            g_slaveCfg.userBrdSpiMask != oldConfig.userBrdSpiMask ||
            g_slaveCfg.userBrdPwrNum  != oldConfig.userBrdPwrNum ||
            g_slaveCfg.userBrdPwrLen  != oldConfig.userBrdPwrLen ||
            g_slaveCfg.userBrdBtcNum  != oldConfig.userBrdBtcNum ||
            g_slaveCfg.userBrdBtcMask != oldConfig.userBrdBtcMask)
        {
            log("Reconfigure grid due to changes in config!\n");

            if (g_slaveCfg.userBrdRev == BoardRevisions::REV_ID_AUTO)
            {
                g_multyBoardMgr.configureBoardsAuto();
            }
            else
            {
                uint8_t revId = BoardRevisions::REV_ID_DEFAULT;

                // override default with user settings
                if (g_slaveCfg.userBrdRev > 0) revId = g_slaveCfg.userBrdRev;

                // load base board spec
                RevisionData *rev = BoardRevisions::getRevByRevId(revId);

                if (rev)
                {
                    BoardSpec spec = rev->spec;
                    spec.revisionId = BoardRevisions::REV_ID_MANUAL_CONFIG;

                    if (g_slaveCfg.userBrdSpiNum  > 0) spec.spiNum  = g_slaveCfg.userBrdSpiNum;
                    if (g_slaveCfg.userBrdSpiLen  > 0) spec.spiLen  = g_slaveCfg.userBrdSpiLen;
                    if (g_slaveCfg.userBrdSpiMask > 0) spec.spiMask = g_slaveCfg.userBrdSpiMask;
                    if (g_slaveCfg.userBrdPwrNum  > 0) spec.pwrNum  = g_slaveCfg.userBrdPwrNum;
                    if (g_slaveCfg.userBrdPwrLen  > 0) spec.pwrLen  = g_slaveCfg.userBrdPwrLen;
                    if (g_slaveCfg.userBrdBtcNum  > 0) spec.btcNum  = g_slaveCfg.userBrdBtcNum;
                    if (g_slaveCfg.userBrdBtcMask > 0) spec.btcMask = g_slaveCfg.userBrdBtcMask;

                    g_multyBoardMgr.configureBoardsBySpec(g_slaveCfg.userSlotMask, spec);
                }
                else {
                    log("ERROR: can not find board spec with revision id = %d!\n", revId);
                }
            }
        }

        g_motherBoard.setupFan(g_slaveCfg.fanConfig);
    }

    // Board Config
    for (int boardId = 0; boardId < MAX_BOARD_PER_SLAVE; boardId++)
    {
        g_multyBoardMgr.getBoard(boardId).onBoardConfig(masterData.boardConfig[boardId]);
    }

    // PWC Config
    {
        PwcCmnConfig &c1 = oldMasterData.pwcConfig;
        PwcCmnConfig &c2 = masterData.pwcConfig;

        if (memcmp(&c1, &c2, sizeof(c1)) != 0)
        {
            log("New pwcConfig received: "); masterData.pwcConfig.dump();
            g_multyBoardMgr.cmnConfigBroadcasts = 0;
        }
    }

	//chenbo add begin 20180109
    {
		if(masterData.hstboardOSC.changedFlag)
		{
			if(masterData.hstboardOSC.changedFlag == 0xff){
				/* Power down the hash board, add by gezhihua 20180308, 
				   today i am not happy for the bad remember has been remind by someone else:( */
				g_multyBoardMgr.cmnConfigBroadcasts = 0;
				g_multyBoardMgr.hashBoardPowerDownFlag = 1;
			}else{
				for(int i = 0; i < MAX_BOARD_PER_SLAVE; i++)
				{
					if(masterData.hstboardOSC.changedFlag & (0x01 << i))
						g_multyBoardMgr.getBoard(i).setBoardOSCconfig(masterData.hstboardOSC.HashBoardOsc[i]);
				}
				g_multyBoardMgr.cmnConfigBroadcasts = 0;
				g_multyBoardMgr.hashBoardPowerDownFlag = 0;
			}
		}	
	}
	//chenbo add end

	//chenbo add begin 20180123
	if(masterData.hashboardSN.changedFlag)
	{
		for(int i = 0; i < MAX_BOARD_PER_SLAVE; i++)
		{
			if(masterData.hashboardSN.changedFlag & (0x01 << i))
				g_multyBoardMgr.getBoard(i).setBoardSNforHash(masterData.hashboardSN.HashBoardSN[i]);
		}
		g_multyBoardMgr.cmnConfigBroadcasts = 0;
	}
	//chenbo add end
	
	//gezhihua add begin 20180423
	if(masterData.hstboardBin.changedFlag)
	{
		for(int i = 0; i < MAX_BOARD_PER_SLAVE; i++)
		{
			if(masterData.hstboardBin.changedFlag & (0x01 << i))
				g_multyBoardMgr.getBoard(i).setBoardBinforHash(masterData.hstboardBin.HashBoardBin[i]);
		}
		g_multyBoardMgr.cmnConfigBroadcasts = 0;
	}
	//gezhihua add end
	
	//gzh add begin 20180125
	if(masterData.psuWorkCond.changedFlag){
		g_ModuleMgr.pHBWorkCond->PsuPoutLowTh = masterData.psuWorkCond.PsuPoutLowTh;
		g_ModuleMgr.pHBWorkCond->PsuPoutHighTh = masterData.psuWorkCond.PsuPoutHighTh;
		g_ModuleMgr.pHBWorkCond->PsuIoutLowTh = masterData.psuWorkCond.PsuIoutLowTh;
		g_ModuleMgr.pHBWorkCond->PsuIoutHighTh = masterData.psuWorkCond.PsuIoutHighTh;
		g_ModuleMgr.pHBWorkCond->PsuVoutHighTh = masterData.psuWorkCond.PsuVoutHighTh;
		g_ModuleMgr.pHBWorkCond->PsuFanSpeedLowTh = masterData.psuWorkCond.PsuFanSpeedLowTh;
		g_ModuleMgr.pHBWorkCond->PsuTempOutHighTh = masterData.psuWorkCond.PsuTempOutHighTh;
		g_ModuleMgr.pHBWorkCond->FanRpmLowTh = masterData.psuWorkCond.FanRpmLowTh;
		g_ModuleMgr.pHBWorkCond->FanFlag = masterData.psuWorkCond.FanFlag;
		g_ModuleMgr.pHBWorkCond->HashTempFlag = masterData.psuWorkCond.HashTempFlag;
		if(g_ModuleMgr.pHBWorkCond->HashTempHi != masterData.psuWorkCond.HashTempHi){
			g_ModuleMgr.pHBWorkCond->HashTempHi = masterData.psuWorkCond.HashTempHi;
			g_slaveCfg.maxTempHi = masterData.psuWorkCond.HashTempHi;
			g_multyBoardMgr.writeTmpAlert();
		}
		g_ModuleMgr.pHBWorkCond->PsuFlag = masterData.psuWorkCond.PsuFlag;
		g_ModuleMgr.pHBWorkCond->FanWorkMode = masterData.psuWorkCond.FanWorkMode;
	}
	
	//gzh add end
    // LEDs
    {
        g_motherBoard.greenLed.setBlinkType( masterData.ledStates.greenLedState );
        g_motherBoard.redLed.setBlinkType( masterData.ledStates.redLedState );

        for (int i = 0; i < MAX_BOARD_PER_SLAVE; i++)
        {
            g_motherBoard.boardLeds[i].setBlinkType( masterData.ledStates.boardLedState[i] );
            g_motherBoard.boardLeds[i].setColor( masterData.ledStates.boardLedColor[i] );
        }
    }
}

void MasterGate::processMsg(PsuConfig &pc)
{
    g_psuConfig = pc;
}

void MasterGate::processMsg(PwcMiningData &md)
{
    if (miningData.masterJobId != md.masterJobId)
    {
        g_multyBoardMgr.miningDataBroadcasts = 0;
        memcpy(&miningData, &md, sizeof(md));

        log("New MiningData received: "); miningData.dump();
    }
}

void MasterGate::pushSlaveData(MsPacket &packet)
{
    SlaveData sd;
    sd.cmnInfo.uid              = g_motherBoard.uid;
    sd.cmnInfo.swVersion        = swVersion;
    sd.cmnInfo.totalTime        = getSeconds();
    sd.cmnInfo.loopbackTime     = masterData.time;
    sd.cmnInfo.slaveSpiStat     = spiStat;
    sd.cmnInfo.mbInfo           = g_motherBoard.mbInfo;

    for (int id = 0; id < MAX_BOARD_PER_SLAVE; id++)
    {
        BoardData &boardData = sd.boardData[id];
        BoardMgr &board = g_multyBoardMgr.getBoard(id);

        boardData.info = board.info;
        boardData.spec = board.spec;
    }

    packet.pushMsg(sd);
}

void MasterGate::pushSlaveTestData(MsPacket &packet)
{
    SlaveHbtData msg;
    msg.testInfo = testInfo;

    for (int id = 0; id < MAX_BOARD_PER_SLAVE; id++)
    {
        msg.boardTest[id] = g_multyBoardMgr.getBoard(id).test;
    }

    packet.pushMsg(msg);
}

void MasterGate::fillPsuInfo(PsuInfoArray &pia)
{
    AtomicBlock ab;
    pia.psuMgrInfo = g_powerMgr.info;

    pia.len = g_ModuleMgr.pModuleMgr->getNumRectifiers();
    for (int i = 0; i < pia.len; i++)
    {
        PsuInfo *pPsuInfo = g_ModuleMgr.pModuleMgr->getRectifierInforByIndex(i);
		memcpy((char *)(&pia.psuInfo[i]), (char *)pPsuInfo, sizeof(PsuInfo));
    }
}

void MasterGate::pushPwcQuickTestArr(MsPacket &packet)
{
    PwcQuickTestArr arr;
    arr.clear();

    int totalChips = 0;
    for (int boardId = 0; boardId < MAX_BOARD_PER_SLAVE; boardId++)
    {
        BoardMgr &board = g_multyBoardMgr.getBoard(boardId);
        if (!board.info.boardFound) continue;

        int boardChips = board.spec.spiNum * board.spec.spiLen;
        if (totalChips + boardChips > MAX_PWC_PER_SLAVE) break;

        arr.itemsPerBoard[boardId] = boardChips;

        for (int i = 0; i < boardChips; i++, totalChips++)
        {
            PwrChip &chip = g_multyBoardMgr.getBoardChip(boardId, i);
            arr.items[totalChips] = chip.testRes;
        }
    }

    packet.pushMsg(arr);
}

void MasterGate::fillPwcSpiArray(PwcSpiArray &arr)
{
    static uint32_t currentIndex = 0;

    arr.len = MIN(arr.MAX_LEN, g_multyBoardMgr.numChips);

    for (int i = 0; i < arr.len; i++, currentIndex++)
    {
        if (currentIndex >= g_multyBoardMgr.numChips)
        {
            currentIndex = 0;
        }

        PwrChip &chip = g_multyBoardMgr.getChip(currentIndex);
        PwrChipChain &chain = chip.getChain();

        PwcSpiData &item = arr.items[i];
        PwcSpiData &itemSrc = chip.spiData;

        item = itemSrc;

        item.boardId    = chip.boardId;
        item.spiId      = chain.getBoardSpiId();
        item.spiSeq     = chip.spiSeq;
    }
}

/* lxj add begin 20180328 */
void MasterGate::fillSlaveErrorArray(SlaveErrorArray &arr)
{
    static uint32_t currentIndex = 0;

	arr.num = g_slaveError.num;
	
    for (int i = 0; i < arr.num; i++, currentIndex++)
    {
        SlaveErrorData &item = arr.items[i];
		item.error = g_slaveError.items[i].error;
    }

	g_slaveError.num = 0;	// reset
}

/* lxj add end */


void MasterGate::sendNonce(const PwcNonceData &nd)
{
    if (!foundNonces.contains(nd))
    {
        if (g_slaveCfg.mcuLogStratum)
        {
            log("stratum sending: "); nd.dump();
        }

        foundNonces.add(nd);
        foundNoncesContainer.numSent++;
    }
    else {
        // duplicate received
        foundNoncesContainer.numDrop++;
    }
}

void MasterGate::sendPwcData(PwrChip &chip, const PwcBlock &pwcData)
{
    // Each pwc data is related to some power chip. When new data is loaded,
    // it should be transfered to the master. For this purpose we have buffer
    // with limited space. If buffer is full, then we should replace data
    // for chip with highest tx id.

    uint32_t highestTxIdIndex = 0;
    uint32_t highestTxId = 0;

    for (int i = 0; i < pwcDataArray.len; i++)
    {
        PwcDataItem &item2 = pwcDataArray.items[i];
        PwrChip &chip2 = g_multyBoardMgr.getSpiGridChip(item2.boardId, item2.spiId, item2.spiSeq);

        if (chip.spiId == chip2.spiId && chip.spiSeq == chip2.spiSeq)
        {
//            log("sendPwcData(spi/seq = %d/%d): position %d - the same chip\n",
//                chip.spiId, chip.spiSeq, i);

            sendPwcData(chip, pwcData, i);
            return;
        }

        if (chip2.toMasterTxId > highestTxId)
        {
            highestTxIdIndex = i;
            highestTxId = chip2.toMasterTxId;
        }
    }

    if (pwcDataArray.len < pwcDataArray.MAX_LEN)
    {
//        log("sendPwcData(spi/seq = %d/%d): position %d - add at the end\n",
//            chip.spiId, chip.spiSeq, array.len);

        sendPwcData(chip, pwcData, pwcDataArray.len++);
    }
    else {
        if (chip.toMasterTxId < highestTxId)
        {
//            log("sendPwcData(spi/seq = %d/%d): position %d - replacing higher tx id\n",
//                chip.spiId, chip.spiSeq, highestTxIdIndex);

            sendPwcData(chip, pwcData, highestTxIdIndex);
        }
        else {
//            log("sendPwcData(spi/seq = %d/%d): buffer is full! nothing to replace!\n",
//                chip.spiId, chip.spiSeq);
        }
    }
}

void MasterGate::sendPwcData(PwrChip &chip, const PwcBlock &pwcData, uint8_t index)
{
    PwcDataItem &item = pwcDataArray.items[index];

    item.boardId = chip.getChain().getBoardId();
    item.spiId   = chip.getChain().getBoardSpiId();
    item.spiSeq  = chip.spiSeq;

    item.pwcData = pwcData;
}

uint32_t MasterGate::getPingTime()
{
    uint32_t now = getMiliSeconds();
    return (lastRxTime == 0 || now < lastRxTime) ? 666666 : now - lastRxTime;
}

void MasterGate::emulateOnMiningData()
{
    log("Emulating OnMiningData\n");
    uint32_t tBegin = getMiliSeconds();

    memset(rxBuffer, 0, sizeof(rxBuffer));
    MsPacket packet(rxBuffer, sizeof(rxBuffer));

    {
        MasterData masterDataLocal;
        masterDataLocal.slaveId = 0;
        masterDataLocal.time = 0x12345678;

        memset(&masterDataLocal.ledStates, 0, sizeof(masterDataLocal.ledStates));

        packet.pushMsg(masterDataLocal);
    }
    {
        PwcMiningData miningDataLocal;
        miningDataLocal.difficulty = 300;

        miningDataLocal.extraNonce1Size = 4;
        *(uint32_t*)miningDataLocal.extraNonce1 = 0x11223344;
        miningDataLocal.extraNonce2Size = 4;

        static int masterJobId = 100;
        masterJobId++;

        miningDataLocal.masterJobId = masterJobId;
        miningDataLocal.version = 0x00000002;
        miningDataLocal.nBits = 0x1901f52c;
        miningDataLocal.nTime = 0x52e8ca2d;
        miningDataLocal.cleanJobs = false;

        packet.pushMsg(miningDataLocal);
    }

    packet.pushEnd();

    uint32_t tEnd = getMiliSeconds();
    log("Time stats: master block generated in %d ms\n", tEnd - tBegin);

    //log("Generated master block:\n"); hexdump(buffer, mb.getSize());

    onMasterBlockReceived(packet);
}
