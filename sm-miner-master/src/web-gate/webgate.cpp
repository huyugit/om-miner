#include <time.h>

#include "format.hpp"
#include "version.h"
#include "app/Application.h"
#include "ms-protocol/ms_packet.h"
#include "pool/StratumPool.h"
#include "env/EnvManager.h"
#include "hw/GpioManager.h"
#include "stats/MasterStat.h"
#include "stats/hist/HistoryStatCalc.h"
#include "base/DateTimeStr.h"
#include "sys/NetInfoUtil.h"
#include "stats/FanStat.h"


#include <fstream>

#include "webgate.h"
#include "webjson.h"


WebGate g_webgate;

void MinerInsertFileTEST(char* file, std::string json_str)
{
	std::ofstream ofs;
	ofs.open(file);

	ofs << json_str;
	ofs.close();
}

void WebGate::init(void)
{
	Json::Value root;

	root["id"] = "00001";
	root["type"] = "A1";
	root["swversion"] = swVersion;
	root["swVersionDate"] = swVersionDate;
	
}

void WebGate::PushMinerStatus(unsigned int mask)
{
	if(mask & MASTER_STATUS_MASK)		PushMinerMasterStatus();
	if(mask & MINER_CHAIN_MASK)			PushMinerChainStatus();
	if(mask & MINER_PSU_STATUS_MASK)	PushMinerPSUStatus();
	if(mask & MINER_FAN_STATUS_MASK)	PushMinerFanStatus();
	if(mask & MINER_POOL_STATUS_MASK)	PushMinerPoolStatus();
	if(mask & MINER_INFO_MASK)			PushMinerInfo();
	if(mask & MINER_BIN_MASK)			PushMinerBinStatus();
}

void WebGate::PushASICStatus(void)
{
	Json::Value root;
	Json::StyledWriter swriter;
	std::string str_writer;
	Webjson MinerJson;

	root = ASICStatus();
	
	str_writer = swriter.write(root);
	MinerJson.ASICInsertToFile(str_writer);
}

void WebGate::PushMinerMasterStatus(void)
{
	Json::Value masterroot;
	Json::StyledWriter swriter;
	std::string str_writer;
	Webjson MinerJson;

	masterroot = MasterStatus();
	str_writer = swriter.write(masterroot);
	MinerJson.MasterStatusInsertToFile(str_writer);
}

void WebGate::PushMinerChainStatus(void)
{
	Json::Value chainroot;
	Json::Value root;
	Json::StyledWriter swriter;
	std::string str_writer;
	Webjson MinerJson;
	int id;

	for(id = 0; id<6; id++)
	{
		BoardStat &board = g_masterStat.getSlave(0).getBoardStat(id);
		if (!board.isFound()) continue;

		chainroot.append(ChainStatus(0, id));
	}
	
	root = chainroot;
	str_writer = swriter.write(root);
	MinerJson.ChainStatusInsertToFile(str_writer);
}

void WebGate::PushMinerBinStatus(void)
{
	Json::Value chainroot;
	Json::Value root;
	Json::StyledWriter swriter;
	std::string str_writer;
	Webjson MinerJson;
	int id;
	uint8_t boardCnt = 0;
	uint8_t lastChipLevel = 0;
	uint8_t chipLevel = 0x0;
	uint8_t chipLevelSum = 0x0;
	char slot[6][9] = {"slot0bin", "slot1bin", "slot2bin", "slot3bin", "slot4bin", "slot5bin"};

	for(id = 0; id<6; id++)
	{
		BoardStat &board = g_masterStat.getSlave(0).getBoardStat(id);
		if (!board.isFound()) continue;

		chainroot[slot[id]] = board.info.boardBin;
		/* find highest bin level */
		if(board.info.boardBin > chipLevel){
			chipLevel = board.info.boardBin;
		}
		boardCnt++;
		chipLevelSum += board.info.boardBin;
		lastChipLevel = board.info.boardBin;
	}

	/* Following printf information will be used in start.pl, 
	 * any changes should be sync to start.pl
	 */
	if(boardCnt == 0){
		printf("NoBoardPlug\n");
	}else if(boardCnt == 1){
		printf("ChipLevel%d\n", chipLevel);
	}else{
		if(lastChipLevel*boardCnt != chipLevelSum){
			printf("ChipMixed\n");
		}else{
			printf("ChipLevel%d\n", chipLevel);
		}
	}
	
	root = chainroot;
	str_writer = swriter.write(root);
	MinerJson.BinStatusInsertToFile(str_writer);
}

