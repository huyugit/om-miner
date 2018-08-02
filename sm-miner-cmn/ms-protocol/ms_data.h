#ifndef MS_DATA_H
#define MS_DATA_H

#include "ms_defines.h"
#include "ms_packet_ids.h"
#include "ms_data_fan.h"
#include "ms_data_hbt.h"
#include "ms_data_psu.h"
#include "format.hpp"
#include "cmn_block.h"

/* lxj add begin 20180329 */
#include "ms_error.h"
/* lxj add end 20180329 */

#define SLAVE_DEBUG_LOG_ADC             0x00000001
#define SLAVE_DEBUG_TEST_POWER_SW       0x00000002


struct SlaveConfig
{
    uint16_t spiTimeRst;
    volatile uint16_t spiTimeTx0;
    volatile uint16_t spiTimeTx1;
    volatile uint16_t spiTimeTx2;
    uint16_t spiTimeRx0;
    uint16_t spiTimeRx1;
    uint16_t spiResetCycles;

    uint8_t  pwcClockCfg;

    bool mcuLogChipStat;
    bool mcuLogStratum;

    bool mcuLogSpi;
    uint8_t mcuLogSpi_spi, mcuLogSpi_seq;

    uint8_t userSlotMask;
    uint8_t userBrdRev;
    uint8_t userBrdIfFound;
    uint8_t userBrdSpiNum;
    uint8_t userBrdSpiLen;
    uint8_t userBrdSpiMask;
    uint8_t userBrdPwrNum;
    uint8_t userBrdPwrLen;
    uint8_t userBrdBtcNum;
    uint16_t userBrdBtcMask;

    uint8_t overCurrentProtection;

    uint16_t psLowCurrent;
    uint8_t psLowCurrentPeriod;

    int8_t maxTempLo;
    int8_t maxTempHi;

    uint32_t debugOpt;
    uint8_t testMode;

    SlaveHbtConfig hbtConfig;
    FanConfig fanConfig;


    SlaveConfig()
        : spiTimeRst(2),
          spiTimeTx0(2),
          spiTimeTx1(2),
          spiTimeTx2(4),
          spiTimeRx0(10),
          spiTimeRx1(30),
          spiResetCycles(512),
          pwcClockCfg(PWC_CPU_CLOCK_CFG),
          userSlotMask(0x7),
          userBrdRev(0),
          userBrdIfFound(0),
          userBrdSpiNum(0),
          userBrdSpiLen(0),
          userBrdSpiMask(0),
          userBrdPwrNum(0),
          userBrdPwrLen(0),
          userBrdBtcNum(0),
          userBrdBtcMask(0),
          overCurrentProtection(0x60),
          psLowCurrent(1000),
          psLowCurrentPeriod(20),
          maxTempLo(60),
          maxTempHi(85), //chenbo modify 90 -> 85 20180102
          debugOpt(0),
          testMode(TEST_MODE_NONE)
    {}

    void dump()
    {
        log("SlaveConfig:\n");
        log("  spiResetCycles   = %d\n", spiResetCycles);

        log("  mcuLogChipStat   = %d\n", mcuLogChipStat);
        log("  mcuLogStratum    = %d\n", mcuLogStratum);

        log("  mcuLogSpi        = %d\n", mcuLogSpi);
        log("    spi            = %d\n", mcuLogSpi_spi);
        log("    seq            = %d\n", mcuLogSpi_seq);

        log("  customSlotMask   = %d\n", userSlotMask);

        log("  userBrdRev       = %d\n", userBrdRev);
        log("  userBrdIfFound   = %d\n", userBrdIfFound);
        log("  userBrdSpiNum    = %d\n", userBrdSpiNum);
        log("  userBrdSpiLen    = %d\n", userBrdSpiLen);
        log("  userBrdSpiMask   = %d\n", userBrdSpiMask);
        log("  userBrdPwrNum    = %d\n", userBrdPwrNum);
        log("  userBrdPwrLen    = %d\n", userBrdPwrLen);
        log("  userBrdBtcNum    = %d\n", userBrdBtcNum);
        log("  userBrdBtcMask   = %d\n", userBrdBtcMask);

        log("  overCurrentProt  = 0x%02x\n", overCurrentProtection);

        log("  psLowCurrent     = %d\n", psLowCurrent);
        log("  psLowCurrentT    = %d\n", psLowCurrentPeriod);

        log("  debugOpt         = 0x%08x\n", debugOpt);
        log("  testMode         = %d\n", testMode);
    }
}
__attribute__ ((packed));


struct BoardConfig
{
    bool      powerOn;
    uint8_t   ocp;

