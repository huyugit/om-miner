#include <pic.h>
#include <xc.h>
#include <stdio.h>
#include <stdint.h>
#include "usart_pic16.h"
#include "config.h"
#include "pin_manager.h"
#include "adda.h"
#include "i2c.h"

// CONFIG
#ifdef PIC16F877A
#pragma config FOSC = XT        // Oscillator Selection bits (XT oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable bit (BOR disabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = 1FOURTH    // Flash Program Memory Write Enable bits (0000h to 07FFh write-protected; 0800h to 1FFFh may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)
#else 
/* use for PIC16F1704 chip */
// Configuration bits: selected in the GUI

// CONFIG1
#pragma config FOSC = INTOSC    // Oscillator Selection Bits->INTOSC oscillator: I/O function on CLKIN pin
#pragma config WDTE = OFF    // Watchdog Timer Enable->WDT disabled
#pragma config PWRTE = ON    // Power-up Timer Enable->PWRT disabled
#pragma config MCLRE = ON    // MCLR Pin Function Select->MCLR/VPP pin function is MCLR
#pragma config CP = OFF    // Flash Program Memory Code Protection->Program memory code protection is disabled
#pragma config BOREN = OFF    // Brown-out Reset Enable->Brown-out Reset enabled
#pragma config CLKOUTEN = OFF    // Clock Out Enable->CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin
#pragma config IESO = ON    // Internal/External Switchover Mode->Internal/External Switchover Mode is enabled
#pragma config FCMEN = ON    // Fail-Safe Clock Monitor Enable->Fail-Safe Clock Monitor is enabled

// CONFIG2
#pragma config WRT = OFF    // Flash Memory Self-Write Protection->Write protection off
#pragma config PPS1WAY = ON    // Peripheral Pin Select one-way control->The PPSLOCK bit cannot be cleared once it is set by software
#pragma config ZCDDIS = ON    // Zero-cross detect disable->Zero-cross detect circuit is disabled at POR
#pragma config PLLEN = OFF    // Phase Lock Loop enable->4x PLL is enabled when software sets the SPLLEN bit
#pragma config STVREN = ON    // Stack Overflow/Underflow Reset Enable->Stack Overflow or Underflow will cause a Reset
#pragma config BORV = LO    // Brown-out Reset Voltage Selection->Brown-out Reset Voltage (Vbor), low trip point selected.
#pragma config LPBOR = OFF    // Low-Power Brown Out Reset->Low-Power BOR is disabled
#pragma config LVP = ON    // Low-Voltage Programming Enable->Low-voltage programming enabled
#endif

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#define DAC_OUTPUT (0x4d)
#define DIGITAL_R (0x40)
#define LOOP_CYCLE (2000) /* 2000ms */

struct eeprom_stu {
	adc_result_t vfault;
	adc_result_t ifault;	
	uint8_t tempL;
	uint8_t tempR;
	uint8_t dac_output;
	uint8_t resistor;
};
/* init AT24LC01 data map */
struct eeprom_stu eeprom_data_map = {
	0xFFFF, \
	0xFFFF, \
	0xFF, \
	0xFF, \
	0x4D, \
	0x40 \
};

uint8_t ee_data[PAGE_SIZE];
const uint8_t ee_test_data[PAGE_SIZE] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};
uint8_t read_data[PAGE_SIZE + 1] = {0};
uint8_t int2str_tmp[INT_LEN] = {0};

void GPIO_init(void);
void USARTInit(uint16_t baud_rate);
void OSCILLATOR_Initialize(void);
void WDT_Initialize(void);
uint8_t uint2str(uint16_t i);
void printEEPROMData(uint8_t size, uint8_t *buf);
bool EEPROM_WR_check(uint8_t size, uint8_t *buf);