void WebGate::PushMinerPSUStatus(void)
{
	Json::Value PSUroot;
	Json::Value root;
	Json::StyledWriter swriter;
	std::string str_writer;
	Webjson MinerJson;
	int id;

	for(id = 0; id<2; id++)
	{
		PSUroot.append(PsuStatus(0, id));
	}

	root = PSUroot;
	str_writer = swriter.write(root);
	MinerJson.PSUStatusInsertToFile(str_writer);
}

void WebGate::PushMinerFanStatus(void)
{
	Json::Value fanroot;
	Json::StyledWriter swriter;
	std::string str_writer;
	Webjson MinerJson;

	fanroot = FanStatus();
	str_writer = swriter.write(fanroot);
	MinerJson.FanStatusInsertToFile(str_writer);
}

void WebGate::PushMinerPoolStatus(void)
{
	Json::Value poolroot;
	Json::StyledWriter swriter;
	std::string str_writer;
	Webjson MinerJson;

	poolroot = PoolStatus();
	str_writer = swriter.write(poolroot);
	MinerJson.PoolStatusInsertToFile(str_writer);
}

void WebGate::PushMinerInfo(void)
{
	Json::Value inforoot;
	Json::StyledWriter swriter;
	std::string str_writer;
	Webjson MinerJson;

	inforoot = MinerInfo();
	str_writer = swriter.write(inforoot);
	MinerJson.MinerInfoInsertToFile(str_writer);
}


void WebGate::PullMinerConfig(void)
{
	Json::Value root;
	Webjson minerconf;

	minerconf.MinerParsefromFile(root);
	PoolSetting(root);
}

void WebGate::PullPsuWorkCondConfig(void)
{
	int ret = 0;
	Json::Value root;
	Webjson minerconf;

	ret = minerconf.PsuWorkCondParsefromFile(root);
	if(ret != -1){
		psuWorkCond.changedFlag = 0;

		if(root.isMember("PsuPoutLowTh")){
			psuWorkCond.PsuPoutLowTh = root.get("PsuPoutLowTh", 0).asDouble();
		}
		
		if(root.isMember("PsuPoutHighTh")){
			psuWorkCond.PsuPoutHighTh = root.get("PsuPoutHighTh", 0).asDouble();
		}
		
		if(root.isMember("PsuIoutLowTh")){
			psuWorkCond.PsuIoutLowTh = root.get("PsuIoutLowTh", 0).asDouble();
		}

		if(root.isMember("PsuIoutHighTh")){
			psuWorkCond.PsuIoutHighTh = root.get("PsuIoutHighTh", 0).asDouble();
		}

		if(root.isMember("PsuVoutHighTh")){
			psuWorkCond.PsuVoutHighTh = root.get("PsuVoutHighTh", 0).asDouble();
		}
		if(root.isMember("PsuFanSpeedLowTh")){
			psuWorkCond.PsuFanSpeedLowTh = (uint16_t)(root.get("PsuFanSpeedLowTh", 0).asUInt());
		}

		if(root.isMember("PsuTempOutHighTh")){
			psuWorkCond.PsuTempOutHighTh = (uint8_t)(root.get("PsuTempOutHighTh", 0).asUInt());
		}

		if(root.isMember("FanRpmLowTh")){
			psuWorkCond.FanRpmLowTh = (uint16_t)(root.get("FanRpmLowTh", 0).asUInt());
		}

		if(root.isMember("HashTempFlag")){
			psuWorkCond.HashTempFlag = (uint8_t)(root.get("HashTempFlag", 0).asUInt());
		}

		if(root.isMember("HashTempHi")){
			psuWorkCond.HashTempHi = (int8_t)(root.get("HashTempHi", 0).asUInt());
		}

		if(root.isMember("PsuFlag")){
			psuWorkCond.PsuFlag = (uint8_t)(root.get("PsuFlag", 0).asUInt());
		}

		if(root.isMember("FanFlag")){
			psuWorkCond.FanFlag = (uint8_t)(root.get("FanFlag", 0).asUInt());
		}

		if(root.isMember("FanWorkMode")){
			psuWorkCond.FanWorkMode = (uint8_t)(root.get("FanWorkMode", 0).asUInt());
		}
		psuWorkCond.changedFlag = 1;
	}else{
		psuWorkCond.changedFlag = 0;
	}
}