    BoardConfig()
        : powerOn(true),
          ocp(0)
    {}

    void dump()
    {
        log("BoardConfig:\n");
        log("  powerOn:         %d\n", powerOn);
    }
}
__attribute__ ((packed));


struct LedStates
{
    uint8_t greenLedState;
    uint8_t redLedState;
    uint8_t boardLedColor[MAX_BOARD_PER_SLAVE];
    uint8_t boardLedState[MAX_BOARD_PER_SLAVE];

    void dump()
    {
        log("LedStates:\n");
        log("  green            = %d\n", greenLedState);
        log("  red              = %d\n", redLedState);
        for (int i = 0; i < MAX_BOARD_PER_SLAVE; i++)
        {
            log("  baord[%d].color   = %d\n", i, boardLedColor[i]);
            log("  baord[%d].state   = %d\n", i, boardLedState[i]);
        }
    }
}
__attribute__ ((packed));

//chenbo add begin 20180102
struct FanStates
{
	bool fan_fault[FAN_NUM];
	uint32_t RPM_Speed[FAN_NUM];

	void dump()
	{
		log("FanStates:\n");
		for (int i = 0; i < FAN_NUM; i++)
        {
            log("  Fan[%d].speed   = %d\n", i, RPM_Speed[i]);
            log("  Fan[%d].fault   = %d\n", i, fan_fault[i]);
        }
	}
}
__attribute__ ((packed));
//chenbo add end

//chenbo add begin 20180109
struct OscConfig
{
	uint8_t changedFlag;
	int HashBoardOsc[MAX_BOARD_PER_SLAVE];

	OscConfig()
		: changedFlag(0x00)
    {}
		  
	void dump()
	{
		log("OscConfig:\n");
		for (int i = 0; i < MAX_BOARD_PER_SLAVE; i++)
        {
			log("  HashBoardOsc[%d]   = %d\n", i, HashBoardOsc[i]);
        }
		log("  changedFlag   = %d\n", changedFlag);
	}
}
__attribute__ ((packed));
//chenbo add end

//gezhihua add begin 20180423
struct BinConfig
{
	uint8_t changedFlag;
	uint8_t HashBoardBin[MAX_BOARD_PER_SLAVE];

	BinConfig()
		: changedFlag(0x00)
    {}
		  
	void dump()
	{
		log("BinConfig:\n");
		for (int i = 0; i < MAX_BOARD_PER_SLAVE; i++)
        {
            log("  HashBoardBin[%d]   = %d\n", i, HashBoardBin[i]);
        }
		log("  changedFlag   = %d\n", changedFlag);
	}
}
__attribute__ ((packed));
//gezhihua add end

//chenbo add begin 20180123
struct HashSNConfig
{
	uint8_t changedFlag;
	char HashBoardSN[MAX_BOARD_PER_SLAVE][HASH_BOARD_SN_LENGHT];

	HashSNConfig()
		: changedFlag(0x00)
    {}
		  
	void dump()
	{

	}
}
__attribute__ ((packed));
//chenbo add end

//gzh add begin 20180125
struct PsuWorkCondConfig
{
	double 			PsuPoutLowTh;
	double 			PsuPoutHighTh;
	double 			PsuIoutLowTh;
	double 			PsuVoutHighTh;
	double 			PsuIoutHighTh;
	uint16_t 		PsuFanSpeedLowTh;
	uint16_t 		FanRpmLowTh;
	uint8_t 		FanFlag;
	uint8_t 		HashTempFlag;
	int8_t			HashTempHi;
	uint8_t 		PsuFlag;
	uint8_t 		FanWorkMode;
	int8_t 			PsuTempOutHighTh;
	uint8_t 		changedFlag;

	PsuWorkCondConfig()
		: PsuPoutLowTh(50),			//最低输出功率，小于该值关hash板
		  PsuPoutHighTh(3050),		//最高输出功率，大于该值关hash板
		  PsuIoutLowTh(1),			//最低输出电流，小于该值关hash板
		  PsuVoutHighTh(50),		//最高输出电压，大于该值关hash板
		  PsuIoutHighTh(70),		//最高输出电流，大于该值关hash板
		  PsuFanSpeedLowTh(70),		//最低风扇值，小于该值关hash板
		  FanRpmLowTh(5400),		//风扇最低转速，小于该值关闭hash板
		  FanFlag(1),				//开启风扇状态保护
		  HashTempFlag(1),			//开启hash板温度保护
		  HashTempHi(85),			//hash板温度保护上限是85度
		  PsuFlag(1),				//开启电源状态保护
		  FanWorkMode(1),			//开启风扇动态调速
		  PsuTempOutHighTh(85),		//最高温度值，高于该值关hash板
		  changedFlag(0x00)
    {}
		  
