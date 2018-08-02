/******************************************************************************

 Serial communication library for PIC16F series MCUs.

 Compiler: Microchip XC8 v1.12 (http://www.microchip.com/xc)

 MCU: PIC16F877A
 Frequency: 20MHz

                                     NOTICE

NO PART OF THIS WORK CAN BE COPIED, DISTRIBUTED OR PUBLISHED WITHOUT A
WRITTEN PERMISSION FROM EXTREME ELECTRONICS INDIA. THE LIBRARY, NOR ANY PART
OF IT CAN BE USED IN COMMERCIAL APPLICATIONS. IT IS INTENDED TO BE USED FOR
HOBBY, LEARNING AND EDUCATIONAL PURPOSE ONLY. IF YOU WANT TO USE THEM IN
COMMERCIAL APPLICATION PLEASE WRITE TO THE AUTHOR.


WRITTEN BY:
AVINASH GUPTA
me@avinashgupta.com

*******************************************************************************/

#ifndef USART_PIC16_H
#define	USART_PIC16_H

#include <stdint.h>

#ifdef	__cplusplus
extern "C" {
#endif

//Constants
#define RECEIVE_BUFF_SIZE 64

//Varriables
volatile char URBuff[RECEIVE_BUFF_SIZE];	//USART Receive Buffer
volatile int8_t UQFront;
volatile int8_t UQEnd;

void USARTInit(uint16_t baud_rate);
void USARTWriteChar(char ch);
void USARTWriteString(const char *str);
void USARTWriteLine(const char *str);
void USARTWriteInt(int16_t val, int8_t field_length);
void USARTHandleRxInt();
char USARTReadData();
uint8_t USARTDataAvailable();
void USARTGotoNewLine();
void USARTReadBuffer(char *buff,uint16_t len);
void USARTFlushBuffer();



#ifdef	__cplusplus
}
#endif

#endif	/* USART_PIC16_H */

