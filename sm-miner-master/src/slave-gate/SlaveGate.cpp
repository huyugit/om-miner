#include "SlaveGate.h"

#include <stdint.h>
#include "old/cByteBufferType.h"

#include "hw/SpiTransmitter.h"
#include "hw/GpioManager.h"
#include "app/Application.h"
#include "config/Config.h"
#include "pool/StratumPool.h"
#include "ms-protocol/ms_packet.h"
#include "stats/MasterStat.h"
#include "env/EnvManager.h"
#include "base/MiscUtil.h"

#include "web-gate/webgate.h"   //chenbo add 20180109
#include "log/BflLog.h"

using namespace util;


namespace {

uint8_t txBuffer[MS_FRAME_SIZE];
uint8_t rxBuffer[MS_FRAME_SIZE];

}


SlaveGate g_slaveGate;


SlaveGate::SlaveGate()
    : numTx(0), numTxMs(0)
{
}

void SlaveGate::init()
{
}

void SlaveGate::runPollingIteration()
{
    runTxRx();
}

void SlaveGate::runTxRx()
{
    PollTimer t;
    SpiTransmitter spi;

    int rxTotal = 0;
    int rxOk = 0;

    for (int slaveId = -1; EnvManager::nextSlaveId(slaveId); )
    {
        currentSlaveId = slaveId;
        SlaveStat &slave = g_masterStat.getSlave(slaveId);

        // Set NSS via gpio
        usleep(5*1000);
        g_gpioManager.slaveSelect(slaveId, 1);

        // Prepare tx
        prepareTxBuffer(slaveId);

        // SPI txrx
        try
        {
            spi.transfer(txBuffer, rxBuffer, sizeof(rxBuffer));
        }
        catch (const Exception& e)
        {
            ::printf("ERROR: %s\n", e.what());
        }

        if (Application::config()->logSlaveLevel >= LOG_LEVEL_DEBUG)
        {
            printf("SPI TXRX: slaveId: %d\n", slaveId);
            printf("TX: "); hexdumpBeginEnd(txBuffer, sizeof(txBuffer));
            printf("RX: "); hexdumpBeginEnd(rxBuffer, sizeof(rxBuffer));
        }

        numTx++;
        slave.msSpiTx++;
        rxTotal++;

        g_gpioManager.slaveSelect(slaveId, 0);

        // Parse rx
        // TODO
        uint32_t firstWord = *(uint32_t*)rxBuffer;
        firstWord &= 0xffffff7f;
        if (firstWord != 0x00000000 && firstWord != 0xffffff7f)
        {
            if (processResponse(slaveId))
            {
                slave.msSpiRxOk++;
                rxOk++;
            }
            else {
                printf("WARNING: slave[%d]: packet ignored\n", slaveId);
                hexdumpBeginEnd(rxBuffer, sizeof(rxBuffer));
                slave.msSpiRxError++;
            }
        }
        else {
            // no activity on the line
        }
    }

    spi.close();

    numTxMs += t.elapsedMs();

    if (Application::config()->logSlaveLevel >= LOG_LEVEL_DEBUG)
    {
        printf("SlaveGate: spi rx/tx packs: %d/%d, time = %d ms\n",
               rxOk, rxTotal, (int)t.elapsedMs());
    }
}

