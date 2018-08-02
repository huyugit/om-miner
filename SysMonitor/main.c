/*************************************************************************
        > File Name: main.c
        > Author: gzh
        > Mail: zhihua.ge@bitfily.com 
        > Created Time: 2017年12月12日 星期二 09时33分42秒
 ************************************************************************/
#include "fan_temp.h"
#include "fan_board_led.h"
#include "common.h"

extern MON_ITEM mon_fan_bd_temp;
extern MON_ITEM mon_net_link_led;
extern MON_ITEM mon_net_data_led;
extern MON_ITEM mon_fan_board_key;
extern MON_ITEM mon_a1_master;
extern MON_ITEM mon_feed_dog;

MON_ITEM *g_mon_array[MON_MAX_NUM] = {
	&mon_fan_bd_temp,
	&mon_net_link_led,
	&mon_net_data_led,
	&mon_fan_board_key,
	&mon_a1_master,
	&mon_feed_dog
};

struct timeval g_mon_interval[MON_MAX_NUM] = {
	{TIMER_INTVAL_MIN_S,	0},	//fan board temperature
	{0,	TIMER_INTVAL_MIN_US},	//net_link_led
	{0,	TIMER_INTVAL_MIN_US},	//net_data_led
	{0, TIMER_INTVAL_MIN_US},	//fan board keys
	{TIMER_INTVAL_MIN_S,	0},	//a1-master
	{0,	TIMER_INTVAL_MIN_US*2}	//watchdog
};

struct itimerval g_timer = {{0}};
struct sigaction g_timer_sa;

void timer_process(int signum)
{
	int ret = 0;
	int mon_index = 0;
	for(mon_index = 0; mon_index < MON_MAX_NUM; mon_index++){
		g_mon_array[mon_index]->timer_count--;
		if(g_mon_array[mon_index]->timer_count == 0){
			/* 恢复定时器计数器初始值 */
			g_mon_array[mon_index]->timer_count = g_mon_array[mon_index]->timer_times;
			/* 调用监测项的回掉函数 */
			if(g_mon_array[mon_index]->pMonitorCheck){
				ret = g_mon_array[mon_index]->pMonitorCheck(NULL);
				if(g_mon_array[mon_index]->pAlarmFun){
					g_mon_array[mon_index]->pAlarmFun(ret);
				}
			}
		}
	}
}

void timer_init(void)
{
	int mon_index = 0;

	/* 装载定时器处理函数 */
	g_timer_sa.sa_handler = &timer_process;
	sigaction(SIGALRM, &g_timer_sa, NULL);
	/* 初始化定时器超时时间 */
	g_timer.it_interval.tv_sec = 0;
	g_timer.it_interval.tv_usec = TIMER_INTVAL_MIN_US;
	g_timer.it_value.tv_sec = TIMER_INTVAL_MIN_S;
	g_timer.it_value.tv_usec = 0;
	setitimer(ITIMER_REAL, &g_timer, NULL);
	
	/* 初始化每个监测项结构体中的timer_count为timer_times */
	for(mon_index = 0; mon_index < MON_MAX_NUM; mon_index++){
		g_mon_array[mon_index]->timer_times = (g_mon_interval[mon_index].tv_sec*1000000+g_mon_interval[mon_index].tv_usec)/TIMER_INTVAL_MIN_US;
		g_mon_array[mon_index]->timer_count = g_mon_array[mon_index]->timer_times;
	}
}

/*
系统启动一个定时器，触发时间间隔为common.h中配置的TIMER_INTVAL_MIN_1S
每次定时器被触发的时候，会对g_mon_array数组中每个监测项中的
*/
int main(void)
{
	int ret = 0;
	pthread_t led_th;
	printf("\n\n******************Bitfily Sysmonitor****************\n\n");
	
	/* 初始化定时器 */
	timer_init();

	/* 启动LED闪烁线程 */
	ret = pthread_create(&led_th, NULL, (void *)led_flash_thread, NULL);
	if(ret != 0){
		perror("Err: led_flash_thread create failed\n");
		exit(1);
	}

	while(1){
		sleep(1000);
	}
	
	printf("Never Ever been here\n");
	return 0;
}
