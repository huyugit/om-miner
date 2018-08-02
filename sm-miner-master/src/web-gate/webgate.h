#ifndef WEBGATE_H
#define WEBGATE_H
#include "base/NonCopyable.h"
#include "webjson.h"
#include "config/Config.h"

#define MASTER_STATUS_MASK			0x1
#define MINER_CHAIN_MASK			0x2
#define MINER_PSU_STATUS_MASK		0x4
#define MINER_FAN_STATUS_MASK		0x8
#define MINER_POOL_STATUS_MASK		0x10
#define MINER_INFO_MASK				0x20
#define MINER_TEST_MASK				0x40
#define MINER_BIN_MASK				0x80
#define MINER_STATUS_ALL			0xffffffff

class WebGate
        : private NonCopyable  // Prevent copy and assignment.
{
public:
	//WebGate();

	void init(void);

	void PushMinerStatus(unsigned int     mask);
	void PushASICStatus(void);
	void PushMinerMasterStatus(void);	
	void PushMinerChainStatus(void);	
	void PushMinerBinStatus(void);
	void PushMinerPSUStatus(void);
	void PushMinerFanStatus(void);
	void PushMinerPoolStatus(void);
	void PushMinerInfo(void);
	
	void PullMinerConfig(void);
	void PullOscConfig(void);
	void PullHashSNConfig(void);
	void PullFanRpmConfig(void);
	void PullPsuWorkCondConfig(void);
	void PullBinConfig(void);
	Config GetMinerConfig(Config& appConfig);

	Config minerconfig;
	OscConfig hstboardOSC;
	OscConfig PrevHstOsc;
	BinConfig hstboardBin;
	HashSNConfig hashSN;
	PsuWorkCondConfig psuWorkCond;
	uint32_t fanRpmCfg[FAN_NUM];
private:
	Json::Value ASICStatus(void);
	Json::Value chainASICStatus(BoardStat& hashboard);
	Json::Value MinerStatus(void);
	Json::Value SWversionStatus(void);
	Json::Value ChainStatus(int slaveId, int boardId);
	Json::Value PsuStatus(int slaveId, int psuId);
	Json::Value FanStatus(void);
	Json::Value PoolStatus(void);
	Json::Value MasterStatus(void);
	Json::Value MinerInfo(void);
	
	void PoolSetting(Json::Value poolset);

	PoolConfig webpoolset;
	int alertTMP;
	int fanSpeed;
	int asicFreq;
};

extern WebGate g_webgate;
#endif