void SlaveGate::prepareTxBuffer(int slaveId)
{
    MsPacket packet(txBuffer, sizeof(txBuffer));
    memset(txBuffer, 0, sizeof(txBuffer));

    if (Application::config()->msSpiDebug)
    {
        printf("SLAVE[%u]: generating test frame\n", slaveId);
        packet.fillTestMaster();

        return;
    }


    if (slaveId != 0xFF)
    {
        SlaveStat &slave = g_masterStat.getSlave(slaveId);

        MasterData md;
        md.time         = g_upTimer.elapsedMs();
        md.slaveId      = slaveId;
        md.slaveConfig  = Application::configRW().slaveConfig;
        md.pwcConfig    = Application::configRW().pwcConfig;
        md.ledStates    = slave.ledStates;

        for (int boardId = 0; boardId < MAX_BOARD_PER_SLAVE; boardId++)
        {
            md.boardConfig[boardId] = slave.getBoardConfig(boardId);
        }

		//chenbo add begin 20180102
		for(int fanId = 0; fanId < 6; fanId++)
		{
			md.fanStates.fan_fault[fanId] = g_fanstat.getFanFaultStat(fanId);
			md.fanStates.RPM_Speed[fanId] = g_fanstat.getFanRPMSpeed(fanId);
		}
		//chenbo add end

		//chenbo add begin 20180109
		md.hstboardOSC = g_webgate.hstboardOSC;
		g_webgate.hstboardOSC.changedFlag = 0x00;
		//chenbo add end
		
		//gezhihua add begin 20180423
		md.hstboardBin = g_webgate.hstboardBin;
		g_webgate.hstboardBin.changedFlag = 0x00;
		//gezhihua add end

		//chenbo add begin 20180123
		md.hashboardSN = g_webgate.hashSN;
		g_webgate.hashSN.changedFlag = 0x00;
		//chenbo add end

		//gzh add begin 20180126
		md.psuWorkCond = g_webgate.psuWorkCond;
		g_webgate.psuWorkCond.changedFlag = 0x00;
		//gzh add end
		
        packet.pushMsg(md);
    }


    PsuConfig &psuConfig = Application::configRW().psuConfig;
    psuConfig.masterPowerState = g_masterStat.getSlave(0).psuMgrInfo.state;
    packet.pushMsg(psuConfig);


    StratumJob currentJob;
    if (Application::pool().getCurrentJob(currentJob, 0))
    {
        PwcMiningData miningData;
        miningData.difficulty = currentJob.getDifficulty();

        miningData.extraNonce1Size = currentJob.getExtraNonce1Size();
        memcpy(miningData.extraNonce1, currentJob.getExtraNonce1(), currentJob.getExtraNonce1Size());

        miningData.extraNonce2Size = currentJob.getExtraNonce2Size();

        miningData.masterJobId = currentJob.getJobId();

        memcpy(miningData.prevHash, currentJob.getPrevHashData(), c_sha256Size);

        miningData.coinBase1Len = currentJob.getCoinbase1Size();
        memcpy(miningData.coinBase1, currentJob.getCoinbase1Data(), currentJob.getCoinbase1Size());

        miningData.coinBase2Len = currentJob.getCoinbase2Size();
        memcpy(miningData.coinBase2, currentJob.getCoinbase2Data(), currentJob.getCoinbase2Size());

        miningData.merkleRootLen = currentJob.getMerkleBranchLen();
        for (uint32_t i = 0; i < currentJob.getMerkleBranchLen(); i++)
        {
            memcpy(miningData.merkleRoot[i], currentJob.getMerkleHashData(i), c_sha256Size);
        }

        miningData.version = currentJob.getVersion();
        miningData.nBits = currentJob.getNBits();
        miningData.nTime = currentJob.getNTime();
        miningData.cleanJobs = currentJob.getCleanJobs();

        packet.pushMsg(miningData);
    }


    packet.pushEnd();
}

bool SlaveGate::processResponse(int slaveId)
{
    MsPacket packet(rxBuffer, sizeof(rxBuffer));

    if (packet.isTest()) {
        return packet.analyzeTest();
    }

    MS_DISPATCH_BEGIN(packet)
    MS_DISPATCH_MSG(packet, SlaveData)
    MS_DISPATCH_MSG(packet, SlaveHbtData)
    MS_DISPATCH_MSG(packet, NonceContainer)
    MS_DISPATCH_MSG(packet, PsuInfoArray)
    MS_DISPATCH_MSG(packet, PsuSpec)
    MS_DISPATCH_MSG(packet, PwcQuickTestArr)
    MS_DISPATCH_MSG(packet, PwcDataArray)
    MS_DISPATCH_MSG(packet, PwcSpiArray)

	/* lxj add begin 20180328 */
	MS_DISPATCH_MSG(packet, SlaveErrorArray)
	/* lxj add end */
	
    MS_DISPATCH_END(packet)
}

void SlaveGate::processMsg(SlaveData &msg)
{
    g_masterStat.getSlave(currentSlaveId).updateData(msg);
}

void SlaveGate::processMsg(SlaveHbtData &msg)
{
    g_masterStat.getSlave(currentSlaveId).updateData(msg);
}