	void dump()
	{
	 	log("changedFlag   = %d\n", changedFlag);
		log("PsuPoutLowTh = %0.2f\n", PsuPoutLowTh);
		log("PsuPoutHighTh = %0.2f\n", PsuPoutHighTh);
		log("PsuIoutLowTh = %0.2f\n", PsuIoutLowTh);
		log("PsuIoutHighTh = %0.2f\n", PsuIoutHighTh);
		log("PsuVoutHighTh = %0.2f\n", PsuVoutHighTh);
		log("PsuFanSpeedLowTh = %d\n", PsuFanSpeedLowTh);
		log("PsuTempOutHighTh = %d\n", PsuTempOutHighTh);
		log("FanRpmLowTh = %d\n", FanRpmLowTh);
		log("HashTempFlag   = %u\n", HashTempFlag);
		log("PsuFlag = %u\n", PsuFlag);
		log("FanFlag = %u\n", FanFlag);
		log("FanWorkMode = %u\n", FanWorkMode);
	}
}
__attribute__ ((packed));
//gzh add end

struct MasterData
{
    static const uint8_t MS_ID = MS_ID_MasterData;

    uint32_t time;
    uint8_t  slaveId;

    SlaveConfig slaveConfig;
    BoardConfig boardConfig[MAX_BOARD_PER_SLAVE];
    PwcCmnConfig pwcConfig;
    LedStates ledStates;
	FanStates fanStates;  //chenbo add 20180102

	OscConfig hstboardOSC;  //chenbo add 20180109
	BinConfig hstboardBin;	//gezhihua add 20180423
	HashSNConfig hashboardSN;
	PsuWorkCondConfig psuWorkCond;	//gzh add 20180125
	
    void dump()
    {
        log("MasterData:");
        log("  time:             0x%08x\n", time);
        log("  slaveId:          %d\n",     slaveId);

        log("slaveConfig: "); slaveConfig.dump();
        for (int i = 0; i < MAX_BOARD_PER_SLAVE; i++)
        {
            log("boardConfig[%d]: ", i); boardConfig[i].dump();
        }
        log("pwcConfig: "); pwcConfig.dump();
        log("ledStates: "); ledStates.dump();
    }
}
__attribute__ ((packed));


#define BOARD_SPEC_FLAG_TYPE_A              0x01
#define BOARD_SPEC_FLAG_TYPE_B              0x02
#define BOARD_SPEC_FLAG_MISSING_CURRENT     0x04
#define BOARD_SPEC_FLAG_HEAT_SINK_ERR       0x08
#define BOARD_SPEC_FLAG_TMP_NUM_2           0x10
#define BOARD_SPEC_FLAG_TMP_ALERT           0x20


struct BoardSpec
{
    uint8_t revisionId;
    uint8_t spiNum;
    uint8_t spiLen;
    uint8_t spiMask;
    uint8_t pwrNum;
    uint8_t pwrLen;
    uint8_t btcNum;
    uint8_t btcMask;
    uint8_t voltage;
    uint8_t flags;
    float   adcKu;              // mV/adc
    float   adcKi;              // mA/adc

    char    humanName[32];

    bool isTypeA() const {
        return flags & BOARD_SPEC_FLAG_TYPE_A;
    }
    bool isTypeB() const {
        return flags & BOARD_SPEC_FLAG_TYPE_B;
    }
    bool isMissingCurrent() const {
        return flags & BOARD_SPEC_FLAG_MISSING_CURRENT;
    }
    bool isHeatSinkErrPin() const {
        return flags & BOARD_SPEC_FLAG_HEAT_SINK_ERR;
    }
    int getNumTmp() const {
        return (flags & BOARD_SPEC_FLAG_TMP_NUM_2) ? 2 : 1;
    }
    int getTmpOffset() const {
        return (flags & BOARD_SPEC_FLAG_TMP_NUM_2) ? 2 : 0;
    }
    bool isTmpAlert() const {
        return flags & BOARD_SPEC_FLAG_TMP_ALERT;
    }

    void dump() const
    {
        log("rev=%d, grid=%dx%d, spiMask=0x%x, pwrLen=%d, btcNum=%d, btcMask=0x%x, v=%d\n",
            revisionId, spiNum, spiLen, spiMask, pwrLen, btcNum, btcMask, voltage);
    }
}
__attribute__ ((packed));


struct TmpAlertInfo
{
    uint8_t     alertLo;
    uint8_t     alertHi;
    uint16_t    numWrite;