void WebGate::PullOscConfig(void)
{
	Json::Value root;
	Webjson minerconf;
	int ret = 0;
	int oscSum = 0;
	char slot[6][6] = {"slot0", "slot1", "slot2", "slot3", "slot4", "slot5"};
	PrevHstOsc = hstboardOSC;

	hstboardOSC.changedFlag = 0x00;
	ret = minerconf.OscParsefromFile(root);
	if(ret != -1)
	{
		for(int board_id = 0; board_id < MAX_BOARD_PER_SLAVE; board_id++){
			hstboardOSC.HashBoardOsc[board_id] = root.get(slot[board_id], 0).asInt();
			if(hstboardOSC.HashBoardOsc[board_id] != 0)	{
				oscSum += hstboardOSC.HashBoardOsc[board_id];
				if(PrevHstOsc.HashBoardOsc[board_id] != hstboardOSC.HashBoardOsc[board_id])
				{
					hstboardOSC.changedFlag = hstboardOSC.changedFlag | (0x01 << board_id);
				}
			}
		}

		/* Set power down flag to slave */
		if(oscSum == 0){
			hstboardOSC.changedFlag = 0xff;
		}
	}
	else
	{
		hstboardOSC.changedFlag = 0x00;
	}

	printf("hstboardOSC.changedFlag = %d\n", hstboardOSC.changedFlag);
}

void WebGate::PullBinConfig(void)
{
	Json::Value root;
	Webjson minerconf;
	int ret = 0;
	char slot[6][9] = {"slot0bin", "slot1bin", "slot2bin", "slot3bin", "slot4bin", "slot5bin"};

	hstboardBin.changedFlag = 0x00;
	ret = minerconf.BinParsefromFile(root);
	if(ret != -1)
	{
		for(int board_id = 0; board_id < MAX_BOARD_PER_SLAVE; board_id++){
			hstboardBin.HashBoardBin[board_id] = (uint8_t)(root.get(slot[board_id], 0).asInt());
			hstboardBin.changedFlag = hstboardBin.changedFlag | (0x01 << board_id);
		}
	}

	printf("hstboardBin.changedFlag = %d\n", hstboardBin.changedFlag);
}

void WebGate::PullHashSNConfig(void)
{
	Json::Value root;
	Webjson minerconf;
	int ret = 0;
	std::string sn_str;
	char slot[6][6] = {"slot0", "slot1", "slot2", "slot3", "slot4", "slot5"};

	hashSN.changedFlag = 0x00;
	ret = minerconf.HashSNParsefromFile(root);

	if(ret != -1)
	{
		for(int board_id = 0; board_id < MAX_BOARD_PER_SLAVE; board_id++){
			sn_str = root.get(slot[board_id], 0).asString();
			strcpy(hashSN.HashBoardSN[board_id], sn_str.c_str());
			if(hashSN.HashBoardSN[board_id][0] != 0) {
				hashSN.changedFlag = hashSN.changedFlag | (0x01 << board_id);
			}			
		}
	}
	else
	{
		hashSN.changedFlag = 0x00;
	}
}

