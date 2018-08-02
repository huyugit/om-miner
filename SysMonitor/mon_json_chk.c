/*************************************************************************
        > File Name: mon_json_chk.c
        > Author: gzh
        > Mail: zhihua.ge@bitfily.com 
        > Created Time: 2017年12月25日 星期一 09时33分42秒
 ************************************************************************/
#include "common.h"
#include "fan_board_led.h"
#include "mon_json_chk.h"

#define MON_JSON_CHK_CMD "cat /tmp/json/sysmonitor.json | grep syserrlevel"

/* a1-master检测结果函数 */
int MonJsonCheck(void *pParam)
{
	int ret = 0;
	char a1_mon_rst[128] = {0};
	
	ret = do_popen_get_string(MON_JSON_CHK_CMD, a1_mon_rst, 128);
	if(ret){
		perror("Err: Get a1-mon.json fail\n");
		return ERR_GREP_A1_MON;
	}
	
	if(strstr(a1_mon_rst, "error1") || 
	   strstr(a1_mon_rst, "error2") || 
	   strstr(a1_mon_rst, "error3")){
		return ERR_A1_MON_ERROR;
	}
	
	return 0;
}

/* a1-master问题报警函数 */
int MonJsonAlarm(int err_type)
{
	switch(err_type){
		case ERR_GREP_A1_MON:
		case ERR_A1_MON_ERROR:
			mon_item_rst_set(MON_AI_MASTER_ID, LED_STATUS_QUICK);
			break;

		default:
			/* Everything goes well, take easy:) */
			mon_item_rst_set(MON_AI_MASTER_ID, LED_STATUS_OFF);
			break;
	}

	return 0;
}

MON_ITEM mon_a1_master = {
	"a1-master",
	MON_AI_MASTER_ID,
	0,
	0,
	MonJsonCheck,
	MonJsonAlarm
};


