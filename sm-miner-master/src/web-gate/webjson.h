#ifndef WEBJSON_H
#define WEBJSON_H

#include "json/json-forwards.h"
#include "json/json.h"

class Webjson
{
public:
	Webjson();
	int MinerParsefromFile(Json::Value& root);
	int OscParsefromFile(Json::Value& root);
	int BinParsefromFile(Json::Value& root);
	int FanRpmParsefromFile(Json::Value& root);
	int PsuWorkCondParsefromFile(Json::Value& root);
	int HashSNParsefromFile(Json::Value& root);
	void MinerInsertToFile(std::string json_str);
	void ASICInsertToFile(std::string json_str);

	int TestConfParsefromFile(Json::Value& root);

	void MasterStatusInsertToFile(std::string json_str);
	void BinStatusInsertToFile(std::string json_str);
	void ChainStatusInsertToFile(std::string json_str);
	void PSUStatusInsertToFile(std::string json_str);
	void FanStatusInsertToFile(std::string json_str);
	void PoolStatusInsertToFile(std::string json_str);
	void MinerInfoInsertToFile(std::string json_str);

private:
	void PoolSetting(Json::Value poolset);
	void WriteJsonFile(const char* file, std::string str);

	const char* StatusTmpFilename = "/tmp/json/miner-status-tmp.json";
	const char* StatusFilename = "/tmp/json/miner-status.json";
	const char* ASICStatusFilename = "/tmp/json/ASIC-status.json";

	const char* MasterStatusFilename = "/tmp/json/master-status.json";
	const char* MasterStatusTmpFilename = "/tmp/json/master-status.tmp";
	const char* ChainStatusFilename = "/tmp/json/chain-status.json";
	const char* ChainStatusTmpFilename = "/tmp/json/chain-status.tmp";
	const char* PSUStatusFilename = "/tmp/json/psu-status.json";
	const char* PSUStatusTmpFilename = "/tmp/json/psu-status.tmp";
	const char* FanStatusFilename = "/tmp/json/fan-status.json";
	const char* FanStatusTmpFilename = "/tmp/json/fan-status.tmp";
	const char* PoolStatusFilename = "/tmp/json/pool-status.json";
	const char* PoolStatusTmpFilename = "/tmp/json/pool-status.tmp";
	const char* MinerInfoFilename = "/tmp/json/miner-info.json";
	const char* MinerInfoTmpFilename = "/tmp/json/miner-info.tmp";
	
	const char* ConfigFilename = "/opt/json/miner-config.json";
	const char* OSCConfigFilename = "/tmp/json/osc-config.json";
	const char* BinConfigRdFilename = "/tmp/json/bin-config.json";
	const char* BinConfigRdTmpFilename = "/tmp/json/bin-config.tmp";
	const char* BinConfigFilename = "/tmp/json/bin-config-temp.json";
	const char* HashSNConfigFilename = "/tmp/json/hashsn-config.json";
	const char* PsuWorkCondFilename = "/opt/json/psu-work-cond.json";
	const char* TestConfFilename = "/tmp/json/test-config.json";
	const char* FanRpmFilename ="/tmp/json/fan-speed-config.json";
};
#endif

