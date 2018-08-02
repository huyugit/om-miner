/*************************************************************************
        > File Name: watchdog.c
        > Author: gzh
        > Mail: zhihua.ge@bitfily.com 
        > Created Time: 2018年01月09日 星期二 09时33分42秒
 ************************************************************************/
#include "watchdog.h"
#include "common.h"

int g_wd_fd = -1;
/* 喂狗函数 */
int FeedDog(void *pParam)
{
#if 0
	long arg = 0;

	if(g_wd_fd < 0){
		g_wd_fd = open("/dev/watchdog", O_RDWR);
		if(g_wd_fd < 0){
			printf("Err: Watchdog open fail\n");
			return ERR_WATCHDOG_OPEN;
		}
	}

	if(ioctl(g_wd_fd, WDIOC_KEEPALIVE, &arg) < 0){
		printf("Watchdog Warning: fit dog failed\n");
	}
#endif
	return 0;
}

/* 喂狗问题报警函数 */
int FeedDogAlarm(int err_type)
{
	return 0;
}

MON_ITEM mon_feed_dog = {
	"watchdog",
	MON_FEED_DOG_ID,
	0,
	0,
	FeedDog,
	FeedDogAlarm
};

