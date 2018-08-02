/*************************************************************************
        > File Name: watchdog.h
        > Author: gzh
        > Mail: zhihua.ge@bitfily.com 
        > Created Time: 2018年01月09日 星期二 09时33分42秒
 ************************************************************************/
#ifndef __WATCHDOG_H__
#define __WATCHDOG_H__
#define	WATCHDOG_IOCTL_BASE	'W'

struct watchdog_info {
	unsigned int options;		/* Options the card/driver supports */
	unsigned int firmware_version;	/* Firmware version of the card */
	unsigned char identity[32];	/* Identity of the board */
};

#define	WDIOC_GETSUPPORT	_IOR(WATCHDOG_IOCTL_BASE, 0, struct watchdog_info)
#define	WDIOC_GETSTATUS		_IOR(WATCHDOG_IOCTL_BASE, 1, int)
#define	WDIOC_GETBOOTSTATUS	_IOR(WATCHDOG_IOCTL_BASE, 2, int)
#define	WDIOC_GETTEMP		_IOR(WATCHDOG_IOCTL_BASE, 3, int)
#define	WDIOC_SETOPTIONS	_IOR(WATCHDOG_IOCTL_BASE, 4, int)
#define	WDIOC_KEEPALIVE		_IOR(WATCHDOG_IOCTL_BASE, 5, int)
#define	WDIOC_SETTIMEOUT        _IOWR(WATCHDOG_IOCTL_BASE, 6, int)
#define	WDIOC_GETTIMEOUT        _IOR(WATCHDOG_IOCTL_BASE, 7, int)
#define	WDIOC_SETPRETIMEOUT	_IOWR(WATCHDOG_IOCTL_BASE, 8, int)
#define	WDIOC_GETPRETIMEOUT	_IOR(WATCHDOG_IOCTL_BASE, 9, int)
#define	WDIOC_GETTIMELEFT	_IOR(WATCHDOG_IOCTL_BASE, 10, int)
#endif