void main(void)
{	
	bool pass = true;
	uint8_t i = 0;
	uint8_t *peeprom = (uint8_t *)(&eeprom_data_map);
	
	OSCILLATOR_Initialize();
	WDT_Initialize();
	GPIO_init();
	USARTInit(38400);
	init_i2c();
	ADC_Initialize();
	DAC_Initialize();
	OPA1_Initialize();

	LOG("enter\r\n");

	while(true) {
		//Get the amount of data waiting in USART queue
		uint8_t n = USARTDataAvailable();
		
#ifdef EEPROM_TEST_MODE
		pass = true;

		write(I2C_ADDR, WORD_ADDR_RW, PAGE_SIZE, ee_test_data);
		__delay_ms(EEPROM_PAGE_W_PERIOD);
		read(I2C_ADDR, WORD_ADDR_RW, PAGE_SIZE, read_data);
		if (EEPROM_WR_check(PAGE_SIZE, ee_test_data) == true) {
			LOG("C OK\r\n");
		} else {
			printEEPROMData(PAGE_SIZE, read_data);
			LOG("C NOK\r\n");
		}
#endif
		/* capture HSB sensor data */
		eeprom_data_map.vfault = get_adc_value(VFault_ADC);
		LOG("vfault:");
		uint2str(eeprom_data_map.vfault & 0x3ff);
		LOG(int2str_tmp);
		USARTGotoNewLine();
		
		eeprom_data_map.ifault = get_adc_value(IFault_ADC);
		LOG("ifault:");
		uint2str(eeprom_data_map.ifault & 0x3ff);
		LOG(int2str_tmp);
		USARTGotoNewLine();

		if (get_HSB_temp_L(LM75L) != R_ERROR) {
			eeprom_data_map.tempL = read_data[0]; // ignore fractional part, eg: 27.125 deg, display 27 deg
		} else {
			/* TODO */
			eeprom_data_map.tempL = 0xff;
		}

		if (get_HSB_temp_R(LM75R) != R_ERROR) {
			eeprom_data_map.tempR = read_data[0]; // ignore fractional part, eg: 27.125 deg, display 27 deg
		} else {
			/* TODO */
			eeprom_data_map.tempR = 0xff;
		}
		
		LOG("LM75:");
		uint2str(eeprom_data_map.tempL);
		LOG(int2str_tmp);
		USARTWriteChar(' ');
		uint2str(eeprom_data_map.tempR);
		LOG(int2str_tmp);
		USARTGotoNewLine();

		if (sizeof(eeprom_data_map) <= PAGE_SIZE) {
			write(I2C_ADDR, WORD_ADDR_RW, sizeof(eeprom_data_map) - RO_SIZE, peeprom);
			__delay_ms(EEPROM_PAGE_W_PERIOD);
			read(I2C_ADDR, WORD_ADDR_RW, sizeof(eeprom_data_map) - RO_SIZE, read_data);
		} else {
			/* several pages write */
			// TODO
		}
		
		if (EEPROM_WR_check(sizeof(eeprom_data_map) - RO_SIZE, peeprom) == true) {
			LOG("I/V/temp to EEPROM OK\r\n");
		} else {
			printEEPROMData(sizeof(eeprom_data_map) - RO_SIZE, read_data);
			LOG("I/V/temp to EEPROM NOK\r\n");
		}

		__delay_ms(EEPROM_PAGE_W_PERIOD);
		read(I2C_ADDR, WORD_ADDR_RW, sizeof(eeprom_data_map), read_data);
		
		set_dac_value(DAC_OUTPUT);
		eeprom_data_map.dac_output = DAC_OUTPUT;
		LOG("DC2DC(V):");
		uint2str(eeprom_data_map.dac_output);
		LOG(int2str_tmp);
		USARTGotoNewLine();
		
		set_resistor(DIGITAL_R);// TODO
		get_resistor();
		eeprom_data_map.resistor = DIGITAL_R;
		LOG("AD5246(ohm):");
		uint2str(eeprom_data_map.resistor);
		LOG(int2str_tmp);
		USARTGotoNewLine();
		
		LOG("check:");
		printEEPROMData(sizeof(eeprom_data_map), read_data);
		USARTGotoNewLine();
		
		__delay_ms(LOOP_CYCLE);
		//If we have some data
		if(n != 0) {
			//Read it
			char data = USARTReadData();
			USARTWriteChar(data);
		}
	}
}

uint8_t uint2str(uint16_t i)
{
	uint16_t base = 10000;
	uint8_t j = 0;

	if (i == 0) {
		int2str_tmp[j++] = '0';
		int2str_tmp[j++] = '\0';
		return 0;
	}
	
	while (i / base == 0 && base != 0) {
		base /= 10;
	}

	while ( base != 0) {
		int2str_tmp[j++] = i / base + '0';
		i %= base;
		base /= 10;
	}
	
	int2str_tmp[j] = '\0';

	return 0;
}

void GPIO_init(void)
{
	LCE_SetDigitalOutput();
	LCE_SetPullup();
	LCE_SetLow();
}

void WDT_Initialize(void)
{
    // WDTPS 1:65536; SWDTEN OFF; 
    WDTCON = 0x16;
}

void OSCILLATOR_Initialize(void)
{
    // SCS FOSC; SPLLEN disabled; IRCF 16MHz_HF; 
    OSCCON = 0x78;
    // SOSCR disabled; 
    OSCSTAT = 0x00;
    // TUN 0; 
    OSCTUNE = 0x00;
    // SBOREN disabled; BORFS disabled; 
    BORCON = 0x00;
}

void printEEPROMData(uint8_t size, uint8_t *buf)
{
	uint8_t i = 0;

	for (i = 0; i < size; i++) {
		USARTWriteChar(buf[i]);
	}
}

bool EEPROM_WR_check(uint8_t size, uint8_t *buf)
{
	uint8_t i = 0;
	
	for (i = 0; i < size; i++) {
		if (buf[i] != read_data[i]) {
			return false;
		}
	}

	return true;
}



