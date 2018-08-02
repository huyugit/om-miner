/*************************************************************************
        > File Name: fan_board_key.c
        > Author: gzh
        > Mail: zhihua.ge@bitfily.com 
        > Created Time: 2017年12月12日 星期二 09时33分42秒
 ************************************************************************/
#include "common.h"
#include "fan_board_key.h"
#include "pca9536.h"

int g_ipkey_status_old = 0;	/* 上一次定时器触发，IP Key的状态 */
int g_rckey_status_old = 0;	/* 上一次定时器触发，复位键的状态 */
int g_ipkey_hicount = 0;	/* 连续几次定时器触发检测IP Key都是1的次数 */
int g_rckey_hicount = 0;	/* 连续几次定时器触发检测复位键都是1的次数 */
int g_gpio_init_flag = 0;	/* gpio的sysfs是否初始化 */
#define DO_IPREPORT_CMD "java -jar /opt/IPReport/IP-reportor.jar"
#define DO_RECOVER_CMD "/Recover/recover.sh"
#define IPR_KEY_UNEXP_CMD	"echo 0 > /sys/class/gpio/unexport"
#define RCV_KEY_UNEXP_CMD	"echo 2 > /sys/class/gpio/unexport"
#define IPR_KEY_EXP_CMD	"echo 0 > /sys/class/gpio/export"
#define RCV_KEY_EXP_CMD	"echo 2 > /sys/class/gpio/export"
#define IPR_KEY_IN_CMD "echo \"in\" > /sys/class/gpio/gpio0/direction"
#define RCV_KEY_IN_CMD "echo \"in\" > /sys/class/gpio/gpio2/direction"
#define IPR_KEY_VALUE_GET_CMD "cat /sys/class/gpio/gpio0/value"
#define RCV_KEY_VALUE_GET_CMD "cat /sys/class/gpio/gpio2/value"

/* 按键1按1秒之后发送自身IP地址的UDP广播报文 */
int do_ipreport(void)
{
	return do_popen_set(DO_IPREPORT_CMD);
}

/* 长按5秒后进行复位 */
int do_recover(void)
{
	return do_popen_set(DO_RECOVER_CMD);
}

/* 
初始化风扇板上两个按钮对应GPIO在/sys/class/gpio中的文件
*/
int fan_board_sw_keys_gpio_file_init(void)
{
	int ret = 0;

	ret = do_popen_set(IPR_KEY_UNEXP_CMD);
	if(ret)	return ret;
	ret = do_popen_set(RCV_KEY_UNEXP_CMD);
	if(ret)	return ret;
	ret = do_popen_set(IPR_KEY_EXP_CMD);
	if(ret)	return ret;
	ret = do_popen_set(RCV_KEY_EXP_CMD);
	if(ret)	return ret;
	ret = do_popen_set(IPR_KEY_IN_CMD);
	if(ret)	return ret;
	ret = do_popen_set(RCV_KEY_IN_CMD);
	if(ret)	return ret;
	
	return 0;
}

/* 
获取IP Report按键的值
*/
int ip_report_key_value_get(int *pValue)
{
	return do_popen_get_oct_num(IPR_KEY_VALUE_GET_CMD, pValue);
}

/* 
获取Recover按键的值
*/
int recover_key_value_get(int *pValue)
{
	return do_popen_get_oct_num(RCV_KEY_VALUE_GET_CMD, pValue);
}

/* 按键检测函数 */
int FanBoardKeyCheck(void *pParam)
{
	int ret = 0;
	int ip_key_value = 0;
	int rc_key_value = 0;

	if(!g_gpio_init_flag){
		ret = fan_board_sw_keys_gpio_file_init();
		if(ret){
			return ret;
		}

		g_gpio_init_flag = 1;
	}

	/* 判断是否长按了IP上报按键 */
	ret = ip_report_key_value_get(&ip_key_value);
	if(ret)	return ret;
	if(ip_key_value == 0){
		g_ipkey_hicount = 0;
	}else{
		g_ipkey_hicount += 1;
		if(g_ipkey_hicount == 2){
			do_ipreport();
		}
	}

	/* 判断是否长按了恢复出厂设置按键 */
	recover_key_value_get(&rc_key_value);
	if(ret) return ret;
	if(rc_key_value == 0){
		g_rckey_hicount = 0;
	}else{
		g_rckey_hicount += 1;
		if(g_rckey_hicount == 20){
			do_recover();
		}
	}

	return ret;
}

MON_ITEM mon_fan_board_key = {
	"fan_board_key_check",
	MON_FAN_BRD_KEY_ID,
	0,
	0,
	FanBoardKeyCheck,
	NULL
};

