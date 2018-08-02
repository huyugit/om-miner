/*************************************************************************
        > File Name: fan_board_led.c
        > Author: gzh
        > Mail: zhihua.ge@bitfily.com 
        > Created Time: 2017�?2�?2�?星期�?09�?3�?2�?
 ************************************************************************/
#include "common.h"
#include "fan_board_led.h"
#include "fan_board_rj45.h"
#include "pca9536.h"

int g_red_status = LED_STATUS_OFF;
int g_green_status = LED_STATUS_ON;

/* 风扇板上绿灯控制 */
int green_led_ctrl(int on_off)
{
	int ret = 0;
	int pca9635_status = 0;
	int pca9635_status_new = 0;

	ret = pca9536_pin_status_get(&pca9635_status);
	if(ret){
		return ret;
	}

	if(LED_OFF == on_off){
		pca9635_status_new = pca9635_status & GREEN_LED_OFF_MASK;
	}else{
		pca9635_status_new = pca9635_status | GREEN_LED_ON_MASK;
	}

	ret = pca9536_pin_status_set(pca9635_status_new);
	
	return ret;
}

/* 风扇板上红灯控制 */
int red_led_ctrl(int on_off)
{
	int ret = 0;
	int pca9635_status = 0;
	int pca9635_status_new = 0;

	ret = pca9536_pin_status_get(&pca9635_status);
	if(ret){
		return ret;
	}

	if(LED_OFF == on_off){
		pca9635_status_new = pca9635_status & RED_LED_OFF_MASK;
	}else{
		pca9635_status_new = pca9635_status | RED_LED_ON_MASK;
	}

	ret = pca9536_pin_status_set(pca9635_status_new);
	
	return ret;

}

int led_ctrl(int led_id, int on_off)
{
	int ret = 0;
	int pca9635_status = 0;
	int pca9635_status_new = 0;
	int mask = 0;

	ret = pca9536_pin_status_get(&pca9635_status);
	if(ret){
		return ret;
	}

	if(led_id == LED_GREEN)	mask = GREEN_LED_MASK;
	if(led_id == LED_RED)	mask = RED_LED_MASK;

	if(LED_OFF == on_off){
		pca9635_status_new = pca9635_status & (~mask);
	}else{
		pca9635_status_new = pca9635_status | mask;
	}

	ret = pca9536_pin_status_set(pca9635_status_new);
	
	return ret;
}

void red_led_blink(int red_led_status)
{
	int count = 0;
	int loop_count = 0;
	switch(red_led_status){
		case LED_STATUS_SLOW:
			loop_count = 3;
			for(count = 0; count < loop_count; count++){
				led_ctrl(LED_RED, LED_OFF);
				usleep(250000);
				led_ctrl(LED_RED, LED_ON);	
				usleep(250000);
			}
			break;
		case LED_STATUS_QUICK:
			loop_count = 10;
			for(count = 0; count < loop_count; count++){
				led_ctrl(LED_RED, LED_OFF);
				usleep(100000);
				led_ctrl(LED_RED, LED_ON);	
				usleep(100000);
			}

			break;
		case LED_STATUS_ON:
			led_ctrl(LED_RED, LED_ON);
			break;
		case LED_STATUS_OFF:
			led_ctrl(LED_RED, LED_OFF);
			break;
	}
}

void green_led_blink(int green_led_status)
{
	int count = 0;
	int loop_count = 0;
	switch(green_led_status){
		case LED_STATUS_SLOW:
			loop_count = 3;
			for(count = 0; count < loop_count; count++){
				led_ctrl(LED_GREEN, LED_OFF);
				usleep(250000);
				led_ctrl(LED_GREEN, LED_ON);	
				usleep(250000);
			}
			break;
		case LED_STATUS_QUICK:
			loop_count = 10;
			for(count = 0; count < loop_count; count++){
				led_ctrl(LED_GREEN, LED_OFF);
				usleep(100000);
				led_ctrl(LED_GREEN, LED_ON);	
				usleep(100000);
			}

			break;
		case LED_STATUS_ON:
			led_ctrl(LED_GREEN, LED_ON);
			break;
		case LED_STATUS_OFF:
			led_ctrl(LED_GREEN, LED_OFF);
			break;
	}

}

void led_flash_thread(void)
{
	int mon_id = 0;
	int err_level = 0;
	led_ctrl(LED_GREEN, LED_ON);
	led_ctrl(LED_RED, LED_OFF);

	while(1){
		err_level = 0;
		for(mon_id = 0; mon_id <= MON_AI_MASTER_ID; mon_id++){
			if(mon_item_rst_get(mon_id) > err_level){
				err_level = mon_item_rst_get(mon_id);
				printf("mon_id = %d, err_level = %d\n", mon_id, err_level);
			}
		}
		
		if(err_level == 0){
			led_ctrl(LED_GREEN, LED_ON);
			led_ctrl(LED_RED, LED_OFF);
		}else if(err_level == 3){
			printf("err_level 3\n");
			led_ctrl(LED_GREEN, LED_OFF);
			led_ctrl(LED_RED, LED_ON);
		}else{
			led_ctrl(LED_GREEN, LED_ON);
			if(err_level == 1){
				printf("err_level 1\n");
				red_led_blink(LED_STATUS_SLOW);
			}else{
				printf("err_level 2\n");
				red_led_blink(LED_STATUS_QUICK);
			}
		}

		//LinkStatusCheck(NULL);
		//DataStatusCheck(NULL);
		
		usleep(100000);
	}
}