    void dump()
    {
        log("alertLo=%u, alertHi=%u, numWrite=%u\n",
            alertLo, alertHi, numWrite);
    }
}
__attribute__ ((packed));


struct BoardInfo
{
    bool        boardFound;
    bool        powerOn;

    int8_t     boardTemperature[MAX_TMP_PER_BOARD];
    uint16_t    revAdc;

	uint8_t     boardOSC;

    uint8_t     overCurrentProtection;
    TmpAlertInfo taInfo[MAX_TMP_PER_BOARD];

    bool        heaterErr;
    uint16_t    heaterErrNum;

    uint32_t    voltage;
    uint16_t    currents[MAX_PL_PER_BOARD];

    uint16_t    lowCurrRst;

    uint16_t    ohNum;
    uint32_t    ohTime;

	//chenbo add begin 20180123
	char hashSN[HASH_BOARD_SN_LENGHT];
	//chenbo add end
	uint8_t 	boardBin;
    void dump()
    {
        log("found=%d, pwrOn=%u, revADC=%d\n",
            boardFound, powerOn, revAdc);
    }
}
__attribute__ ((packed));


struct BoardData
{
    BoardInfo   info;
    BoardSpec   spec;

    void dump()
    {
        log("BOARD: info: "); info.dump();
        log("BOARD: spec: "); spec.dump();
    }
}
__attribute__ ((packed));


struct SlaveMbInfo
{
    uint8_t     hwVer;
    uint16_t    adc50V;
    FanInfo     fan;

    SlaveMbInfo()
        : hwVer(0),
          adc50V(0)
    {}

    double getMbVoltage() const;
    uint32_t getMbVoltageMV() const;

    void dump()
    {
        log("SlaveMbInfo:\n");
        log("  hwVer:   0x%02x\n",  hwVer);
        log("  adc50V:  %u\n",      adc50V);
        fan.dump();
    }
}
__attribute__ ((packed));


struct SlaveCmnData
{
    McuUID   uid;

    uint32_t swVersion;
    uint32_t totalTime;
    uint32_t loopbackTime;

    SlaveSpiStat slaveSpiStat;
    SlaveMbInfo mbInfo;


    SlaveCmnData()
        : swVersion(0),
          totalTime(0),
          loopbackTime(0)
    {}

    void dump()
    {
        log("SlaveCmnData:\n");
        log("  swVersion:       0x%08x\n", swVersion);
        log("  totalTime:       %u\n", totalTime);
        log("  loopbackTime:    %u\n", loopbackTime);
    }
}
__attribute__ ((packed));


struct SlaveData
{
    static const uint8_t MS_ID = MS_ID_SlaveData;

    SlaveCmnData cmnInfo;
    BoardData boardData[MAX_BOARD_PER_SLAVE];

    void dump()
    {
        log("SlaveBlock:\n");
        for (int i = 0; i < MAX_BOARD_PER_SLAVE; i++)
        {
            log("  board[%d]: ", i); boardData[i].dump();
        }
    }
}
__attribute__ ((packed));


struct SlaveHbtData
{
    static const uint8_t MS_ID = MS_ID_SlaveHbtData;

    SlaveTestInfo testInfo;
    BoardTest boardTest[MAX_BOARD_PER_SLAVE];

    void dump()
    {
        log("SlaveHbtData:\n");
        testInfo.dump();
    }
}
__attribute__ ((packed));


template <uint32_t SIZE_T>
struct NonceContainerT
{
    static const uint8_t MS_ID = MS_ID_NonceContainer;

    uint32_t numSent;
    uint32_t numDrop;

    static const uint32_t SIZE = SIZE_T;
    PwcNonceData nonces[SIZE];

    void clear() {
        numSent = 0;
        numDrop = 0;
        memset(nonces, 0, sizeof(nonces));
    }

    void dump() const
    {
        log("NonceContainer<%d>:\n", SIZE);
        for (uint32_t i = 0; i < SIZE; i++)
        {
            log("  nonces[%d]: ", i); nonces[i].dump();
        }
    }
}
__attribute__ ((packed));

typedef NonceContainerT<40> NonceContainer;


struct PwcQuickTestRes
{
    uint8_t pwcResult;
    uint16_t btcResults;

    void clear() {
        pwcResult   = PWC_TEST_NA;
        btcResults  = 0;
    }
}
__attribute__ ((packed));

struct PwcQuickTestArr
{
    static const uint8_t MS_ID = MS_ID_PwcQuickTestArr;

