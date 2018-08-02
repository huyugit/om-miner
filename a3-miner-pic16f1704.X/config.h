#ifndef CINFIG_H
#define CONFIG_H

#include <xc.h>
#include <stdint.h>
#include "adda.h"

#ifdef	__cplusplus
extern "C" {
#endif

/* 1: open log trace, 0: close log trace */
/* i2c log trace switch */
#define DEBUG 1
/* main log trace switch */
#define MAIN_DEBUG 1
#define ERROR_DEBUG 1

#define LOGD(log) do { \
	if (DEBUG) { \
		USARTWriteString(log); \
	} else { \
		NOP(); \
	} \
} while(0);

#define LOG(log) do { \
	if (MAIN_DEBUG) { \
		USARTWriteString(log); \
	} else { \
		NOP(); \
	} \
} while(0);

#define LOGE(log) do { \
	if (ERROR_DEBUG) { \
		USARTWriteString(log); \
	} else { \
		NOP(); \
	} \
} while(0);

//#define PIC16F877A 1
#define PIC16F1704 1
#define EEPROM_TEST_MODE 1
#define PAGE_SIZE 8
#define INT_LEN 7
#define EEPROM_DATA_SIZE 8
#define EEPROM_PAGE_W_PERIOD 10
#define RO_SIZE 2 // OPA setting, digital resistor setting
#define WORD_ADDR_RW 0

#define UINT82STR(i) uint2str(i)

#define UINT162STR(i) uint2str(i)

#define _XTAL_FREQ 16000000

#define R_ERROR -1
#define W_ERROR -1
#define OK 0

#ifdef	__cplusplus
}
#endif

#endif	/* USART_PIC16_H */

