#include "pwr_chip_chain.h"

#include "pwc_defines.h"
#include "pwr_chip_bin.h"
#include "pwr_chip_packet.h"
#include "pwr_chip_rx_packet.h"
#include "stm_multi_spi.h"
#include "stm_gpio.h"
#include "uart.hpp"
#include "master_gate.h"
#include "multy_board_mgr.h"
#include "utils.h"
#include "exchange_zone.h"
#include "exchange_zone_bt.h"
#include "exchange_zone_mt.h"


ExchangeZone &g_ez = *(ExchangeZone*)0x00000020;

const uint8_t PwrChipChain::ALL_SPI = MultySPI::ALL_SPI;

PwrChipChain::PwrChipChain()
    : debugOn(false),
      boardId(0), 
      boardSpiId(0),
      serviceSeq(0)
{
}

PwrChip &PwrChipChain::getChip(uint8_t seq)
{
    return g_multyBoardMgr.getSpiGridChip(boardId, boardSpiId, seq);
}

void PwrChipChain::init(uint8_t _spiId, uint8_t _spiLen,
                        uint8_t _boardId, uint8_t _boardSpiId)
{
    spiId       = _spiId;
    spiLen      = _spiLen;
    boardId     = _boardId;
    boardSpiId  = _boardSpiId;

    chainSetupDone = false;
    serviceSeq = 0;

    switch(g_multyBoardMgr.getBoard(boardId).info.revAdc)
    {
    case 25: parritySchema = 1; break;
    default: parritySchema = 0; break;
    }

    for (uint32_t seq = 0; seq < spiLen; seq++)
    {
        PwrChip &chip = getChip(seq);
        chip.spiId      = spiId;
        chip.spiSeq     = seq;
    }
}

void PwrChipChain::setupChain()
{
    if (!chainSetupDone)
    {
        const int attempts = 5;

        log("---------------------------------------------\n");
        log("SETUP CHAIN: %d, len %d\n", spiId, spiLen);
        log("---------------------------------------------\n");

        {
            resetLine();

            uint8_t flags = PwrChipPacket::A_ALL;

            for (int kkk = 0; kkk < 3; kkk++)
            {
                //log("[packet] BROADCAST CHIP RESET HI: ");
                for (int i = 0; i < attempts; i++)
                {
                    writeReg(flags, PwrChipPacket::ADDR_SYS_RESET, 0x00000000);
                }

                //log("[packet] BROADCAST CHIP RESET LO: ");
                for (int i = 0; i < attempts; i++) // TODO
                {
                    writeReg(flags, PwrChipPacket::ADDR_SYS_RESET, PwrChipPacket::CHIP_RESET_MAGIC);
                }
            }
        }

        while (0)
        {
            gotoFirstChip();

            //uint8_t flags = seqToAddr(0);
            uint8_t flags = PwrChipPacket::A_ALL;

            uint32_t vendorId = 0xeeeeeeee;
            PwcReadResult r = readReg(flags, PwrChipPacket::ADDR_CHIP_VENDOR_ID, vendorId);

            log("CHIP[%2d/%d]: r=%u, VendorID = 0x%08x\n",
                currentSeq, 0, r, vendorId);

            mysleep(500);
        }

        for (gotoFirstChip(); currentSeq < spiLen; gotoNextChip())
        {
            uint8_t flags = seqToAddr(currentSeq);

            PwrChip &chip = getChip(currentSeq);
            chip.spiData.found = false;

            uint32_t vendorId = 0;
            for (uint32_t attempt = 0 ; attempt < attempts; attempt++)
            {
                // read vendor id register to determine if chip is preset
                if (readReg(flags, PwrChipPacket::ADDR_CHIP_VENDOR_ID, vendorId) == PWC_READ_OK)
                {
                    chip.spiData.found = true;
                    break;
                }
            }

            uint32_t data = 0;
            for (uint32_t attempt = 0 ; attempt < attempts; attempt++)
            {
                if (readReg(flags, 0x0000fff0, data) == PWC_READ_OK)
                {
                    break;
                }
            }

            log("CHIP[%2d]: VendorID = 0x%08x, MEM 0xfff0 = 0x%08x\n", currentSeq, vendorId, data);

            uint8_t x = 250;
            //uint8_t x = 3;
            //uint8_t x = 4;

            if (currentSeq == x)
            {
                log("CHIP[%d] SPI CONFIG: FWD_DISABLE\n", currentSeq);
                for (uint32_t attempt = 0 ; attempt < attempts; attempt++)
                {
                    writeReg(flags, PwrChipPacket::ADDR_SPI_CFG, PwrChipPacket::SPI_CFG_FWD_DISABLE);
                }
            }
            if (currentSeq == x + 1)
            {
                log("CHIP[%d] SPI CONFIG: FWD_LATCHED\n", currentSeq);
                for (uint32_t attempt = 0 ; attempt < attempts; attempt++)
                {
                    writeReg(flags, PwrChipPacket::ADDR_SPI_CFG, PwrChipPacket::SPI_CFG_FWD_LATCHED);
                }
            }
        }

        chainSetupDone = true;

        //while (1) { testChain(); mysleep(1000); }

        memTest();
        //testChain();
    }
}