void WebGate::PullFanRpmConfig(void)
{
	Json::Value root;
	Webjson minerconf;
	int ret = 0;
	std::string sn_str;
	char fanRpm[6][6] = {"one", "two", "three", "four", "five", "six"};

	ret = minerconf.FanRpmParsefromFile(root);

	if(ret != -1)
	{
		printf("Target fan RPM:	");
		for(int fanId = 0; fanId < FAN_NUM; fanId++){
			fanRpmCfg[fanId] = (uint32_t)(root.get(fanRpm[fanId], 0).asInt());
			printf("fanRpmCfg[%d] = %d ", fanId, fanRpmCfg[fanId]);
		}
		printf("\n");
	}else{
		for(int fanId = 0; fanId < FAN_NUM; fanId++){
			fanRpmCfg[fanId] = 0;
		}
	}
}

Config WebGate::GetMinerConfig(Config& appConfig)
{
	Config newMinerConf;

	PullMinerConfig();

	minerconfig = appConfig;

	minerconfig.poolConfig.host = webpoolset.host;
	minerconfig.poolConfig.port = webpoolset.port;
	minerconfig.poolConfig.userName = webpoolset.userName;
	minerconfig.poolConfig.password = webpoolset.password;
#ifdef MULTI_POOL_SUPPORT
	// fcj add begin 20180313
	// bak1 pool
	minerconfig.poolConfig.bak1Host = webpoolset.bak1Host;
	minerconfig.poolConfig.bak1Port = webpoolset.bak1Port;
	minerconfig.poolConfig.bak1UserName = webpoolset.bak1UserName;
	minerconfig.poolConfig.bak1Password = webpoolset.bak1Password;

	// bak2 pool
	minerconfig.poolConfig.bak2Host = webpoolset.bak2Host;
	minerconfig.poolConfig.bak2Port = webpoolset.bak2Port;
	minerconfig.poolConfig.bak2UserName = webpoolset.bak2UserName;
	minerconfig.poolConfig.bak2Password = webpoolset.bak2Password;
	// fcj add end
#endif
	PullOscConfig();  //chenbo add for osc config
	PullHashSNConfig();
	PullBinConfig();
	PullPsuWorkCondConfig();	//gzh add for psu work condition config
	return newMinerConf;
}

Json::Value WebGate::ASICStatus(void)
{
	Json::Value root;
	Json::Value chiproot;
	
	for (int slaveId = -1; EnvManager::nextSlaveId(slaveId); )
    {
    	for (int boardId = 0; boardId < MAX_BOARD_PER_SLAVE; boardId++)
		{
			BoardStat &board = g_masterStat.getSlave(0).getBoardStat(boardId);
			if (!board.isFound()) continue;
			
			for (int spiId = 0; spiId < g_masterStat.getSlave(slaveId).getBoardStat(boardId).spiNum(); spiId++)
	        {
	            for (int chipId = 0; chipId < g_masterStat.getSlave(slaveId).getBoardStat(boardId).spiLen(); chipId++)
	            {
			    	PwcStat& pwcChipst = g_masterStat.getSlave(slaveId).getPwcStat(boardId, spiId ,chipId);

					for (PwcBtcIterator it(pwcChipst); it.next(); )
			        {
						ChipStat chip = it.getChipStat();
						TotalStat totalStat(chip);
        				HistoryStatCalc calc(totalStat, 1);

						chiproot["boardId"] = boardId;

						chiproot["spiSeq"] = chip.spiSeq;
						chiproot["pwcSeq"] = chip.pwcSeq;
						chiproot["osc"] = chip.osc;

						chiproot["SOL"] = calc.getSol();
						chiproot["ERR"] = calc.getErr();
						chiproot["E/S"] = calc.getErrToSol();
						chiproot["SPEED"] = calc.getGHs();

						root.append(chiproot);
			        }
            	}
			}
		}
    }

	return root;
}

