/*************************************************************************
        > File Name: fan_temp.c
        > Author: gzh
        > Mail: zhihua.ge@bitfily.com 
        > Created Time: 2017年12月12日 星期二 09时33分42秒
 ************************************************************************/
#include "fan_temp.h"
#include "fan_board_led.h"
#include "common.h"

#define TEMP1_GET "cat /sys/class/hwmon/hwmon1/device/temp1_input"
#define TEMP2_GET "cat /sys/class/hwmon/hwmon2/device/temp1_input"

/* 温度检测函数 */
int TmpCheck(void *pParam)
{
	int ret = 0;
	int temp1 = 0;
	int temp2 = 0;
	
	ret = do_popen_get_oct_num(TEMP1_GET, &temp1);
	if(ret){
		perror("Err: Get fan board temperature 1 fail\n");
		return ERR_FAN_TEMP1_RD;
	}
	temp1 = temp1/1000;
	ret = do_popen_get_oct_num(TEMP2_GET, &temp2);
	if(ret){
		perror("Err: Get fan board temperature 2 fail\n");
		return ERR_FAN_TEMP2_RD;
	}
	temp2 = temp2/1000;

	//printf("temp1 = %d, temp2 = %d\n", temp1, temp2);
	if(temp1 > TEMP1_ALARM_TH){
		return ERR_FAN_TEMP2_OVER;
	}

	if(temp2 > TEMP2_ALARM_TH){
		return ERR_FAN_TEMP2_OVER;
	}
	
	return 0;
}

/* 温度问题报警函数 */
int TmpAlarm(int err_type)
{
	switch(err_type){
		case ERR_FAN_TEMP1_RD:
		case ERR_FAN_TEMP2_RD:
		case ERR_FAN_TEMP1_OVER:
		case ERR_FAN_TEMP2_OVER:
			mon_item_rst_set(MON_TMP_ID, LED_STATUS_QUICK);
			break;

		default:
			/* Everything goes well, take easy:) */
			mon_item_rst_set(MON_TMP_ID, LED_STATUS_OFF);
			break;
	}

	return 0;
}

MON_ITEM mon_fan_bd_temp = {
	"temprature",
	MON_TMP_ID,
	0,
	0,
	TmpCheck,
	TmpAlarm
};

