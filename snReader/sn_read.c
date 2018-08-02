/*************************************************************************
	> File Name: spi-flash.c
	> Author: gzh
	> Mail: zhihua.ge@163.com 
	> Created Time: 2018年01月29日 星期一 23时24分09秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#define SN_LENGTH		32

int main(int argc, char *argv[])
{
	int mtd_fd = -1;
	char rd_buf[SN_LENGTH] = {0};
	
	mtd_fd = open("/dev/mtd0", O_RDWR|O_SYNC);
	if(mtd_fd < 0){
		printf("spi-flash device open failed\n");
		return -1;
	}

	read(mtd_fd, rd_buf, SN_LENGTH);
	printf("%s\n");

	close(mtd_fd);

	return 0;
}