Json::Value WebGate::chainASICStatus(BoardStat& hashboard)
{
	Json::Value root;
	Json::Value chiproot;
	
	for (int spiId = 0; spiId < hashboard.spiNum(); spiId++)
	{
        for (int chipId = 0; chipId < hashboard.spiLen(); chipId++)
        {
	    	PwcStat& pwcChipst = hashboard.getPwcStat(spiId ,chipId);

			for (PwcBtcIterator it(pwcChipst); it.next(); )
	        {
				ChipStat chip = it.getChipStat();
				TotalStat totalStat(chip);
				HistoryStatCalc calc(totalStat, 1);

				chiproot["spiSeq"] = chip.spiSeq;
				chiproot["pwcSeq"] = chip.pwcSeq;
				//chiproot["osc"] = chip.osc;

				chiproot["SOL"] = calc.getSol();
				//chiproot["ERR"] = calc.getErr();
				//chiproot["E/S"] = calc.getErrToSol();
				chiproot["SPEED"] = calc.getGHs();

				root.append(chiproot);
	        }
    	}
	}
	
	//root = chiproot;
	return root;
}



Json::Value WebGate::MinerStatus(void)
{
	Json::Value root;
	Json::Value chain;
	Json::Value psu;

	int id;

	for(id = 0; id<6; id++)
	{
		BoardStat &board = g_masterStat.getSlave(0).getBoardStat(id);
		if (!board.isFound()) continue;

		chain.append(ChainStatus(0, id));
	}
	root["chain"] = chain;

	for(id = 0; id<2; id++)
	{
		psu.append(PsuStatus(0, id));
	}
	root["psu"] = psu;
	root["fan"] = FanStatus();
	root["pool"] = PoolStatus();
	root["version"] = SWversionStatus();
	root["master"] = MasterStatus();

	return root;
}

Json::Value WebGate::SWversionStatus(void)
{
	Json::Value verroot;
	char buffer[8];

	verroot["model"] = model;
	verroot["miner"] = swVersion;
	verroot["minerDate"] = swVersionDate;
	snprintf(buffer, sizeof(buffer), "0x%04x", MsPacket::VERSION);
	verroot["mspVer"] = buffer;

	return verroot;
}

Json::Value WebGate::ChainStatus(int slaveId, int boardId)
{
	Json::Value root;
	Json::Value chainroot;
	char buffer[8];
	
	BoardStat &board = g_masterStat.getSlave(slaveId).getBoardStat(boardId);
	//if (!board.isFound()) continue;

	HistoryStatCalc calc(board.getBoardTotal(), HistoryStatMgr::PERIOD_DELTA);

	snprintf(buffer, sizeof(buffer), "%d", board.getNum());

	chainroot["slotId"] = buffer;
	chainroot["revision"] = board.spec.revisionId;
	chainroot["spiNum"] = board.spec.spiNum;
	chainroot["spiLen"] = board.spec.spiLen;
	chainroot["pwrNum"] = board.spec.pwrNum;
	chainroot["pwrLen"] = board.spec.pwrLen;
	chainroot["btcNum"] = board.spec.btcNum;
	chainroot["specVoltage"] = board.spec.voltage;
	chainroot["chips"] = board.getBoardTotal().chips;
	
	int8_t temperature = board.info.boardTemperature[0];
	chainroot["temperature"] = temperature;
	int8_t temperature1 = board.info.boardTemperature[1];
	chainroot["temperature1"] = temperature1;
	
	chainroot["ocp"] = board.info.overCurrentProtection;
	chainroot["heaterErr"] = board.info.heaterErr;
	chainroot["heaterErrNum"] = board.info.heaterErrNum;

	chainroot["overheats"] = board.info.ohNum;
	chainroot["overheatsTime"] = board.info.ohTime / 1000;
	chainroot["lowCurrRst"] = board.info.lowCurrRst;
	chainroot["voltage"] = board.info.voltage;
	chainroot["osc"] = board.info.boardOSC;

	chainroot["hashSN"] = board.info.hashSN;

	chainroot["brokenPwc"] = board.getBoardTotal().numBrokenPwc;
	chainroot["solutions"] = calc.getSol();
	chainroot["errors"] = calc.getErr();
	chainroot["ghs"] = calc.getGHs();
	chainroot["errorRate"] = calc.getErrToSol();
	chainroot["chipRestarts"] = calc.getChipRestarts();
	chainroot["wattPerGHs"] = util::safeDiv(board.getBoardTotal().power, calc.getGHs());


	for (int i = 0; i < board.spec.pwrNum; i++)
	{
		chainroot["currents"].append(board.info.currents[i]);
		chainroot["power"].append(board.info.currents[i] * board.info.voltage /1000/1000);
	}

	chainroot["ICStatus"] = chainASICStatus(board);

	Json::Value tmpAlert;
	for (int i = 0; i < MAX_TMP_PER_BOARD; i++)
	{
		Json::Value Alert;
		Alert["alertLo"] = board.info.taInfo[i].alertLo;
		Alert["alertHi"] = board.info.taInfo[i].alertHi;
		Alert["numWrite"] = board.info.taInfo[i].numWrite;

		tmpAlert.append(Alert);
	}
	chainroot["tmpAlert"] = tmpAlert;
	
	//root[buffer] = chainroot;
	
	//return root;
	return chainroot;
}

