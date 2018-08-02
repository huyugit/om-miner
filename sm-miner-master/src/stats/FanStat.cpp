#include <stdio.h>
#include "FanStat.h"
#include "stats/MasterStat.h"

FanStat g_fanstat;

FanStat::FanStat()
{
	FILE* name_fd;
	int fan_id;
	int hwmon_id = 0;
	char name_buf[8] = {0};
	char filenamebuff[64] = {0};
	
	fan_hwid = 0;
	for(hwmon_id = 1; hwmon_id < 5; hwmon_id++){
		sprintf(filenamebuff, "/sys/class/hwmon/hwmon%d/device/name", hwmon_id);
		name_fd = fopen(filenamebuff, "r");
		if(!name_fd){
			continue;
		}

		fread(name_buf, 1, sizeof(name_buf), name_fd);
		if(!strncmp(name_buf, "max31790", 8)){
			fan_hwid = hwmon_id;
		}
		fclose(name_fd);
		if(fan_hwid){
			break;
		}
	}
	
	/* open fanX_input files */
	for(fan_id = 0; fan_id < FAN_NUM; fan_id++){
		sprintf(filenamebuff, "/sys/class/hwmon/hwmon%d/device/fan%d_input", fan_hwid, fan_id+1);
		fan_speed_fd[fan_id] = fopen(filenamebuff, "r");
		if(!fan_speed_fd[fan_id]){
			fan_hwid = 0;
			return;
		}
	}

	/* open fanX_fault files */
	for(fan_id = 0; fan_id < FAN_NUM; fan_id++){
		sprintf(filenamebuff, "/sys/class/hwmon/hwmon%d/device/fan%d_fault", fan_hwid, fan_id+1);
		fan_fault_fd[fan_id] = fopen(filenamebuff, "r");
		if(!fan_fault_fd[fan_id]){
			fan_hwid = 0;
			return;
		}
	}

	/* open pwmX files */
	for(fan_id = 0; fan_id < FAN_NUM; fan_id++){
		sprintf(filenamebuff, "/sys/class/hwmon/hwmon%d/device/pwm%d", fan_hwid, fan_id+1);
		fan_pwm_fd[fan_id] = fopen(filenamebuff, "rb+");
		if(!fan_pwm_fd[fan_id]){
			fan_hwid = 0;
			return;
		}
	}
}

void FanStat::setFanVoltage(void)
{
    FanInfo &fanInfo = g_masterStat.getSlave(0).cmnInfo.mbInfo.fan;

	fan_voltage = fanInfo.getFanU();
}

void FanStat::setFanCurrent(void)
{
    FanInfo &fanInfo = g_masterStat.getSlave(0).cmnInfo.mbInfo.fan;

	fan_current = fanInfo.getFanI();
}

void FanStat::setFanRPMSpeed(void)
{
	int fan_id;
	if(fan_hwid > 0){
		for(fan_id = 0; fan_id < FAN_NUM; fan_id++){
			char buffer[8] = {0};
			fseek(fan_speed_fd[fan_id], 0, SEEK_SET);
			if(fread(buffer, 1, sizeof(buffer), fan_speed_fd[fan_id]) > 0) {
				RPM_Speed[fan_id] = atoi(buffer);
			}
		}
	}
}

void FanStat::setFanPWMValue(void)
{
	int fan_id;
	if(fan_hwid > 0){
		for(fan_id = 0; fan_id < FAN_NUM; fan_id++){
			char buffer[8] = {0};
			fseek(fan_pwm_fd[fan_id], 0, SEEK_SET);
			if(fread(buffer, 1, sizeof(buffer), fan_pwm_fd[fan_id]) > 0) {
				PWM_Value[fan_id] = atoi(buffer);
			}
		}
	}
}

int FanStat::getFanRPMSpeed(int id)
{
	return RPM_Speed[id];
}

int FanStat::getFanPWMValue(int id)
{
	return PWM_Value[id];
}

int FanStat::getFanRPMinSpeed(void)
{
	int fan_id, min;
	min = RPM_Speed[0];
	
	for(fan_id = 0; fan_id < FAN_NUM; fan_id++) {
		min = (RPM_Speed[fan_id] < min) ? RPM_Speed[fan_id] : min;
	}
	return min;
}

void FanStat::setFanFaultStat(void)
{
	int fan_id;

	if(fan_hwid > 0){
		for(fan_id = 0; fan_id < FAN_NUM; fan_id++) {
			/* fanX_fault: 0 or 1 */
			char buffer[4] = {0};
			fseek(fan_fault_fd[fan_id], 0, SEEK_SET);
			if(fread(buffer, 1, sizeof(buffer), fan_fault_fd[fan_id]) > 0) {
				fan_fault[fan_id] = atoi(buffer);
				if(1 == fan_fault[fan_id]){
					/* pwm value 0 - 255 */ 
					memset(buffer, 0, 4);
					strcpy(buffer, "255");
					/* if the fan fall into fault status, write the pwm register
					   to clear the fault status if it recover to normal status */
					fwrite(&buffer, 1, 3, fan_pwm_fd[fan_id]);
					rewind(fan_pwm_fd[fan_id]);
				}
			}
		}
	}
}

int FanStat::getFanFaultStat(int id)
{
	return fan_fault[id];
}

int FanStat::setFanPWM(int pwm)
{
	int fan_id;
	char buffer[4] = {0};
	
	if (pwm < PWM_CTL_BASE) {
		pwm = PWM_CTL_BASE;
	} else if (pwm > PWM_FULL_SPEED) {
		pwm = PWM_FULL_SPEED;
	}

	sprintf(buffer, "%d", pwm);
	if(fan_hwid > 0){
		for(fan_id = 0; fan_id < FAN_NUM; fan_id++) {
			fseek(fan_pwm_fd[fan_id], 0, SEEK_SET);
			fwrite(&buffer, 1, 3, fan_pwm_fd[fan_id]);
			fflush(fan_pwm_fd[fan_id]);
			//fsync(fan_pwm_fd[fan_id]);
			rewind(fan_pwm_fd[fan_id]);
		}
	}
	return 0;
}

int FanStat::getFanPWM(int fan_id)
{
	int pwm = 0;
	char buffer[4] = {0};

	sprintf(buffer, "%d", pwm);
	if(fan_hwid > 0){
		if ((fan_id < FAN_NUM) && (fan_id >= 0)) {
			fflush(fan_pwm_fd[fan_id]);
			//fsync(fan_pwm_fd[fan_id]);
			fseek(fan_pwm_fd[fan_id], 0, SEEK_SET);
			if(fread(buffer, 1, sizeof(buffer), fan_pwm_fd[fan_id]) > 0) 
				pwm = atoi(buffer);
			return pwm;
		}
	}
	return -1;
}