void PwrChipChain::testChain()
{
    log("---------------------------------------------\n");
    log("TEST CHAIN: %d, len %d\n", spiId, spiLen);
    log("---------------------------------------------\n");

//    while (1)
//    {
//        log("reset line...\n");
//        resetLine();

//        mysleep(500);
//    }

    while (0)
    {
        gotoFirstChip();
        mysleep(1);
        sendNext( seqToAddr(currentSeq) ); currentSeq++;
        sendNext( seqToAddr(currentSeq) ); currentSeq++;
        mysleep(1);

//        g_spiExchange.debugOn = true;
//        g_spiExchange.debugBits = true;

        int i = 0;
//        int i = 1;
//        int i = 2;
        //for (int k = 0; k < 20; k++)
        //for (int i = 0; i < 4; i++)
        {
            uint8_t flags = seqToAddr(currentSeq);
//            uint8_t flags = seqToAddr(currentSeq+1);

            uint32_t vendorId = 0xeeeeeeee;
            PwcReadResult r = readReg(flags, PwrChipPacket::ADDR_CHIP_VENDOR_ID, vendorId);

            log("T=%u: CHIP[%2d/%d]: r=%u, VendorID = 0x%08x\n",
                getMiliSeconds(), currentSeq, i, r, vendorId);
        }
        log("\n");

        mysleep(200);
    }

    if (1)
    for (gotoFirstChip(); currentSeq < spiLen; gotoNextChip())
    {
        //for (int k = 0; k < 2; k++)
        for (int i = 0; i < 4; i++)
        {
//            bool f = (currentSeq == 3);
//            g_spiExchange.debugOn = f;
//            g_spiExchange.debugBits = f;

            //uint8_t flags = seqToAddr(i);
            uint8_t flags = PwrChipPacket::A_0 + i;

//            if (currentSeq == 3 || currentSeq == 4)
//            {
//                flags = seqToAddr(3 - i);
//            }

//            int n = (currentSeq == 3 && i == 3 ? 5 : 1);
//            for (int j = 0; j < n; j++)
            {
                static const uint32_t NUM_REGS = 6;

                static uint32_t regAddr[NUM_REGS] =
                {
                    PwrChipPacket::ADDR_CHIP_VENDOR_ID,
                    PwrChipPacket::ADDR_SPI_COND_FLAGS,
                    0x0000,
                    0x0020,
                    0x0024,
                    0x7ff0
                };

                static uint32_t regValue[NUM_REGS];
                static uint32_t regNumR[NUM_REGS];

                for (uint32_t regIndex = 0; regIndex < NUM_REGS; regIndex++)
                {
                    regValue[regIndex] = 0xeeeeeeee;
                    regNumR[regIndex] = 0;

                    //for (int regAttempt = 0; regAttempt < 5; regAttempt++)
                    while (regNumR[regIndex] < 5)
                    {
                        regNumR[regIndex]++;

                        PwcReadResult r = readReg(flags, regAddr[regIndex], regValue[regIndex]);
                        if (r == PWC_READ_OK) break;
                    }
                }

                log("CHIP[%2d/%d]:", currentSeq, i);
                for (uint32_t regIndex = 0; regIndex < NUM_REGS; regIndex++)
                {
                    log(" [0x%02x]=0x%08x(%u)", regAddr[regIndex], regValue[regIndex], regNumR[regIndex]);
                }
                log("\n");
            }
        }
        log("\n");

//        sendNext( PwrChipPacket::A_ALL );
//        currentSeq++;
    }

    if (0)
    for (gotoFirstChip(); currentSeq < spiLen; gotoNextChip())
    {
        for (int i = 0; i < 4; i++)
        {
            uint8_t flags = seqToAddr(i);

            uint32_t vendorId = 0xeeeeeeee;
            PwcReadResult r = readReg(flags, PwrChipPacket::ADDR_CHIP_VENDOR_ID, vendorId);

            uint32_t d1 = 0xeeeeeeee;
            readReg(flags, 0x0000, d1);

            uint32_t d2 = 0xeeeeeeee;
            readReg(flags, 0x0020, d2);

            uint32_t d3 = 0xeeeeeeee;
            readReg(flags, 0x0024, d3);

            uint32_t d4 = 0xeeeeeeee;
            readReg(flags, 0x7ff0, d4);

            log("CHIP[%2d/%d]: r=%u, VendorID = 0x%08x, [0x00] 0x%08x, [0x20] 0x%08x, [0x24] 0x%08x, [0xfff0] 0x%08x\n",
                currentSeq, i, r, vendorId, d1, d2, d3, d4);
        }
        log("\n");
    }
}

