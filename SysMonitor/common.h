/*************************************************************************
        > File Name: common.h
        > Author: gzh
        > Mail: zhihua.ge@bitfily.com 
        > Created Time: 2017年12月12日 星期二 09时33分42秒
 ************************************************************************/
#ifndef __COMMON_H__
#define __COMMON_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <linux/ioctl.h>

/* 定时器相关检测项 */
#define TIMER_INTVAL_MIN_S		1		/* 定时器最小间隙，sec 1s */
#define TIMER_INTVAL_MIN_US		500000	/* 定时器最小间隙，usec 0.5s */

/* 监测项 */
#define MON_TMP_ID			0x0		/* 温度 */
#define MON_NET_LINK_LED_ID	0x1		/* 网络连接LED */
#define MON_NET_DATA_LED_ID	0x2		/* 网络数据LED */
#define MON_FAN_BRD_KEY_ID	0x3		/* 风扇板按键 */
#define MON_AI_MASTER_ID	0x4		/* A1 master告警信息检测 */
#define MON_FEED_DOG_ID		0x5		/* 看门狗喂狗 */
#define	MON_MAX_NUM			0x6

/* 通用错误值定义 */
#define ERR_POPEN_CLOSE			0x01000001
#define ERR_POPEN_EXEC			0x01000002
#define ERR_POPEN_CMD_NULL		0x01000003
#define ERR_POPEN_RET_NULL		0x01000004
#define ERR_FAN_LINK_LED		0x01000005
#define ERR_FAN_DATA_LED		0x01000006
#define ERR_NET_OPERSTATE		0x01000007
#define ERR_NET_TXBYTES			0x01000008
#define ERR_NET_RXBYTES			0x01000009
#define ERR_GREP_A1_MON			0x0100000a
#define ERR_A1_MON_ERROR		0x0100000b
#define ERR_FAN_TEMP1_RD		0x0100000c
#define ERR_FAN_TEMP2_RD		0x0100000d
#define ERR_FAN_TEMP1_OVER		0x0100000e
#define ERR_FAN_TEMP2_OVER		0x0100000f
#define ERR_WATCHDOG_OPEN		0x01000010
typedef int (*check_fun)(void *pParam);		//检测项回掉函数
typedef int (*alarm_fun)(int err_type);		//问题处理回掉函数
typedef struct {
	char *monitor_name;			/* 监测项名字 */
	int monitor_id;				/* 监测项ID */
	int timer_times;			/* 定时器倍数 */
	int timer_count;			/* 定时器剩余次数 */
	check_fun pMonitorCheck;	/* 监测项回掉函数 */
	alarm_fun pAlarmFun;		/* 监测项告警函数 */
}MON_ITEM;

int htoi(char s[]);
int do_popen_set(char *cmd);
int do_popen_get_hex_num(char *cmd, int *pRetValue);
int do_popen_get_oct_num(char *cmd, int *pRetValue);
int do_popen_get_string(char *cmd, char *buf, int buf_len);
int mon_item_rst_set(int id, int rst);
int mon_item_rst_get(int id);
#endif