/******************************************************************************

 Serial communication library for PIC16F series MCUs.

 Compiler: Microchip XC8 v1.12 (http://www.microchip.com/xc)

 Version: 1.0 (21 July 2013)

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
#include <stdint.h>
#include <xc.h>

#include "usart_pic16.h"

void USARTInit(uint16_t baud_rate)
{
    //Setup queue
    UQFront = UQEnd = -1;

	//BR=Fosc/[16(SPBRG+1)]
    //SPBRG
    switch(baud_rate)
    {
     case 38400: 
        SPBRG = 25;
        break;
     case 19200:
        SPBRG = 64; // TODO
        break;
     case 28800:
        SPBRG = 42; // TODO
        break;
     case 33600:
        SPBRG = 36; // TODO
        break;
    }
    //TXSTA
    TXSTAbits.TX9 = 0;  //8 bit transmission
    TXSTAbits.TXEN = 1; //Transmit enable
    TXSTAbits.SYNC = 0; //Async mode
    TXSTAbits.BRGH = 1; //High speed baud rate

    //RCSTA
    RCSTAbits.SPEN = 1;   //Serial port enabled
    RCSTAbits.RX9 = 0;    //8 bit mode
    RCSTAbits.CREN = 1;   //Enable receive
    RCSTAbits.ADDEN = 0;  //Disable address detection
	
	RXPPSbits.RXPPS = 0x15;   //RC5->EUSART:RX;
    RC4PPSbits.RC4PPS = 0x14;   //RC4->EUSART:TX;

    //Receive interrupt
    GIE = 1;
    RCIE = 1;
    PEIE = 1;

    ei();
}

void USARTWriteChar(char ch)
{
  while(!PIR1bits.TXIF);

  TXREG = ch;
}

void USARTWriteString(const char *str)
{
  while(*str != '\0')
  {
      USARTWriteChar(*str);
      str++;
  }
}

void USARTWriteLine(const char *str)
{
    USARTWriteChar('\r');//CR
    USARTWriteChar('\n');//LF

    USARTWriteString(str);
}

void USARTHandleRxInt()
{
  /*if(RB1==1)
    RB1=0;
  else
    RB1=1;*/
  
    //Read the data
    char data=RCREG;

    //Now add it to q
    if(((UQEnd == RECEIVE_BUFF_SIZE-1) && UQFront == 0) || ((UQEnd+1) == UQFront))
    {
        //Q Full
	UQFront++;
	if(UQFront == RECEIVE_BUFF_SIZE) UQFront = 0;
    }

    if(UQEnd == RECEIVE_BUFF_SIZE-1)
        UQEnd = 0;
    else
	UQEnd++;

    URBuff[UQEnd] = data;

    if(UQFront == -1) UQFront = 0;
    
}

char USARTReadData()
{
    char data;

    //Check if q is empty
    if(UQFront == -1)
	return 0;

    data = URBuff[UQFront];

    if(UQFront == UQEnd)
    {
        //If single data is left
	//So empty q
	UQFront = UQEnd = -1;
    }
    else
    {
	UQFront++;

	if(UQFront == RECEIVE_BUFF_SIZE)
            UQFront = 0;
    }

    return data;
}

uint8_t USARTDataAvailable()
{
    if(UQFront == -1) return 0;
    if(UQFront < UQEnd)
	return(UQEnd - UQFront + 1);
    else if(UQFront > UQEnd)
	return (RECEIVE_BUFF_SIZE - UQFront + UQEnd + 1);
    else
	return 1;
}

void USARTWriteInt(int16_t val, int8_t field_length)
{
    char str[5] = {0,0,0,0,0};
    int8_t i = 4,j = 0;

    //Handle negative integers
    if(val < 0)
    {
        USARTWriteChar('-');   //Write Negative sign
        val=val*-1;     //convert to positive
    }
    else
    {
        USARTWriteChar(' ');
    }

    if(val == 0 && field_length < 1)
    {
        USARTWriteChar('0');
        return;
    }
    while(val)
    {
        str[i] = val%10;
        val = val/10;
        i--;
    }

    if(field_length == -1)
        while(str[j] == 0) j++;
    else
        j = 5-field_length;


    for(i = j; i < 5; i++)
    {
        USARTWriteChar('0'+str[i]);
    }
}

void USARTGotoNewLine()
{
    USARTWriteChar('\r');//CR
    USARTWriteChar('\n');//LF
}

void USARTReadBuffer(char *buff, uint16_t len)
{
	uint16_t i;
	for(i = 0; i < len; i++)
	{
		buff[i] = USARTReadData();
	}
}
void USARTFlushBuffer()
{
	while(USARTDataAvailable()>0)
	{
		USARTReadData();
	}
}