Json::Value WebGate::PsuStatus(int slaveId, int psuId)
{
	Json::Value psuroot;
	char buffer[32];
    PsuInfo &pi = g_masterStat.getSlave(slaveId).psuInfo[psuId];
    PsuSpec &ps = g_masterStat.getSlave(slaveId).psuSpec[psuId];
	
	psuroot["id"] = pi.id;
	psuroot["serial"] = pi.serial;//EltekSerialToStr(pi.serial).str;
	psuroot["vOut"] = pi.getVOut();
	psuroot["iOut"] = pi.getIOut();
	psuroot["pOut"] = pi.getPOut();
	psuroot["vIn"] = pi.getVIn();
	psuroot["cond"] = pi.condition;

	int8_t tin = pi.tempIn;
	psuroot["tIn"] = tin;

	int8_t tOut = pi.tempOut;
	psuroot["tOut"] = tOut;

	psuroot["numSt"] = pi.numStatus;
	psuroot["lstStT"] = pi.lastStatusTime;

	psuroot["minAlarm"] = pi.minorAlarm;
	psuroot["majAlarm"] = pi.majorAlarm;

	psuroot["defV"] = pi.defaultVoltage;

	psuroot["ledG"] = pi.greenLed;
	psuroot["ledY"] = pi.yellowLed;
	psuroot["ledR"] = pi.redLed;

	psuroot["ut"] = pi.upTime;
	psuroot["fsr"] = pi.fanSpeedRef;
	psuroot["fs"] = pi.fanSpeed;

	Json::Value product;
	product["part"] = ps.prodPart;
	product["ver"] = ps.prodVer;

	snprintf(buffer, sizeof(buffer), "%04u-%02u-%02u", ps.prodYear, ps.prodMonth, ps.prodDay);
	product["date"] = buffer;
	product["desc"] = ps.prodDesc;

	psuroot["product"] = product;

	return psuroot;

}

Json::Value WebGate::FanStatus(void)
{
	Json::Value fanroot;

	FanConfig &fanConfig = Application::configRW().slaveConfig.fanConfig;
    FanInfo &fanInfo = g_masterStat.getSlave(0).cmnInfo.mbInfo.fan;

	fanroot["cfgU"] = fanConfig.getFanVoltage();
	fanroot["fanU"] = fanInfo.getFanU();
	fanroot["fanI"] = fanInfo.getFanI();

	for(int id = 0; id < FAN_NUM; id++)
	{
		fanroot["speed"].append(g_fanstat.getFanRPMSpeed(id));
	}

	for(int id = 0; id < FAN_NUM; id++)
	{
		fanroot["pwm"].append(g_fanstat.getFanPWMValue(id));
	}

	return fanroot;
}

