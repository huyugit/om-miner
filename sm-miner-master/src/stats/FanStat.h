#ifndef FANSTAT_H
#define FANSTAT_H
#include "stats/StatCommon.h"

#define FAN_NUM		6
#define HSB_TEMP_LOW 35
#define HSB_TEMP_HIGH 65
#define FAN_SPEED_BASE 2500
#define FAN_CTL_STEP 120
#define PWM_CTL_BASE 102
#define PWM_FULL_SPEED 255
#define PWM_CTL_STEP 5
#define AUTO_SPEED_MODE 1
#define FAN_FULL_SPEED 0

class FanStat
{
public:
	FanStat();

	void setFanVoltage(void);
	void setFanCurrent(void);
	void setFanRPMSpeed(void);
	void setFanPWMValue(void);
	int getFanRPMSpeed(int id);
	int getFanPWMValue(int id);
	void setFanFaultStat(void);
	int getFanFaultStat(int id);
	int setFanPWM(int pwm);
	int getFanPWM(int fan_id);
	int getFanRPMinSpeed(void);
	
private:
	int fan_hwid;		//detect max31790 hwmonX in sysfs
	FILE *fan_speed_fd[FAN_NUM];
	FILE *fan_pwm_fd[FAN_NUM];
	FILE *fan_fault_fd[FAN_NUM];
	uint32_t RPM_Speed[FAN_NUM];
	uint32_t PWM_Value[FAN_NUM];
	bool fan_fault[FAN_NUM];
	double fan_voltage;
	double fan_current;
};
extern FanStat g_fanstat;
#endif