    uint16_t itemsPerBoard[MAX_BOARD_PER_SLAVE];
    PwcQuickTestRes items[MAX_PWC_PER_SLAVE];

    void clear() {
        memset(this, 0, sizeof(PwcQuickTestArr));
    }

    void dump()
    {
        log("PwcQuickTestArr\n");
    }
}
__attribute__ ((packed));


struct PwcDataItem
{
    uint8_t boardId;
    uint8_t spiId;
    uint8_t spiSeq;

    PwcBlock pwcData;

    void dump()
    {
        log("PwcDataItem:\n");
        log("  boardId      = %d\n", boardId);
        log("  spiId        = %d\n", spiId);
        log("  spiSeq       = %d\n", spiSeq);

        log("pwcData: "); pwcData.dump();
    }
}
__attribute__ ((packed));


struct PwcDataArray
{
    static const uint8_t MS_ID = MS_ID_PwcDataArray;

    static const uint8_t MAX_LEN = 12;

    uint8_t len;
    PwcDataItem items[MAX_LEN];

    void clear() {
        memset(this, 0, sizeof(PwcDataArray));
    }

    void dump()
    {
        log("PwcDataArray: %d elements\n", len);
        for (int i = 0; i < len; i++)
        {
            log("  item[%d]: ", i); items[i].dump();
        }
    }
}
__attribute__ ((packed));


struct PwcSpiData
{
    uint8_t boardId;
    uint8_t spiId;
    uint8_t spiSeq;

    bool found;
    uint8_t pwcTest;
    bool memOk;
    bool uniqConfigLoaded;

    uint8_t memVendor;
    uint8_t memConst;
    uint8_t memUniq;
    uint8_t memTotal;

    uint32_t memUniqVal;

    uint32_t pwcStatTotal;
    uint32_t pwcStatOk;
    uint32_t pwcStatShift;

    void dump()
    {
        log("PwcSpiData:\n");
        log("  boardId      = %d\n", boardId);
        log("  spiId        = %d\n", spiId);
        log("  spiSeq       = %d\n", spiSeq);
        log("  found        = %d\n", found);
        log("  pwcTest      = %d\n", pwcTest);
        log("  config       = %d\n", uniqConfigLoaded);

        log("  memVendor    = %d\n", memVendor);
        log("  memConst     = %d\n", memConst);
        log("  memUniq      = %d\n", memUniq);
        log("  memTotal     = %d\n", memTotal);
        log("  memUniqVal   = %d\n", memUniqVal);

        log("  statTotal    = %d\n", pwcStatTotal);
        log("  statOk       = %d\n", pwcStatOk);
        log("  pwcStatShift = %d\n", pwcStatShift);
    }
}
__attribute__ ((packed));


struct PwcSpiArray
{
    static const uint8_t MS_ID = MS_ID_PwcSpiArray;

    static const uint32_t MAX_LEN = MAX_PWC_PER_SPI;

    uint8_t len;
    PwcSpiData items[MAX_LEN];

    void dump()
    {
        log("PwcSpiArray: %d elements\n", len);
        for (uint32_t i = 0; i < len; i++)
        {
            log("  item[%2d]: ", i); items[i].dump();
        }
    }
}
__attribute__ ((packed));


/* lxj add begin 20180328 */

struct SlaveErrorData
{
    uint32_t error;

    void dump()
    {
        log("SlaveErrorData:\n");
        log("  error = %d\n", error);
    }

}
__attribute__ ((packed));

struct SlaveErrorArray
{
    static const uint8_t MS_ID = MS_ID_SlaveErrorArray;

    static const uint32_t MAX_NUM = MAX_SLAVE_ERROR_NUMS;	// current only support report an error 

    uint8_t num = 0;
    SlaveErrorData items[MAX_NUM];

    void dump()
    {
    	if (num > 0)
    	{
	        log("SlaveErrorArray: %d elements\n", num);
	        for (uint32_t i = 0; i < MAX_NUM; i++)
	        {
	            log("  item[%2d]: ", i); 
				items[i].dump();
	        }
    	}
    }

	void saveErrorCode(uint32_t errCode)
    {
#if 1
    	bool repeatflag = false;
    	for (uint32_t i=0; i < MAX_NUM; i++)
    	{
    		if (items[i].error == errCode)
    		{
    			repeatflag = true;
				break;
    		}
    	}

		if (repeatflag == false)
		{
			items[num % MAX_NUM].error = errCode;
			num++;
		}
#else
		if(num < MAX_NUM){
			items[num % MAX_NUM].error = errCode;
			num++;
		}
#endif
    }
}

__attribute__ ((packed));

/* lxj add end */




#endif // MS_DATA_H


