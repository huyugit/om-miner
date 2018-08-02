#ifndef _I2C_H
#define	_I2C_H

#include <stdint.h>
#include "config.h"

#ifdef PIC16F1704
#define SSPIF SSP1IF
#endif

#define LM75L 0x94 // temp sensor i2c addr
#define LM75R 0x96

#define AD5246 0x5C // digital resistor i2c addr
#define I2C_ADDR 0xA0 // eeprom i2c adddr

#ifdef	__cplusplus
extern "C" {
#endif

void init_i2c(void);
void AD5246_write(uint8_t i2c_addr, uint8_t data);
uint8_t AD5246_read(uint8_t i2c_addr, uint8_t *buf);

uint8_t write(uint8_t i2c_addr, uint8_t word_addr, uint8_t size, uint8_t *buf);
uint8_t read(uint8_t i2c_addr, uint8_t word_addr, uint8_t size, uint8_t *buf);
void get_resistor(void);
void set_resistor(uint8_t data);

uint8_t get_HSB_temp_L(uint8_t addr);
uint8_t get_HSB_temp_R(uint8_t addr);


#ifdef	__cplusplus
}
#endif

#endif	/* USART_PIC16_H */