void PwrChipChain::memTest()
{
    log("---------------------------------------------\n");
    log("MEMORY TEST: SPI %d, len %d\n", spiId, spiLen);
    log("---------------------------------------------\n");

//    for (gotoFirstChip(); currentSeq < spiLen; gotoNextChip())
//    {
//        uint8_t flags = seqToAddr(currentSeq);
//        if (currentSeq < 5) return;

//        for (int bit = 0; bit < /*32*/2; bit++)
//        {
//            //uint32_t data = 1 << bit;
//            uint32_t data = (bit == 0 ? 0x00000000 : 0xffffffff);
//            uint32_t data2;

//            log("CHIP[%2d]: MEMORY TEST data = 0x%08x\n", currentSeq, data);

//            for (uint32_t addr = 0; addr < 0xffff; addr+=4)
//            {
//                write(flags, (void*)addr, sizeof(data), &data);
//                read(flags, (void*)addr, sizeof(data2), &data2);

//                if (data != data2)
//                {
//                    log("0x%08x: 0x%08x != 0x%08x\n", addr, data, data2);
//                }
//            }
//        }
//    }

    broadcastMemTestImage();

    uint32_t t0 = getMiliSeconds();
    int steps = 0;
    while (getMiliSeconds() - t0 < 700 || steps < 5)
    {
        steps++;
        bool testDone = true;

        for (gotoFirstChip(); currentSeq < spiLen; gotoNextChip())
        {
            PwrChip &chip = getChip(currentSeq);
            uint8_t flags = seqToAddr(currentSeq);

            if (chip.spiData.pwcTest != PWC_TEST_NA)
            {
                // test already done for this chip
                continue;
            }

            ExchangeZoneMt ez;
            if (read(flags, (void*)ExchangeZoneMt::ADDR, sizeof(ez), &ez) != PWC_READ_OK)
            {
                log("CHIP[%2d]: can not read exchange zone!\n", currentSeq);
            }
            else {
                bool markerOk = (ez.marker == ExchangeZoneMt::MARKER_DONE);
                bool crcOk = (ez.crc32 == ez.calcCrc32());

                log("CHIP[%2d]: FLAGS M/C=%u%u M=0x%08x OK=0x%08x CNT=0x%04x EW/EB=%u/%u\n",
                    currentSeq, markerOk, crcOk, ez.marker, ez.result, ez.counter, ez.errWords, ez.errBits);

                if (markerOk && crcOk)
                {
                    switch (ez.result)
                    {
                    case ExchangeZoneMt::RESULT_OK:  chip.spiData.pwcTest = PWC_TEST_OK;      break;
                    case ExchangeZoneMt::RESULT_ERR: chip.spiData.pwcTest = PWC_TEST_MEM_ERR; break;
                    }
                }
            }

            if (chip.spiData.pwcTest == PWC_TEST_NA)
                testDone = false;

            chip.testRes.pwcResult = chip.spiData.pwcTest;
        }

        log("---------------------------------------------\n");

        if (testDone)
            break;
    }
}

