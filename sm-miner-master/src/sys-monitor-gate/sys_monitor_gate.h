#ifndef SYS_MONITOR_GATE_H
#define SYS_MONITOR_GATE_H

#include "mp_packet.h"
#include "ms_defines.h"
#include "json/json-forwards.h"
#include "json/json.h"


typedef struct {
	MST_MON_ITEM hash_temp[MAX_BOARD_PER_SLAVE];
	MST_MON_ITEM fan_current;
	MST_MON_ITEM fan_voltage;
	MST_MON_ITEM fan_fault[6];
	MST_MON_ITEM psu_current[MAX_PSU_PER_SLAVE];
	MST_MON_ITEM psu_voltage[MAX_PSU_PER_SLAVE];
	MST_MON_ITEM psu_power[MAX_PSU_PER_SLAVE];
}MINER_ERR_LEVEL_T;


class SysMonitorGate
{
public:
	SysMonitorGate();
	void SysMonitorRunPollingIteration(void);
	
private:
	void SysMonitorRunTxRx(void);
	void SysMonitorPrepareTxBuffer(void);
	void SysMonitorHashTemp(void);
	void SysMonitorFan(void);
	void SysMonitorPower(void);
	void SysMonitorResetErrLevel(void);
	void SysMonitorUpdateErrLevel(void);

	char sm_buff[MONITOR_ITEM_MAX*2];
	MINER_ERR_LEVEL_T sm_miner_err_level;
	
	char sys_err_level;
	char tmp_sys_err_level;
	Json::Value hash_temp_root;
	Json::Value fan_root;
	Json::Value power_root;
	Json::Value root;
};

#endif
