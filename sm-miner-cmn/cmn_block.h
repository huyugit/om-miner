#ifndef CMN_BLOCK_H
#define CMN_BLOCK_H

#include "format.hpp"
#include "cmn_defines.h"
#include "mcu_uid.h"
#include "ms-protocol/ms_packet_ids.h"


#define OSC_MODE_STATIC         0
#define OSC_MODE_ROUGH_OSC      1


struct PwcCmnConfig
{
    uint8_t     osc;
    uint16_t    jt;

    uint8_t     oscMode;

    uint32_t    btcSerCfg;

    // rough osc staff
    uint8_t     roOscMin;
    uint8_t     roOscMax;
    uint8_t     roEffZone0;
    uint8_t     roEffZone1;
    uint8_t     roDeltasToDec;
    uint8_t     roDeltasToInc;

    uint8_t dummyAlignToInt32[2];

    PwcCmnConfig()
        : osc(0),
          jt(100),
          oscMode(OSC_MODE_STATIC),
          btcSerCfg(BTC16_SER_CONFIG),
          roOscMin(46),
          roOscMax(52),
          roEffZone0(50),
          roEffZone1(98),
          roDeltasToDec(2),
          roDeltasToInc(5)
    {}

    void dump()
    {
        log("PwcCmnConfig:\n");
        log("  jt:              %d\n",          jt);
        log("  btcSerCfg:       0x%08x\n",      btcSerCfg);
    }
}
__attribute__ ((packed));


struct PwcMiningData
{
    static const uint8_t MS_ID = MS_ID_PwcMiningData;

    uint32_t    difficulty;

    uint8_t     extraNonce1Size;
    uint8_t     extraNonce1[EXTRA_NONCE_BYTES];

    uint32_t    extraNonce2Size;

    uint32_t    masterJobId;
    uint8_t     prevHash[HASH_BYTES];
    uint8_t     coinBase1Len;
    uint8_t     coinBase1[COINBASE_BYTES];
    uint8_t     coinBase2Len;
    uint8_t     coinBase2[COINBASE_BYTES];

    uint8_t     merkleRootLen;
    uint8_t     merkleRoot[16][HASH_BYTES];

    uint32_t    version;
    uint32_t    nBits;
    uint32_t    nTime;
    bool        cleanJobs;

    uint8_t     dummyAlignToInt32[3];


    PwcMiningData()
    {
        memset(this, 0, sizeof(PwcMiningData));
    }

    void initDefault()
    {
        difficulty      = DEFAULT_DIFF;

        extraNonce1Size = 4;

        memset(extraNonce1, 0, sizeof(extraNonce1));
        extraNonce1[1] = 1;

        extraNonce2Size = 4;

        masterJobId     = 1;

        coinBase1Len    = 0;
        coinBase2Len    = 0;

        merkleRootLen   = 0;

        version         = 0x00000002;
        nBits           = 0x00000003;
        nTime           = 0x00000004;

        cleanJobs       = true;
    }

    void dump() const
    {
        log("PwcMiningData:\n");
        log("  difficulty:       %d\n",      difficulty);

        log("  extraNonce1Size:  %d\n",      extraNonce1Size);
        log("  extraNonce1:      ");         hexdump8(extraNonce1, extraNonce1Size);

        log("  extraNonce2Size:  %d\n",      extraNonce2Size);
        log("  masterJobId:      0x%08x\n",  masterJobId);
        log("  prevHash:         ");         hexdump8(prevHash, sizeof(prevHash));
        log("  coinBase1Len:     %d\n",      coinBase1Len);
        log("  coinBase1:        ");         hexdump8(coinBase1, sizeof(coinBase1));
        log("  coinBase2Len:     %d\n",      coinBase2Len);
        log("  coinBase2:        ");         hexdump8(coinBase2, sizeof(coinBase2));

        log("  merkleRoot: %d elements\n", merkleRootLen);
        for (uint32_t i = 0; i < merkleRootLen; i++)
        {
            log("  merkleRoot[%d]: ", i); hexdump8(merkleRoot[i], sizeof(merkleRoot[i]));
        }

        log("  version:          0x%08x\n",  version);
        log("  nBits:            0x%08x\n",  nBits);
        log("  nTime:            0x%08x\n",  nTime);
        log("  cleanJobs:        %d\n",      cleanJobs);
    }
}
__attribute__ ((packed));


struct SlaveSpiStat
{
    uint32_t spiTxRx;
    uint32_t spiRxOk;
    uint32_t spiRxErr;

    SlaveSpiStat()
        : spiTxRx(0), spiRxOk(0), spiRxErr(0)
    {}

