#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "stats/MasterStat.h"
#include "sys_monitor_gate.h"
#include "stats/FanStat.h"


#include <utility>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <fstream>
#include <istream>
#include <sstream>
#include <memory>
#include <set>
#include <limits>


SysMonitorGate::SysMonitorGate()
{
	sys_err_level = 0;
	tmp_sys_err_level = 0;
}

void SysMonitorGate::SysMonitorRunPollingIteration(void)
{
	memset(&sm_miner_err_level, 0, sizeof(MINER_ERR_LEVEL_T));

	SysMonitorResetErrLevel();
	SysMonitorHashTemp();
	SysMonitorFan();
	SysMonitorPower();
	SysMonitorUpdateErrLevel();
	SysMonitorPrepareTxBuffer();
	SysMonitorRunTxRx();
}

void SysMonitorGate::SysMonitorRunTxRx(void)
{
	std::ofstream ofs;
	Json::StyledWriter swriter;
	std::string str_writer;
	
	str_writer = swriter.write(root);

	ofs.open("/tmp/json/sysmonitor.json");

	ofs << str_writer;
	ofs.close();
}

void SysMonitorGate::SysMonitorPrepareTxBuffer(void)
{
	char buffer[8];
	
	root["hashTEMP"] = hash_temp_root;
	root["fan"] = fan_root;
	root["power"] = power_root;
	
	snprintf(buffer, sizeof(buffer), "error%d", sys_err_level);
	root["syserrlevel"] = buffer;
}

void SysMonitorGate::SysMonitorHashTemp(void)
{
	int id;
	int8_t temp1;
	int8_t temp2;
	Json::Value temp_root;
	
	for(id = 0; id<MAX_BOARD_PER_SLAVE; id++)
	{
		sm_miner_err_level.hash_temp[id].item_id = hash_bd1_temp + id;
		
		BoardStat &board = g_masterStat.getSlave(0).getBoardStat(id);
		if (!board.isFound()) continue;

		temp1 = board.info.boardTemperature[0];
		temp2 = board.info.boardTemperature[1];

		if((temp1 >= 80)||(temp2 >= 80))
		{
			sm_miner_err_level.hash_temp[id].error_level = ERR_LEVEL_2;
			tmp_sys_err_level = ERR_LEVEL_2;
		}
		else
		{
			sm_miner_err_level.hash_temp[id].error_level = ERR_LEVEL_0;
		}

		temp_root.append(sm_miner_err_level.hash_temp[id].error_level);
	}
	hash_temp_root = temp_root;
}

void SysMonitorGate::SysMonitorFan(void)
{
	FanInfo &fanInfo = g_masterStat.getSlave(0).cmnInfo.mbInfo.fan;
	int fan_id;
	Json::Value fault_root;

	sm_miner_err_level.fan_current.item_id = fan_I;
	if(fanInfo.getFanI() >= 20)
	{
		sm_miner_err_level.fan_current.error_level = ERR_LEVEL_2;
		tmp_sys_err_level = ERR_LEVEL_2;
	}
	else
	{
		sm_miner_err_level.fan_current.error_level = ERR_LEVEL_0;
	}

	fan_root["fan_I"] = sm_miner_err_level.fan_current.error_level;

	sm_miner_err_level.fan_voltage.item_id = fan_V;

	if(fanInfo.getFanU() >= 13)
	{
		sm_miner_err_level.fan_voltage.error_level = ERR_LEVEL_2;
		tmp_sys_err_level = ERR_LEVEL_2;
	}
	else
	{
		sm_miner_err_level.fan_voltage.error_level = ERR_LEVEL_0;
	}

	fan_root["fan_V"] = sm_miner_err_level.fan_voltage.error_level;

	for(fan_id = 0; fan_id < FAN_NUM; fan_id++)
	{
		sm_miner_err_level.fan_fault[fan_id].item_id = fan1_fault + fan_id;
		sm_miner_err_level.fan_fault[fan_id].error_level = g_fanstat.getFanFaultStat(fan_id);
		/* For unknown reason, the fault value may be zero when the fan stopped,
			so do the speed check here will keep machine more safe */
		if(sm_miner_err_level.fan_fault[fan_id].error_level ||
		   g_fanstat.getFanRPMSpeed(fan_id) < 2000)
		{
			tmp_sys_err_level = ERR_LEVEL_2;
		}

		fault_root.append(sm_miner_err_level.fan_fault[fan_id].error_level);
	}

	fan_root["fault"] = fault_root;
}

void SysMonitorGate::SysMonitorPower(void)
{
	int id;
	double vOutTh;
	double iOutTh;
	double pOutTh;
	Json::Value tmp_root;
	Json::Value power_tmp_root;
	
	for(id = 0; id<MAX_PSU_PER_SLAVE; id++)
	{
		PsuInfo &pi = g_masterStat.getSlave(0).psuInfo[id];
		
		if(pi.ratedCurrent > 60){
			vOutTh = 52.0;
			iOutTh = 70.0;
			pOutTh = 3200.0;
		}else{
			vOutTh = 51.0;
			iOutTh = 65.0;
			pOutTh = 2950.0;
		}
		
		if(pi.getVOut() >= vOutTh)
		{
			sm_miner_err_level.psu_voltage[id].item_id = power1_V + id*3;
			sm_miner_err_level.psu_voltage[id].error_level = ERR_LEVEL_2;
			tmp_sys_err_level = ERR_LEVEL_2;
		}
		else
		{
			sm_miner_err_level.psu_voltage[id].item_id = power1_V + id*3;
			sm_miner_err_level.psu_voltage[id].error_level = ERR_LEVEL_0;
		}
		tmp_root["power_V"] = sm_miner_err_level.psu_voltage[id].error_level;

		if(pi.getIOut() >= iOutTh)
		{
			sm_miner_err_level.psu_current[id].item_id = power1_I + id*3;
			sm_miner_err_level.psu_current[id].error_level = ERR_LEVEL_2;
			tmp_sys_err_level = ERR_LEVEL_2;
		}
		else
		{
			sm_miner_err_level.psu_current[id].item_id = power1_I + id*3;
			sm_miner_err_level.psu_current[id].error_level = ERR_LEVEL_0;
		}
		tmp_root["power_I"] = sm_miner_err_level.psu_current[id].error_level;

		if(pi.getPOut() >= pOutTh)
		{
			sm_miner_err_level.psu_power[id].item_id = power1_P + id*3;
			sm_miner_err_level.psu_power[id].error_level = ERR_LEVEL_2;
			tmp_sys_err_level = ERR_LEVEL_2;
		}
		else
		{
			sm_miner_err_level.psu_power[id].item_id = power1_P + id*3;
			sm_miner_err_level.psu_power[id].error_level = ERR_LEVEL_0;
		}
		tmp_root["power_P"] = sm_miner_err_level.psu_power[id].error_level;

		power_tmp_root.append(tmp_root);
	}
	power_root = power_tmp_root;
}

void SysMonitorGate::SysMonitorResetErrLevel(void)
{
	tmp_sys_err_level = 0;
}

void SysMonitorGate::SysMonitorUpdateErrLevel(void)
{
	sys_err_level = tmp_sys_err_level;
}