void PwrChipChain::broadcastMinerImage()
{
    static bool firstLoad = true;

    bool conditional = true;
    int imageAttempts = 1;

    if (firstLoad)
    {
        conditional = false;
        imageAttempts = 1;
    }

    broadcastImage(g_pwr_chip_bin, &g_end_pwr_chip_bin, conditional, imageAttempts);

    firstLoad = false;
}

void PwrChipChain::broadcastMemTestImage()
{
    broadcastImage(g_pwr_chip_mt_bin, &g_end_pwr_chip_mt_bin, false, 1);
}

void PwrChipChain::broadcastBtcTestImage()
{
    broadcastImage(g_pwr_chip_bt_bin, &g_end_pwr_chip_bt_bin, false, 1);
}

void PwrChipChain::broadcastImage(const void *imageBegin, const void *imageEnd,
                                  bool conditional, int imageAttempts)
{
    const uint32_t t0 = getMiliSeconds();
    const int attempts = 3;

    uint8_t flags = PwrChipPacket::A_ALL;

    if (conditional)
    {
        flags |= PwrChipPacket::condIfNot(SPI_FLAG_PWC_ALIVE);
    }

    log("BROADCAST: CHIP RESET\n");
    for (int i = 0; i < attempts; i++)
    {
        resetLineToSpi(ALL_SPI);
        writeRegToSpi(ALL_SPI, flags, PwrChipPacket::ADDR_SYS_RESET, 0x00000000);
    }
    for (int i = 0; i < attempts; i++)
    {
        resetLineToSpi(ALL_SPI);
        writeRegToSpi(ALL_SPI, flags, PwrChipPacket::ADDR_SYS_RESET, PwrChipPacket::CHIP_RESET_MAGIC);
    }


    log("BROADCAST: LOAD IMAGE\n");

    uint8_t *ptrBegin = (uint8_t*)imageBegin;
    uint8_t *ptrEnd = (uint8_t*)imageEnd + 4;

    while ((ptrEnd - ptrBegin) % 4) {
        ptrEnd--;
    }

    //log("Image size: %d bytes\n", ptrEnd - ptrBegin);
    //hexdumpBeginEnd(ptrBegin, ptrEnd - ptrBegin);

    for (int i = 0; i < imageAttempts; i++)
    {
        resetLineToSpi(ALL_SPI);

        const uint32_t BLOCK_SIZE = 3*1024;

        uint8_t *ptr = ptrBegin;
        while (ptr < ptrEnd)
        {
            uint32_t size = ptrEnd - ptr;
            if (size > BLOCK_SIZE) size = BLOCK_SIZE;

            log("WRITE: size = %d\n", size);
            writeToSpi(ALL_SPI, flags, (void*)(ptr - ptrBegin), size, ptr);

            ptr += size;
        }
    }

    flags = PwrChipPacket::A_ALL;

    log("BROADCAST: CPU CLOCK CONFIG: 0x%04x\n", g_slaveCfg.pwcClockCfg);
    for (int i = 0; i < attempts; i++)
    {
        resetLineToSpi(ALL_SPI);
        writeRegToSpi(ALL_SPI, flags, PwrChipPacket::ADDR_SYS_CLK_CFG, g_slaveCfg.pwcClockCfg);
    }

    broadcastCpuEnable(attempts);

    log("BROADCAST: CLEAR PWC ALIVE FLAG\n");
    for (int i = 0; i < attempts; i++)
    {
        resetLineToSpi(ALL_SPI);
        writeRegToSpi(ALL_SPI, flags, PwrChipPacket::ADDR_SPI_COND_FLAGS_CLEAR, 1 << SPI_FLAG_PWC_ALIVE);
    }

    uint32_t dt = getMiliSeconds() - t0;
    log("BROADCAST: done in %d ms, %d iterations\n", dt, imageAttempts);
}

