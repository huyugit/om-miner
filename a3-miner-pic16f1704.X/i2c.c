#include <stdint.h>
#include <xc.h>
#include "i2c.h"
#include "config.h"
#include "usart_pic16.h"

extern uint8_t read_data[PAGE_SIZE + 1];

#define uchar uint8_t

#define RETRY_TIMES 100

void init_i2c()
{	
	TRISC0 = 0x1;
    TRISC1 = 0x1;  // digital input
	
	ANSELCbits.ANSC0 = 0;
	ANSELCbits.ANSC1 = 0; // digital mode
	
	RC1PPSbits.RC1PPS = 0x11;   //RC1->MSSP:SDA;
    SSPDATPPSbits.SSPDATPPS = 0x11;   //RC1->MSSP:SDA;
    RC0PPSbits.RC0PPS = 0x10;   //RC0->MSSP:SCL;
    SSPCLKPPSbits.SSPCLKPPS = 0x10;   //RC0->MSSP:SCL;
#ifdef PIC16F877A
	SSPSTAT = 0x80;
	SSPCON = 0x38;
	SSPCON2 = 0;
	SSPADD = 0x09;
#else   
	// R_nW write_noTX; P stopbit_notdetected; S startbit_notdetected; BF RCinprocess_TXcomplete; SMP Standard Speed; UA dontupdate; CKE disabled; D_nA lastbyte_address; 
    SSP1STAT = 0x80;
    // SSPEN enabled; WCOL no_collision; CKP Idle:Low, Active:High; SSPM FOSC/4_SSPxADD_I2C; SSPOV no_overflow; 
    SSP1CON1 = 0x28;
    // ACKTIM ackseq; SBCDE disabled; BOEN disabled; SCIE disabled; PCIE disabled; DHEN disabled; SDAHT 100ns; AHEN disabled; 
    SSP1CON3 = 0x00;
	SSP1CON2 = 0x00;
    // SSP1ADD 3; 
    SSP1ADD = 0x27;
#endif
}

uint8_t write(uint8_t i2c_addr, uint8_t word_addr, uint8_t size, uint8_t *buf)
{
	uint8_t i;
	SSPIF = 0;
 	SEN = 1;
	while(!SSPIF);
	SSPIF = 0;
	//SSPBUF = 0xA0;
	SSPBUF = i2c_addr & 0xfe;
	while(!SSPIF);
	SSPIF = 0;
	SSPBUF = word_addr;
	while(!SSPIF);
	SSPIF = 0;
	for(i = 0; i < size; i++)
	{
		SSPBUF = buf[i];
		while(!SSPIF);
		SSPIF = 0;
	}
	PEN = 1;
	while(!SSPIF);
	SSPIF = 0;
	return 0;
}

void AD5246_write(uint8_t i2c_addr, uint8_t data)
{
	uint8_t i;
	
	SEN = 1;
	while ((SSPSTAT & 0x04) || (SSPCON2 & 0x1F));
	
	SSPBUF = i2c_addr & 0xfe;
	while(BF) LOGD("ad5246w wait bf");
	while(ACKSTAT) LOGD("ad5246w wait ack");
		
	SSPBUF = data;
	while(BF) LOGD("ad5246w wait bf2");
	while(ACKSTAT) LOGD("ad5246w wait ack2");

	PEN = 1;
	while ((SSPSTAT & 0x04) || (SSPCON2 & 0x1F)) LOGD("ad5246w wait p");
}


uint8_t read(uint8_t i2c_addr, uint8_t word_addr, uint8_t size, uint8_t *buf)
{
	uint8_t i;

	read_data[PAGE_SIZE + 1] = "";
	SSPIF = 0;
	SEN = 1;
	while(!SSPIF);
	SSPIF = 0;
	//SSPBUF = 0xA0;
	SSPBUF = i2c_addr & 0xfe;
	while(!SSPIF);
	SSPIF = 0;
	SSPBUF = word_addr;
	while(!SSPIF);
	SSPIF = 0;
	SSPIF = 0;
	RSEN = 1;
	while(!SSPIF);
	SSPIF = 0;
	//SSPBUF = 0xA1;
	SSPBUF = i2c_addr | 0x1;
	while(!SSPIF);
	SSPIF = 0;
	for(i = 0; i < size; i++)
	{
		RCEN = 1;
		while(!SSPIF);
		buf[i] = SSPBUF;
		while(!SSPIF);
		SSPIF = 0;
		if(i >= size - 1)
		{
			ACKDT = 1;
		}
		else
		{
			ACKDT = 0;	
		}
		ACKEN = 1;
		while(!SSPIF);
		SSPIF = 0;		
	}
	PEN = 1;
	while(!SSPIF);
	SSPIF = 0;
	return 0;
}

uint8_t AD5246_read(uint8_t i2c_addr, uint8_t *buf)
{	
	read_data[PAGE_SIZE + 1] = "";
	
	SEN = 1;
	while ((SSPSTAT & 0x04) || (SSPCON2 & 0x1F)) LOGD("ad5246r wait s");
	
	SSPBUF = i2c_addr | 0x1;
	while (BF) LOGD("ad5246r wait bf");
	while(ACKSTAT) LOGD("ad5246r wait ack");

	RCEN = 1;
	while ((SSPSTAT & 0x04) || (SSPCON2 & 0x1F)) LOGD("ad5246r wait rcen");

	buf[0] = SSPBUF;
	
	ACKDT = 1; // NACK
	ACKEN = 1;

	PEN = 1;
	while ((SSPSTAT & 0x04) || (SSPCON2 & 0x1F)) LOGD("ad5246r wait p");
}

void get_resistor(void)
{
	AD5246_read(AD5246, read_data);
}

void set_resistor(uint8_t data)
{
	AD5246_write(AD5246, data);
}

uint8_t get_HSB_temp_L(uint8_t addr)
{
	if (read(addr, WORD_ADDR_RW, 2, read_data) != 0) {
		LOGE("read err\r\n");

		return R_ERROR;
	}

	return (read_data[0]);
}

uint8_t get_HSB_temp_R(uint8_t addr)
{
	if (read(addr, WORD_ADDR_RW, 2, read_data) != 0) {
		LOGE("read err\r\n");
		
		return R_ERROR;
	}

	return (read_data[0]);
}


