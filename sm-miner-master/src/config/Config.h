#ifndef CONFIG_H
#define CONFIG_H
/*
 * Contains Config class declaration along with a series of dependent data structures.
 */

#include "cmn_block.h"
#include "ms-protocol/ms_data.h"

#include "base/StringBuffer.h"

#include <stdint.h>


// Class holding Pool settings.
struct PoolConfig
{
    StringBuffer<512> host;  // Host name.
    StringBuffer<128> port;  // Port.
    StringBuffer<128> userName;  // User name.
    StringBuffer<128> password;  // User password.
#ifdef MULTI_POOL_SUPPORT
    // fcj add begin 20180313
    // bak1 pool
    StringBuffer<512> bak1Host;  // Bak1 host name.
    StringBuffer<128> bak1Port;  // Bak1 port.
    StringBuffer<128> bak1UserName;  // Bak1 user name.
    StringBuffer<128> bak1Password;  // Bak1 user password.

    // bak2 pool
    StringBuffer<512> bak2Host;  // Bak2 host name.
    StringBuffer<128> bak2Port;  // Bak2 port.
    StringBuffer<128> bak2UserName;  // Bak2 user name.
    StringBuffer<128> bak2Password;  // Bak2 user password.

    unsigned int retryNTimes; // Retry n times, then re-select pool
    // fcj add end
#endif
    bool exitOnError;  // Exit in case of any pool error.
    unsigned int maxConnectionAttempts;  // Exit if cannot connect to the pool after N retries.
    
    unsigned int authorizationTimeoutSec;  // Authorization timeout, sec.
    unsigned int subscriptionTimeoutSec;  // Subscription timeout, sec.
    unsigned int bootstrapTimeoutSec;  // Difficulty and job receiving timeout, sec.
    
    uint32_t initialDifficulty;  // Initial difficulty.

    // Initializes Pool configuration with the default values.
    PoolConfig();
};

struct PsuLogConfig
{
    bool logOn;
    bool logAll;
    uint32_t logDepth;
    double deviation;
    double vInMin;
    double vInMax;
    double vOutMin;
    double vOutMax;
    double iOutMin;
    double iOutMax;

    PsuLogConfig();
};

// Class holding ServerTest settings.
struct TestConfig
{
    uint8_t testMode;
    unsigned int testNum;
    unsigned int testTimeSec;
    unsigned int testCoolingTimeSec;
    double       testFanWaitI;
    unsigned int testMbHwVer;
    unsigned int testSlaveHwVer;
    unsigned int testExpRev;
    unsigned int testTempMin;
    unsigned int testTempMax;
    bool         testNoHeatSink;
    unsigned int testNoHsNumMax;
    unsigned int testHsNumMin;
    unsigned int testVoltageMin;
    unsigned int testVoltageMax;
    unsigned int testCurrentMin;
    unsigned int testCurrentMax;
    unsigned int testVoltage2Min;
    unsigned int testVoltage2Max;
    unsigned int testCurrent2Min;
    unsigned int testCurrent2Max;
    unsigned int testOcpOffMin;
    unsigned int testOcpOffMax;
    unsigned int testMaxBtcNoSol;
    unsigned int testMaxBtcNoJs;
    unsigned int testMaxBtcNoResp;
    unsigned int testBrdOffTimeSec;
    unsigned int testBrdOffCurrentMax;
    bool         testPrintWarnings;

    double       testFanOnVDev;
    double       testFanOnIMin;
    double       testFanOnIMax;

    // Initializes ServerTest configuration with the default values.
    TestConfig();
};

enum SpiDrvMode {
    SPI_DRV_HW = 0,
    SPI_DRV_SW,
    SPI_DRV_AUTO,
};

// A singleton class holding configuration settings for the application.
struct Config
{
    // Runtime options.
    bool showUsage;
    bool noMcuReset;
    
    // General application settings.
    unsigned int pollingDelayMs;  // Polling delay in milliseconds.
    unsigned int aliveEventIntervalSec;

    // Grid/System settings.
    uint8_t  mbHwVer;
    uint32_t slaveMask;
    uint32_t boardMask;
    uint32_t boardMaskB;
    uint8_t  slaveSpiDrv;
    uint32_t slaveSpiSpeed;
    bool msSpiDebug;

    // Pool settings.
    PoolConfig poolConfig;
    
    // SlaveGate settings.
    SlaveConfig slaveConfig;
    PsuConfig psuConfig;
    PsuLogConfig psuLogConfig;
    BoardConfig boardConfig[MAX_BOARD_PER_MASTER];
    PwcCmnConfig pwcConfig;

    // Logging settings.
    bool traceAll;  // Whether to set log level to TRACE globally.
    bool traceStratum;  // Whether to dump Stratum protocol communication.
    unsigned int logSlaveLevel;
    
    // Statistics settings.
    unsigned int logDelaySec; // Print chip stats interval in seconds.
    bool logChipStat;
    bool logEmptyBoards;

    // Events settings.
    StringBuffer<256> eventFilePath;
    
    // ServerTest settings.
    TestConfig testConfig;
    
    // Initializes configuration settings with the default values.
    Config();
};

#endif  // CONFIG_H