void PwrChipChain::broadcastTime()
{
    uint32_t ms = getMiliSeconds();
    log("BROADCAST: TIME %d\n", ms);

    uint8_t flags = PwrChipPacket::A_ALL;

    resetLineToSpi(ALL_SPI);
    writeToSpi(ALL_SPI, flags, &g_ez.hostTimeMs, sizeof(ms), &ms);
}

void PwrChipChain::broadcastCmnConfig(bool changed)
{
    PwcCmnConfig &cmnConfig = g_masterGate.masterData.pwcConfig;

    static uint32_t crc32 = 0;
    static uint32_t seqTx = 0;

    if (changed)
    {
        crc32 = ExchangeZone::calcCrc32(&cmnConfig, sizeof(cmnConfig));
        seqTx++;
    }

    log("BROADCAST CMN CONFIG: crc32 = 0x%08x, seq.tx = %d\n", crc32, seqTx);
    //cmnConfig.dump();

    uint8_t flags = PwrChipPacket::A_ALL;

    resetLineToSpi(ALL_SPI);
    writeToSpi(ALL_SPI, flags, &g_ez.cmnConfig, sizeof(cmnConfig), &cmnConfig);

    resetLineToSpi(ALL_SPI);
    writeToSpi(ALL_SPI, flags, &g_ez.ccCrc.crc32, sizeof(crc32), &crc32);

    resetLineToSpi(ALL_SPI);
    writeToSpi(ALL_SPI, flags, &g_ez.ccSeq.tx, sizeof(seqTx), &seqTx);
}

//chenbo add begin 20180109
void PwrChipChain::broadcastCmnConfig(bool changed , int spiId)
{
    PwcCmnConfig &cmnConfig = g_masterGate.masterData.pwcConfig;
	uint8_t boardId = g_multyBoardMgr.getChain(spiId).getBoardId();
    static uint32_t crc32 = 0;
    static uint32_t seqTx = 0;

	log("BROADCAST CMN CONFIG: changed = %d, boardId = %d\n", changed, spiId);
	
	if(g_multyBoardMgr.getBoard(boardId).info.boardOSC != 0)
	{
		cmnConfig.osc = g_multyBoardMgr.getBoard(boardId).info.boardOSC;
	}	

    //if (changed)
	if(1)
    {
        crc32 = ExchangeZone::calcCrc32(&cmnConfig, sizeof(cmnConfig));
		//if(spiId == 0){
		//	seqTx++;
		//}
		seqTx++;
    }

    log("BROADCAST CMN CONFIG: crc32 = 0x%08x, seq.tx = %d\n", crc32, seqTx);
    //cmnConfig.dump();

    //uint8_t flags = PwrChipPacket::A_0 + boardId;
    uint8_t flags = PwrChipPacket::A_ALL;

    resetLineToSpi(spiId);
    writeToSpi(spiId, flags, &g_ez.cmnConfig, sizeof(cmnConfig), &cmnConfig);

    resetLineToSpi(spiId);
    writeToSpi(spiId, flags, &g_ez.ccCrc.crc32, sizeof(crc32), &crc32);

    resetLineToSpi(spiId);
    writeToSpi(spiId, flags, &g_ez.ccSeq.tx, sizeof(seqTx), &seqTx);
}
//chenbo add end

