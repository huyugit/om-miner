/*************************************************************************
        > File Name: pca9536.c
        > Author: gzh
        > Mail: zhihua.ge@bitfily.com 
        > Created Time: 2017年12月12日 星期二 09时33分42秒
 ************************************************************************/
#include "common.h"
#include "pca9536.h"

char *pca9536_get_cmd = "i2cget -y 0 0x41 0x1";
char *pca9536_set_cmd = "i2cset -y 0 0x41 0x1 0x%x";

int pca9536_pin_status_get(int *pPinStatus)
{
	int rc = 0;
	char pin_status[5] = {0};
	FILE *fp = NULL;

	/* 清除原始数据 */
	*pPinStatus = 0;

	/* 先读取PCA9536的4个输出管脚状态 */
	fp = popen(pca9536_get_cmd, "r");
	if(NULL == fp){
		perror("Err: Get pca9536 pins output value failed\n");
		return ERR_PCA9536_REG1_GET;
	}else{
		fgets(pin_status, 5, fp);
		if(pin_status[strlen(pin_status) - 1] == '\n'){
			pin_status[strlen(pin_status) - 1] = '\0';
		}
		*pPinStatus = htoi(pin_status);
	}

	//等待命令执行完毕
	rc = pclose(fp);
	if(-1 == rc){
		perror("Err: fp close failed\n");
		return ERR_POPEN_CLOSE;
	}else{
		fp = NULL;
	}

	return 0;
}

int pca9536_pin_status_set(int pinstatus)
{
	int rc = 0;
	char set_cmd[128] = {0};
	
	FILE *fp = NULL;

	snprintf(set_cmd, 1024, pca9536_set_cmd, pinstatus);
	/* 先读取PCA9536的4个输出管脚状态 */
	fp = popen(set_cmd, "r");
	if(NULL == fp){
		perror("Err: Set pca9536 pins output value failed\n");
		return ERR_PCA9536_REG1_SET;
	}

	//等待命令执行完毕
	rc = pclose(fp);
	if(-1 == rc){
		perror("Err: fp close failed\n");
		return ERR_POPEN_CLOSE;
	}else{
		fp = NULL;
	}

	return 0;
}
