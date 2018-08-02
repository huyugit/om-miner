/*************************************************************************
        > File Name: common.c
        > Author: gzh
        > Mail: zhihua.ge@bitfily.com 
        > Created Time: 2017年12月12日 星期二 09时33分42秒
 ************************************************************************/
#include "common.h"

/* 记录各检测项结果 */
int g_mon_item_rst[MON_MAX_NUM] = {0};

int mon_item_rst_set(int id, int rst)
{
	if(id >= MON_MAX_NUM){
		return -1;
	}else{
		g_mon_item_rst[id] = rst;
	}

	return 0;
}

int mon_item_rst_get(int id)
{
	return g_mon_item_rst[id];
}

/* 字符串按16进制转数字 */
int htoi(char s[])
{  
    int i;  
    int n = 0;  
    if (s[0] == '0' && (s[1]=='x' || s[1]=='X')) {  
        i = 2;  
    } else {  
        i = 0;  
    }
	
    for (; (s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'z') || (s[i] >='A' && s[i] <= 'Z');++i) {  
        if (tolower(s[i]) > '9') {  
            n = 16 * n + (10 + tolower(s[i]) - 'a');  
        } else {  
            n = 16 * n + (tolower(s[i]) - '0');  
        }  
    }
	
    return n;  
}

int do_popen_set(char *cmd)
{
	int rc = 0;
	FILE *fp = NULL;

	if(NULL == cmd){
		return ERR_POPEN_CMD_NULL;
	}
	/* 先执行命令 */
	fp = popen(cmd, "r");
	if(NULL == fp){
		printf("%s", cmd);
		perror("Err: exec fail\n");
		return ERR_POPEN_EXEC;
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

int do_popen_get_hex_num(char *cmd, int *pRetValue)
{
	int rc = 0;
	char value[16] = {0};
	FILE *fp = NULL;

	/* 清除原始数据 */
	*pRetValue = 0;

	if(NULL == cmd){
		return ERR_POPEN_CMD_NULL;
	}

	if(NULL == pRetValue){
		return ERR_POPEN_RET_NULL;
	}

	/* 先执行命令 */
	fp = popen(cmd, "r");
	if(NULL == fp){
		printf("%s", cmd);
		perror("Err: exec fail\n");
		return ERR_POPEN_EXEC;
	}else{
		fgets(value, 16, fp);
		if(value[strlen(value) - 1] == '\n'){
			value[strlen(value) - 1] = '\0';
		}
		*pRetValue = htoi(value);
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

int do_popen_get_oct_num(char *cmd, int *pRetValue)
{
	int rc = 0;
	char value[16] = {0};
	FILE *fp = NULL;

	/* 清除原始数据 */
	*pRetValue = 0;

	if(NULL == cmd){
		return ERR_POPEN_CMD_NULL;
	}

	if(NULL == pRetValue){
		return ERR_POPEN_RET_NULL;
	}

	/* 先执行命令 */
	fp = popen(cmd, "r");
	if(NULL == fp){
		printf("%s", cmd);
		perror("Err: exec fail\n");
		return ERR_POPEN_EXEC;
	}else{
		fgets(value, 16, fp);
		if(value[strlen(value) - 1] == '\n'){
			value[strlen(value) - 1] = '\0';
		}
		*pRetValue = atoi(value);
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

int do_popen_get_string(char *cmd, char *buf, int buf_len)
{
	int rc = 0;
	char value[1024] = {0};
	FILE *fp = NULL;


	if(NULL == cmd){
		return ERR_POPEN_CMD_NULL;
	}

	if(NULL == buf){
		return ERR_POPEN_RET_NULL;
	}

	/* 先执行命令 */
	fp = popen(cmd, "r");
	if(NULL == fp){
		printf("%s", cmd);
		perror("Err: exec fail\n");
		return ERR_POPEN_EXEC;
	}else{
		fgets(value, 1024, fp);
		if(value[strlen(value) - 1] == '\n'){
			value[strlen(value) - 1] = '\0';
		}
		if(strlen(value) > buf_len){
			memcpy(buf, value, buf_len);
		}else{
			memcpy(buf, value, strlen(value));
		}
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