void PwrChipChain::broadcastJob(bool changed)
{
    PwcMiningData &miningData = g_masterGate.miningData;

    static uint32_t crc32 = 0;
    static uint32_t seqTx = 0;

    if (changed)
    {
        crc32 = ExchangeZone::calcCrc32(&miningData, sizeof(miningData));
        seqTx++;
    }

    log("BROADCAST JOB: crc32 = 0x%08x, seq.tx = %d\n", crc32, seqTx);
    //miningData.dump();

    uint8_t flags = PwrChipPacket::A_ALL;

    resetLineToSpi(ALL_SPI);
    writeToSpi(ALL_SPI, flags, &g_ez.miningData, sizeof(miningData), &miningData);

    resetLineToSpi(ALL_SPI);
    writeToSpi(ALL_SPI, flags, &g_ez.mdCrc.crc32, sizeof(crc32), &crc32);

    resetLineToSpi(ALL_SPI);
    writeToSpi(ALL_SPI, flags, &g_ez.mdSeq.tx, sizeof(seqTx), &seqTx);
}

void PwrChipChain::broadcastCpuDisable(int attempts)
{
    log("BROADCAST: CPU DISABLE\n");
    for (int i = 0; i < attempts; i++)
    {
        resetLineToSpi(ALL_SPI);
        writeRegToSpi(ALL_SPI, PwrChipPacket::A_ALL, PwrChipPacket::ADDR_CPU_ENABLE, 0x00000000);
    }
}

void PwrChipChain::broadcastCpuEnable(int attempts)
{
    log("BROADCAST: CPU ENABLE\n");
    for (int i = 0; i < attempts; i++)
    {
        resetLineToSpi(ALL_SPI);
        writeRegToSpi(ALL_SPI, PwrChipPacket::A_ALL, PwrChipPacket::ADDR_CPU_ENABLE, 0x00000001);
    }
}

void PwrChipChain::downloadNonces()
{
    log("DOWNLOAD NONCES\n");

    resetLine();

    sendNextLR( PwrChipPacket::condIfNot(SPI_FLAG_HAS_NONCES) );
    sendBreak();

    bool errorDetected = false;

    for (int i = 0; i < spiLen; i++)
    {
        bool allTimeout = true;

        for (int parity = 0; parity < 4; parity++)
        {
            PwcReadResult r = downloadNoncesFrom( parity );

            if (r != PWC_READ_TIMEOUT)
            {
                allTimeout = false;
            }
            if (r == PWC_READ_ERROR)
            {
                errorDetected = true;
            }
        }

        if (allTimeout)
        {
            break;
        }

        sendNextLR();
    }

    if (errorDetected)
    {
        log("DOWNLOAD NONCES: manually\n");
        for (gotoFirstChip(); currentSeq < spiLen; gotoNextChip())
        {
            downloadNoncesFrom( seqToParity(currentSeq) );
        }
    }
}

void PwrChipChain::downloadTestStats()
{
    log("DOWNLOAD TEST STATS: spi=%d, len=%d\n", spiId, spiLen);

    for (gotoFirstChip(); currentSeq < spiLen; gotoNextChip())
    {
        downloadTestStatsFrom(currentSeq);
    }
}