    void dump() const
    {
        log("SlaveSpiStat:\n");
        log("  spiTxRx:     %d\n", spiTxRx);
        log("  spiRxOk:     %d\n", spiRxOk);
        log("  spiRxErr:    %d\n", spiRxErr);
    }
}
__attribute__ ((packed));


struct PwcSharedData
{
    uint32_t totalTime;
    uint32_t totalSolutions;
    uint32_t totalErrors;
    uint32_t totalSolByDiff;
    uint32_t totalJobsDone;
    uint32_t totalFixes;
    uint32_t sharesSent;
    uint32_t sharesDropped;

    uint32_t cpuFreq;
    uint32_t uartSpeed;

    PwcSharedData()
        : totalTime(0),
          totalSolutions(0),
          totalErrors(0),
          totalSolByDiff(0),
          totalJobsDone(0),
          totalFixes(0),
          sharesSent(0),
          sharesDropped(0),
          cpuFreq(0),
          uartSpeed(0)
    {}

    void dump()
    {
        log("PwcSharedData:\n");
        log("  upTime       = %d\n", totalTime);
        log("  sol          = %d\n", totalSolutions);
        log("  err          = %d\n", totalErrors);
        log("  solByDiff    = %d\n", totalSolByDiff);
        log("  jobsDone     = %d\n", totalJobsDone);
        log("  fixes        = %d\n", totalFixes);
        log("  cpuFreq      = %d\n", cpuFreq);
        log("  uartSpeed    = %d\n", uartSpeed);
    }
}
__attribute__ ((packed));


struct BtcFixesStat
{
    uint32_t rejected;
    uint32_t processed;
    uint32_t ok;
    uint32_t hwErr;

    BtcFixesStat()
        : rejected(0),
          processed(0),
          ok(0),
          hwErr(0)
    {}

    void dump()
    {
        log("BtcFixesStat: REJ=%u TEST=%u OK=%u\n",
            rejected, processed, ok);
    }
}
__attribute__ ((packed));

struct BtcExtStat
{
    uint16_t serErrors;
    uint16_t jsErrors;

    BtcExtStat()
        : serErrors(0),
          jsErrors(0)
    {}

    void dump()
    {
        log("BtcExtStat: SE=%u JSE=%u\n",
            serErrors, jsErrors);
    }
}
__attribute__ ((packed));

struct Btc16StatData
{
    uint8_t  chipId;        // NOT used
    uint8_t  osc;
    uint16_t reads;
    uint32_t solutions;
    uint32_t errors;
    uint32_t jobsDone;
    uint16_t lastJobTime;
    uint16_t restarts;

    BtcExtStat   stat;
    BtcFixesStat fixes;

    void dump()
    {
        log("ID=%d OSC=%d R=%d S=%d E=%d J=%d JT=%d CR=%d\n",
            chipId, osc, reads, solutions, errors, jobsDone, lastJobTime, restarts);
        fixes.dump();
    }
}
__attribute__ ((packed));


struct PwcBlock
{
    uint8_t spiSeq;
    PwcSharedData pwcSharedData;

    uint8_t len;
    Btc16StatData btc16StatData[MAX_BTC16_PER_PWC];

    uint8_t dummyAlignToInt32[2];


    void dump()
    {
        log("PwcBlock:\n");
        log("spiSeq:        %d\n", spiSeq);

        pwcSharedData.dump();

        log("btc16StatData: %d elements\n", len);
        for (uint32_t i = 0; i < len; i++)
        {
            log("  btc16[%d]: ", i); btc16StatData[i].dump();
        }
    }
}
__attribute__ ((packed));


#define BTC_TEST_NUM_BITS           2

#define BTC_TEST_NA                 0
#define BTC_TEST_ACTIVITY_DETECTED  1
#define BTC_TEST_JS_DETECTED        2
#define BTC_TEST_OK                 3

struct PwcTestBlock
{
    uint32_t    parity;
    uint32_t    btcTestResults;

    //uint8_t dummyAlignToInt32[0];
}
__attribute__ ((packed));


struct PwcNonceData
{
    uint32_t masterJobId;
    uint32_t extraNonce2;
    uint32_t nTime;
    uint32_t nonce;
    uint32_t shareDiff;

    void dump() const
    {
        log("masterJobId=%d, extraNonce2=0x%08x, nTime=0x%08x, nonce=0x%08x, shareDiff=%d\n",
            masterJobId, extraNonce2, nTime, nonce, shareDiff);
    }
}
__attribute__ ((packed));


#endif // CMN_BLOCK_H
