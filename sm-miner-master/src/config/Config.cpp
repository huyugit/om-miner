/*
 * Contains Config class definition.
 */

#include "Config.h"


PoolConfig::PoolConfig()
    : host("47.95.65.157")
    , port("443")
    , userName("bitfily.Bitfily_BTC")
    , password("1")
#ifdef MULTI_POOL_SUPPORT
    // fcj add begin 20180313
    // bak1 pool
    , bak1Host("47.95.65.157")
    , bak1Port("443")
    , bak1UserName("bitfily.Bitfily_BTC")
    , bak1Password("1")
    // bak2 pool
    , bak2Host("47.95.65.157")
    , bak2Port("443")
    , bak2UserName("bitfily.Bitfily_BTC")
    , bak2Password("1")
    // Retry n times, then re-select pool
    , retryNTimes(3)
    // fcj add end
#endif
    , exitOnError(false)
    , maxConnectionAttempts(1000000)
    , authorizationTimeoutSec(45)
    , subscriptionTimeoutSec(45)
    , bootstrapTimeoutSec(45)
    , initialDifficulty(DEFAULT_DIFF)
{}

PsuLogConfig::PsuLogConfig()
    : logOn(true),
      logAll(false),
      logDepth(4*5),
      deviation(0.1),
      vInMin(200.0),
      vInMax(260.0),
      vOutMin(40.0),
      vOutMax(60.0),
      iOutMin(40.0),
      iOutMax(60.0)
{}

TestConfig::TestConfig()
    : testMode(TEST_MODE_NONE)
    , testNum(3)
    , testTimeSec(60)
    , testCoolingTimeSec(10)
    , testFanWaitI(3.0)
    , testMbHwVer(0xf)
    , testSlaveHwVer(0xe)
    , testExpRev(26)
    , testTempMin(20)
    , testTempMax(40)
    , testNoHeatSink(false)
    , testNoHsNumMax(0)
    , testHsNumMin(10)
    , testVoltageMin(42000)
    , testVoltageMax(45000)
    , testCurrentMin(7000)
    , testCurrentMax(11000)
    , testVoltage2Min(42000)
    , testVoltage2Max(45000)
    , testCurrent2Min(6000)
    , testCurrent2Max(9000)
    , testOcpOffMin(0x0a)
    , testOcpOffMax(0x15)
    , testMaxBtcNoSol(5)
    , testMaxBtcNoJs(0)
    , testMaxBtcNoResp(0)
    , testPrintWarnings(false)
    , testFanOnVDev(0.2)
    , testFanOnIMin(3.0)
    , testFanOnIMax(7.0)
{}

Config::Config()
    : showUsage(false)
    , noMcuReset(false)
    //
    , pollingDelayMs(2)
    , aliveEventIntervalSec(3600)
    //
    , mbHwVer(0xff)
    , slaveMask(0xffffffff)
    , boardMask (0xfff)
    , boardMaskB(0x000)
    , slaveSpiDrv(SPI_DRV_AUTO)
    , slaveSpiSpeed(2000000)
    , msSpiDebug(false)
    //
    , poolConfig()
    //
    , slaveConfig()
    //
    , traceAll(false)
    , traceStratum(false)
    , logSlaveLevel(LOG_LEVEL_NONE)
    //
    , logDelaySec(10)
    , logChipStat(false)
    , logEmptyBoards(false)
    //
    , eventFilePath("/run/sm-miner.events")
    //
    , testConfig()
{}
