/*************************************************************************
        > File Name: fan_board_led.h
        > Author: gzh
        > Mail: zhihua.ge@bitfily.com 
        > Created Time: 2017年12月12日 星期二 09时33分42秒
 ************************************************************************/
#ifndef __FAN_BOARD_LED_H__
#define __FAN_BOARD_LED_H__
#define LED_OFF			0x0
#define LED_ON			0x1

#define LED_GREEN		0x0
#define LED_RED			0x1

#define RED_LED_MASK		0x4
#define RED_LED_ON_MASK		0x4
#define RED_LED_OFF_MASK	0xFB
#define GREEN_LED_MASK 		0x8
#define GREEN_LED_ON_MASK	0x8
#define GREEN_LED_OFF_MASK	0xF7

#define LED_STATUS_OFF		0x0
#define LED_STATUS_SLOW		0x1
#define LED_STATUS_QUICK	0x2
#define LED_STATUS_ON		0x3

#define LED_SLOW_GAP		1000000
#define LED_QUICK_GAP		100000
#define LED_SLOW_COUNT 		3
#define LED_QUICK_COUNT		5
#define LED_FLASH_GAP		3000000
/* 风扇板上绿灯控制 */
int green_led_ctrl(int on_off);
/* 风扇板上红灯控制 */
int red_led_ctrl(int on_off);
void led_flash_thread(void);

#endif