Json::Value WebGate::PoolStatus(void)
{
	Json::Value poolroot;
	//Json::Value intervalsroot;
	char buffer[8];
	StratumPool& pool = Application::pool();
#ifdef MULTI_POOL_SUPPORT
	poolroot["host"] = pool.getCurrentPoolInfo().host.cdata();
	poolroot["port"] = pool.getCurrentPoolInfo().port.cdata();
	poolroot["userName"] = pool.getCurrentPoolInfo().userName.cdata();
#else
	poolroot["host"] = pool.getPoolConfig().host.cdata();
	poolroot["port"] = pool.getPoolConfig().port.cdata();
	poolroot["userName"] = pool.getPoolConfig().userName.cdata();
#endif
	poolroot["diff"] = pool.getCurrentDifficulty();

	for (size_t period = 0; period < HistoryStatMgr::MAX_PERIODS; period++)
	{
		const HistoryStat& periodStat = g_masterStat.getHistory().getByPeriod(period);
		Json::Value interval;
	
		snprintf(buffer, sizeof(buffer), "%d", periodStat.intervalExpected);
		interval["name"] = periodStat.intervalName;
		interval["interval"] = periodStat.getSeconds();
		interval["jobs"] = periodStat.poolReceivedJobs;
		interval["cleanFlags"] = periodStat.poolReceivedJobsWithClean;
		interval["sharesSent"] = periodStat.poolSentShares;
		interval["sharesAccepted"] = periodStat.poolAcceptedShares;
		interval["sharesRejected"] = periodStat.poolRejectedShares;
		interval["solutionsAccepted"] = periodStat.poolAcceptedSolutions;
		interval["shareLoss"] = periodStat.getShareLoss();

		interval["poolTotal"] = static_cast<uint32_t>(periodStat.poolTotalTime / 1000);
		interval["inService"] = static_cast<uint32_t>(periodStat.poolInService / 1000);
		
		interval["subscribeError"] = periodStat.poolSubscribeError;
		interval["diffChanges"] = periodStat.poolDiffChanges;
		interval["reconnections"] = periodStat.poolReconnections;
		interval["reconnectionsOnErrors"] = periodStat.poolReconnectionsOnError;
		
		interval["defaultJobShares"] = periodStat.poolDefaultJobShares;
		interval["staleJobShares"] = periodStat.poolStaleJobShares;
		interval["duplicateShares"] = periodStat.poolDuplicateShares;
		interval["lowDifficultyShares"] = periodStat.poolLowDifficultyShares;

		interval["pwcSharesSent"] = periodStat.pwcSharesSent;
		interval["pwcSharesDropped"] = periodStat.pwcSharesDropped;

		poolroot["intervals"].append(interval);
	}

	return poolroot;
}

Json::Value WebGate::MinerInfo(void)
{
	Json::Value inforoot;
	char buffer[8];

	inforoot["model"] = model;
	inforoot["miner"] = swVersion;
	inforoot["minerDate"] = swVersionDate;
	snprintf(buffer, sizeof(buffer), "0x%04x", MsPacket::VERSION);
	inforoot["mspVer"] = buffer;

	inforoot["mbHwVer"] = g_gpioManager.mbHwVer;
	inforoot["ip"] = util::ipAddressToString(util::getIpAddress("eth0")).cdata();
	inforoot["mac"] = util::macAddressToString(util::getMacAddress("eth0")).cdata();

	return inforoot;
}