PwcReadResult PwrChipChain::downloadNoncesFrom(uint8_t parity)
{
    uint8_t flags = PwrChipPacket::A_0 + parity;

    uint32_t txTime[g_ez.nonces.MAX_LEN];
    PwcReadResult result = read(flags, &g_ez.nonces.txTime, sizeof(g_ez.nonces.txTime), &txTime);
    if (result != PWC_READ_OK)
    {
        //log("parity[%d]: no response or error\n", parity);
        return result;
    }

    uint8_t numNonces = 0;

    for (uint32_t i = 0; i < g_ez.nonces.MAX_LEN; i++)
    {
        if (txTime[i] == 0) continue;

        log("loading nonce: parity[%u]: txTime[%u] = %9u\n", parity, i, txTime[i]);

        PwcNonceData nd;
        if (read(flags, &g_ez.nonces.nonces[i], sizeof(nd), &nd) != PWC_READ_OK)
        {
            log("nonces: can not read nonce data!\n");
            continue;
        }

        log("loaded nonce data: "); nd.dump();

        uint32_t crc32;
        if (read(flags, &g_ez.nonces.crc32[i], sizeof(crc32), &crc32) != PWC_READ_OK)
        {
            log("nonces: can not read nonce data crc32!\n");
            continue;
        }

        uint32_t crc32Calced = ExchangeZone::calcCrc32(&nd, sizeof(nd));
        if (crc32 != crc32Calced)
        {
            log("nonces: nonce data crc mismatch: crc=0x%08x, calced=0x%08x!\n",
                crc32, crc32Calced);
            continue;
        }

        // clearing nonces.txTime[i]
        writeReg(flags, (uint32_t)&g_ez.nonces.txTime[i], 0);

        numNonces++;
        g_masterGate.sendNonce(nd);
    }

    return numNonces > 0 ? PWC_READ_OK : PWC_READ_ERROR;
}

void PwrChipChain::serviceNextLevel()
{
    log("SERVICING CHIPS: spi=%d, seq=%d (of %d)\n", spiId, serviceSeq, spiLen);

    int maxSeq = serviceSeq + 2;

    if (maxSeq > spiLen)
        maxSeq = spiLen;

    for (gotoChip(serviceSeq); currentSeq < maxSeq; gotoNextChip())
    {
        loadConfigTo(currentSeq);
        downloadStatsFrom(currentSeq);
    }

    serviceSeq = currentSeq;
    if (serviceSeq >= spiLen)
        serviceSeq = 0;
}

void PwrChipChain::loadConfigTo(uint8_t spiSeq)
{
    PwrChip &chip = getChip(spiSeq);
    uint8_t flags = seqToAddr(spiSeq);

    PwcUniqConfig &uniqConfig = chip.getUniqConfig();

    uint32_t crc32 = ExchangeZone::calcCrc32(&uniqConfig, sizeof(uniqConfig));

    if (chip.uniqConfigCrc != crc32)
    {
        chip.uniqConfigCrc = crc32;
        chip.uniqConfigSeq++;
    }

    EzSeqRegs seq;
    if (read(flags, &g_ez.ucSeq, sizeof(seq), &seq) != PWC_READ_OK)
    {
        //if (debugOn)
            log("PWC[%d/%d]: CONFIG: can not read seq!\n", spiId, spiSeq);
        return;
    }

    if (seq.tx == chip.uniqConfigSeq && seq.tx == seq.rx)
    {
        chip.spiData.uniqConfigLoaded = true;
    }
    else
    {
        chip.spiData.uniqConfigLoaded = false;

        //if (debugOn)
        {
            log("PWC[%d/%d]: CONFIG: sending config: tx = %d, crc = 0x%08x\n",
                spiId, spiSeq, chip.uniqConfigSeq, chip.uniqConfigCrc);
        }

        write(flags, &g_ez.uniqConfig, sizeof(uniqConfig), &uniqConfig);
        writeReg(flags, (uint32_t)&g_ez.ucCrc.crc32, chip.uniqConfigCrc);
        writeReg(flags, (uint32_t)&g_ez.ucSeq.tx, chip.uniqConfigSeq);
    }
}

