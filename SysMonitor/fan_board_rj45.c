/*************************************************************************
        > File Name: fan_board_rj45.c
        > Author: gzh
        > Mail: zhihua.ge@bitfily.com 
        > Created Time: 2017�?2�?1�?星期�?09�?3�?2�?
 ************************************************************************/
#include "common.h"
#include "fan_board_rj45.h"
#include "pca9536.h"

int g_fd_linkstatus = -1;
int g_fd_txbytes = -1;
int g_fd_rxbytes = -1;
int g_link_state = LINK_DOWN;
int g_tx_bytes_old = 0;
int g_tx_bytes_new = 0;
int g_rx_bytes_old = 0;
int g_rx_bytes_new = 0;
int g_data_led_blink_round = 0;

#define LED_STATUS_GET "i2cget -y 0 0x41 0x1"
#define LED_STATUS_SET "i2cset -y 0 0x41 0x1 0x%x"
#define TX_BYTES_GET "cat /sys/class/net/eth0/statistics/tx_bytes"
#define RX_BYTES_GET "cat /sys/class/net/eth0/statistics/rx_bytes"

#define OPERSTATE_PATH "/sys/class/net/eth0/operstate"
#define TX_BYTES_PATH "/sys/class/net/eth0/statistics/tx_bytes"
#define RX_BYTES_PATH "/sys/class/net/eth0/statistics/rx_bytes"

int link_led_set(int flag)
{
	int rc = 0;
	int pin_value = 0;
	char pin_set_cmd[1024] = {0};

	rc = do_popen_get_hex_num(LED_STATUS_GET, &pin_value);
	if(rc){
		perror("Err: Get fan board link led status fail\n");
		return ERR_FAN_LINK_LED;
	}
	
	if(flag == LINK_LED_OFF){
		pin_value &= 0xfe;
	}else{
		pin_value |= 0x1;
	}
	
	snprintf(pin_set_cmd, 1024, LED_STATUS_SET, pin_value);
	rc = do_popen_set(pin_set_cmd);
	if(rc){
		perror("Err: Set fan board link led status fail\n");
		return ERR_FAN_LINK_LED;
	}
	
	return 0;
}

int link_led_get(int *pPinStatus)
{
	int rc = 0;
	*pPinStatus = 0;

	rc = do_popen_get_hex_num(LED_STATUS_GET, pPinStatus);
	if(rc){
		perror("Err: Get fan board link led status fail\n");
		return ERR_FAN_LINK_LED;
	}

	return 0;
}

int data_led_set(int flag)
{
	int rc = 0;
	int pin_value = 0;
	char pin_set_cmd[1024] = {0};

	rc = do_popen_get_hex_num(LED_STATUS_GET, &pin_value);
	if(rc){
		perror("Err: Get fan board data led status fail\n");
		return ERR_FAN_LINK_LED;
	}
	
	if(flag == DATA_LED_OFF){
		pin_value &= 0xfd;
	}else{
		pin_value |= 0x2;
	}
	
	snprintf(pin_set_cmd, 1024, LED_STATUS_SET, pin_value);
	rc = do_popen_set(pin_set_cmd);
	if(rc){
		perror("Err: Set fan board data led status fail\n");
		return ERR_FAN_LINK_LED;
	}
	
	return 0;
}

/* 网络链接状态检测函�?*/
int LinkStatusCheck(void *pParam)
{
	int ret = 0;
	int pin_status = 0;
	char link_state_cur = 0;

	if(g_fd_linkstatus == -1){
		g_fd_linkstatus = open(OPERSTATE_PATH, O_RDONLY);
		if(-1 == g_fd_linkstatus){
			perror("Err: open operstate failed\n");
			return ERR_NET_OPERSTATE;
		}
	}
	lseek(g_fd_linkstatus, 0, SEEK_SET);
	read(g_fd_linkstatus, &link_state_cur, 1);
	lseek(g_fd_linkstatus, 0, SEEK_SET);
	
	ret = link_led_get(&pin_status);
	if(0 == ret){
		if('u' == link_state_cur) {
			g_link_state = LINK_UP;
			if (0 == (pin_status & LINK_LED_MASK)){
				link_led_set(LINK_LED_ON);
			}
		} else {
			g_link_state = LINK_DOWN;
			if (1 == (pin_status & LINK_LED_MASK)){
				link_led_set(LINK_LED_OFF);	
			}
		}
	}

	return ret;
}

int tx_bytes_update_check(int *pUpdateFlag)
{
	int rc = 0;
	int tx_bytes = 0;

	rc = do_popen_get_oct_num(TX_BYTES_GET, &tx_bytes);
	if(rc){
		perror("Err: Get eth0 tx bytes fail\n");
		return ERR_NET_TXBYTES;
	}
	
	if(tx_bytes > g_tx_bytes_old){
		*pUpdateFlag = 1;
		g_tx_bytes_old = tx_bytes;
	}
	
	return 0;
}

int rx_bytes_update_check(int *pUpdateFlag)
{
	int rc = 0;
	int rx_bytes = 0;

	rc = do_popen_get_oct_num(RX_BYTES_GET, &rx_bytes);
	if(rc){
		perror("Err: Get eth0 rx bytes fail\n");
		return ERR_NET_TXBYTES;
	}
	
	if(rx_bytes > g_rx_bytes_old){
		*pUpdateFlag = 1;
		g_rx_bytes_old = rx_bytes;
	}
	
	return 0;
}

/* 网络数据收发检测函�?*/
int DataStatusCheck(void *pParam)
{
	int ret = 0;
	int tx_bytes_inc = 0;
	int rx_bytes_inc = 0;
	int blink_times = 0;
	int count = 0;

	if(g_fd_txbytes == -1){
		g_fd_txbytes = open(TX_BYTES_PATH, O_RDONLY);
		if(-1 == g_fd_txbytes){
			perror("Err: open tx bytes failed\n");
			return ERR_NET_TXBYTES;
		}
	}

	if(g_fd_rxbytes == -1){
		g_fd_rxbytes = open(RX_BYTES_PATH, O_RDONLY);
		if(-1 == g_fd_rxbytes){
			perror("Err: open rx bytes failed\n");
			return ERR_NET_RXBYTES;
		}
	}

	if(g_link_state == LINK_UP){
		tx_bytes_update_check(&tx_bytes_inc);
		rx_bytes_update_check(&rx_bytes_inc);
		if(tx_bytes_inc || rx_bytes_inc){
			g_data_led_blink_round++;
			if(g_data_led_blink_round % 3 == 0) blink_times = 1;
			if(g_data_led_blink_round % 3 == 1) blink_times = 2;
			if(g_data_led_blink_round % 3 == 2) blink_times = 0;
			for(count = 0; count < blink_times; count++){
				data_led_set(DATA_LED_OFF);
				usleep(10000);
				data_led_set(DATA_LED_ON);
				usleep(10000);
			}
		}
	}else{
		data_led_set(DATA_LED_OFF);
	}

	return ret;
}

MON_ITEM mon_net_link_led = {
	"net_link_led",
	MON_NET_LINK_LED_ID,
	0,
	0,
	LinkStatusCheck,
	NULL
};

MON_ITEM mon_net_data_led = {
	"net_data_led",
	MON_NET_DATA_LED_ID,
	0,
	0,
	DataStatusCheck,
	NULL
};