Json::Value WebGate::MasterStatus(void)
{
	Json::Value masterroot;
	//Json::Value intervalsroot;
	char buffer[8];

	masterroot["upTime"] = g_masterStat.statIntervalTotal;
	masterroot["mbHwVer"] = g_gpioManager.mbHwVer;
	masterroot["diff"] = Application::pool().getCurrentDifficulty();
	masterroot["boards"] = g_masterStat.total.boards;
	masterroot["errorSpi"] = g_masterStat.total.numBrokenSpi;
	masterroot["osc"] = Application::config()->pwcConfig.osc;
	masterroot["ipWhite"] = util::ipAddressToString(util::getIpAddress("eth0")).cdata();
	masterroot["ipGray"] = util::ipAddressToString(util::getIpAddress("eth0")).cdata();
	masterroot["hwAddr"] = util::macAddressToString(util::getMacAddress("eth0")).cdata();

	HistoryStatCalc calc(HistoryStatMgr::PERIOD_TOTAL);
    double wghs = util::safeDiv(g_masterStat.total.psuPower, calc.getGHs());
	masterroot["voltSet"] = Application::configRW().psuConfig.getVoltage();
	masterroot["psuI"] = g_masterStat.total.psuCurrent;
	masterroot["psuW"] = g_masterStat.total.psuPower;

	masterroot["boardsI"] = g_masterStat.total.currentA / 1000;
	masterroot["boardsW"] = g_masterStat.total.power;
	masterroot["wattPerGHs"] = wghs;

	for (size_t period = 0; period < HistoryStatMgr::MAX_PERIODS; period++)
	{
		HistoryStatCalc calc(period);
		Json::Value interval;

		snprintf(buffer, sizeof(buffer), "%d", calc.getStat().intervalExpected);

		interval["name"] = calc.getStat().intervalName;
		interval["interval"] = calc.getSeconds();
		interval["bySol"] = calc.getGHs();
		interval["byDiff"] = calc.getGHsByDiff();
		interval["byPool"] = calc.getGHsByPool();
		interval["byJobs"] = calc.getGHsByJobs();
		interval["solutions"] = calc.getSol();
		interval["errors"] = calc.getErr();
		interval["errorRate"] = calc.getErrToSol();
		interval["chipSpeed"] = calc.getChipGHs();
		interval["chipRestarts"] = calc.getChipRestarts();

		//intervalsroot[buffer] = interval;
		masterroot["intervals"].append(interval);
	}
	//masterroot["intervals"].append(intervalsroot);

	return masterroot;
}

void WebGate::PoolSetting(Json::Value poolset)
{
	webpoolset.host = poolset["url"].asCString();
	webpoolset.port = poolset["port"].asCString();
	webpoolset.userName = poolset["worker"].asCString();
	webpoolset.password = poolset["password"].asCString();
#ifdef MULTI_POOL_SUPPORT
	// fcj add begin 20180313
	// bak1 pool
	if(poolset.isMember("bak1_url")){
		webpoolset.bak1Host = poolset["bak1_url"].asCString();
		webpoolset.bak1Port = poolset["bak1_port"].asCString();
		webpoolset.bak1UserName = poolset["bak1_worker"].asCString();
		webpoolset.bak1Password = poolset["bak1_password"].asCString();
	}else{
		webpoolset.bak1Host = poolset["url"].asCString();
		webpoolset.bak1Port = poolset["port"].asCString();
		webpoolset.bak1UserName = poolset["worker"].asCString();
		webpoolset.bak1Password = poolset["password"].asCString();
	}

	// bak2 pool
	if(poolset.isMember("bak2_url")){	
		webpoolset.bak2Host = poolset["bak2_url"].asCString();
		webpoolset.bak2Port = poolset["bak2_port"].asCString();
		webpoolset.bak2UserName = poolset["bak2_worker"].asCString();
		webpoolset.bak2Password = poolset["bak2_password"].asCString();
	}else{
		webpoolset.bak2Host = poolset["url"].asCString();
		webpoolset.bak2Port = poolset["port"].asCString();
		webpoolset.bak2UserName = poolset["worker"].asCString();
		webpoolset.bak2Password = poolset["password"].asCString();
	}
	// fcj add end
#endif
}