void PwrChipChain::downloadTestStatsFrom(uint8_t spiSeq)
{
    ExchangeZoneBt &ez = *(ExchangeZoneBt*)PWC_EZ_ADDR;

    uint8_t flags = seqToAddr(spiSeq);
    PwrChip &chip = getChip(spiSeq);

    PwcTestBlock b;
    if (read(flags, &ez.pwcTestData, sizeof(b), &b) != PWC_READ_OK)
    {
        log("PWC[%d/%d]: TEST: can not read data!\n", spiId, spiSeq);
        return;
    }

    if (b.parity != seqToParity(spiSeq))
    {
        log("PWC[%d/%d]: TEST: block parity mismatch: %d!\n",
            spiId, spiSeq, b.parity);
        return;
    }

    log("PWC[%d/%d]: TEST: loaded 0x%08x!!!\n", spiId, spiSeq, b.btcTestResults);

    b.btcTestResults = comressBitData(b.btcTestResults, chip.getPwcMask(), BTC_TEST_NUM_BITS);

    for (int i = 0; i < MAX_BTC16_PER_PWC; i++)
    {
        uint32_t mask = ((1 << BTC_TEST_NUM_BITS) - 1);
        mask <<= i * BTC_TEST_NUM_BITS;

        if ((chip.testRes.btcResults & mask) < (b.btcTestResults & mask))
        {
            chip.testRes.btcResults &= ~mask;
            chip.testRes.btcResults |= (b.btcTestResults & mask);
        }
    }
}

void PwrChipChain::downloadStatsFrom(uint8_t spiSeq)
{
    uint8_t flags = seqToAddr(spiSeq);
    PwrChip &chip = getChip(spiSeq);

    if (chip.spiData.memTotal < 0xff)
    {
        chip.spiData.memTotal++;

        uint32_t reg = 0;

        // read VendorId register (from SPI Module address space)
        if (readReg(flags, PwrChipPacket::ADDR_CHIP_VENDOR_ID, reg) == PWC_READ_OK)
        {
            chip.spiData.memVendor++;
        }

        if (readReg(flags, 0x0000, reg) == PWC_READ_OK)
        {
            chip.spiData.memConst++;
        }

        if (readReg(flags, 0x7ff0, reg) == PWC_READ_OK)
        {
            chip.spiData.memUniq++;
            chip.spiData.memUniqVal = reg;
        }
    }

    chip.spiData.pwcStatTotal++;

    EzSeqRegs seq;
    if (read(flags, &g_ez.pwcSeq, sizeof(seq), &seq) != PWC_READ_OK)
    {
        if (debugOn) log("PWC[%d/%d]: STATS: can not read seq!\n", spiId, spiSeq);
        return;
    }

    if (debugOn) { log("PWC[%d/%d]: STATS: seq: ", spiId, spiSeq); seq.dump(); }

    if (seq.tx == seq.rx)
    {
        // data is not ready
        return;
    }

    PwcBlock pwcData;
    if (read(flags, &g_ez.pwcData, sizeof(pwcData), &pwcData) != PWC_READ_OK)
    {
        log("PWC[%d/%d]: STATS: can not read data!\n", spiId, spiSeq);
        return;
    }

    EzCrcRegs crc;
    if (read(flags, &g_ez.pwcCrc, sizeof(crc), &crc) != PWC_READ_OK)
    {
        log("PWC[%d/%d]: STATS: can not read crc!\n", spiId, spiSeq);
        return;
    }

    uint32_t actualCrc = ExchangeZone::calcCrc32(&pwcData, sizeof(pwcData));
    if (actualCrc != crc.crc32)
    {
        log("PWC[%d/%d]: STATS: crc mismatch: actual = 0x%08x, expected = 0x%08x!\n",
            spiId, spiSeq, actualCrc, crc.crc32);
        return;
    }

    if (pwcData.spiSeq == spiSeq)
    {
        chip.spiData.pwcStatOk++;

        log("PWC[%d/%d]: STATS: loaded (tx=%d)!!!\n", spiId, spiSeq, seq.tx);
        if (debugOn) pwcData.dump();

        chip.updateInfo(seq.tx, pwcData.pwcSharedData);

        g_masterGate.sendPwcData(chip, pwcData);
    }
    else
    {
        chip.spiData.pwcStatShift++;

        log("PWC[%d/%d]: STATS: pwc block spi seq mismatch: %d!\n",
            spiId, spiSeq, pwcData.spiSeq);
    }

    seq.rx = seq.tx;
    write(flags, &g_ez.pwcSeq.rx, sizeof(seq.rx), &seq.rx);
}