void SlaveGate::processMsg(NonceContainer &noncesContainer)
{
    SlaveStat &slave = g_masterStat.getSlave(currentSlaveId);

    for (uint32_t i = 0; i < noncesContainer.SIZE; i++)
    {
        PwcNonceData &nonce1 = slave.noncesContainer.nonces[i];
        PwcNonceData &nonce2 = noncesContainer.nonces[i];

        if (memcmp(&nonce1, &nonce2, sizeof(PwcNonceData)) == 0)
            continue;

        if (nonce2.masterJobId == 0)
            continue;

        const StratumShare share(
            nonce2.masterJobId,
            nonce2.extraNonce2,
            nonce2.nTime,
            nonce2.nonce,
            nonce2.shareDiff);

        Application::pool().submitShare(share);
    }

    slave.noncesContainer = noncesContainer;
}

void SlaveGate::processMsg(PsuInfoArray &psuInfoArray)
{
    SlaveStat &slave = g_masterStat.getSlave(currentSlaveId);

    slave.psuMgrInfo = psuInfoArray.psuMgrInfo;
    //printf("RECEIVED psuMgrInfo: "); slave.psuMgrInfo.dump();

    slave.psuNum = psuInfoArray.len;
    for (int i = 0; i < MAX_PSU_PER_SLAVE; i++)
    {
        slave.psuInfo[i] = psuInfoArray.psuInfo[i];
        //printf("RECEIVED PSU CONFIG: "); slave.psuInfo[i].dump();
    }
}

void SlaveGate::processMsg(PsuSpec &psuSpec)
{
    SlaveStat &slave = g_masterStat.getSlave(currentSlaveId);

    if (psuSpec.id < 1 || psuSpec.id > MAX_PSU_PER_SLAVE)
    {
        printf("ERROR: unexpected psu.id=%u!\n", psuSpec.id);
        return;
    }

    slave.psuSpec[psuSpec.id - 1] = psuSpec;
}

void SlaveGate::processMsg(PwcQuickTestArr &arr)
{
    SlaveStat &slave = g_masterStat.getSlave(currentSlaveId);

    int index = 0;
    for (int boardId = 0; boardId < MAX_BOARD_PER_SLAVE; boardId++)
    {
        if (arr.itemsPerBoard[boardId] == 0) continue;

        //log("BOARD[%u]: %u chips: \n", boardId, arr.itemsPerBoard[boardId]);
        for (int i = 0; i < arr.itemsPerBoard[boardId]; i++, index++)
        {
            //log("(%02x|%04x) ", arr.items[index].pwcResult, arr.items[index].btcResults);
            slave.getPwcStat(boardId, 0, i).quickTest = arr.items[index];
        }
        //log("\n");
    }
}

void SlaveGate::processMsg(PwcDataArray &pwcDataArray)
{
    SlaveStat &slave = g_masterStat.getSlave(currentSlaveId);

    for (int i = 0; i < pwcDataArray.len; i++)
    {
        PwcDataItem &pdi = pwcDataArray.items[i];

        //pdi.dump();
        //log("   boardId = %d   spiId = %d   spiSeq = %d\n", pdi.boardId, pdi.spiId, pdi.spiSeq);

        PwcStat &pwc = slave.getPwcStat(pdi.boardId, pdi.spiId, pdi.spiSeq);
        pwc.onPwcBlock( pdi.pwcData );
    }
}

void SlaveGate::processMsg(PwcSpiArray &pwcSpiArray)
{
    SlaveStat &slave = g_masterStat.getSlave(currentSlaveId);

    //printf("received: "); pwcSpiArray.dump();

    for (int i = 0; i < pwcSpiArray.len; i++)
    {
        PwcSpiData &spiData = pwcSpiArray.items[i];
        PwcStat &pwc = slave.getPwcStat(spiData.boardId, spiData.spiId, spiData.spiSeq);

        pwc.spiData = spiData;
    }
}

/* lxj add begin 20180328 */

void SlaveGate::processMsg(SlaveErrorArray &slaveErrorArray)
{
	for (int i = 0; i < slaveErrorArray.num; i++)
	{
		gMsErrorLog.writeLog(slaveErrorArray.items[i].error);
	}
}

/* lxj add end */

