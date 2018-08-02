#include "i2c_nt3h1x01.h"

#include "format.hpp"
#include "mytime.h"

#define NT3H1101_I2C_ADDR 0xAA

I2CNT3H1X01::I2CNT3H1X01(I2CSw &_i2c, uint8_t addr)
    : debug(false),
     i2c(_i2c),
     i2cAddr(NT3H1101_I2C_ADDR)
{
	addr = addr;
}

void I2CNT3H1X01::WriteOSCConfig(uint8_t OSC)
{
	uint8_t buff[16] = {0xEE, 0x00};

	buff[1] = OSC;

	writeMem(USER_MEM_I2C_BLOCK_ADDR, buff, 16);
}

uint8_t I2CNT3H1X01::ReadOSCConfig()
{
	uint8_t buff[16] = {0x00};

	readMem(USER_MEM_I2C_BLOCK_ADDR, buff, 16);

	if(buff[0] == 0xEE)
	{
		return buff[1];  //EEPRON Writed
	}
	else
	{
		return 0x00;   //EEPROM Not writed
	}
}

void I2CNT3H1X01::WriteSNforHash(char * sn)
{
	writeMem(USER_MEM_HASH_SN_ADDR, (uint8_t*)sn, 16);
}

void I2CNT3H1X01::ReadSNforHash(char * sn)
{
	readMem(USER_MEM_HASH_SN_ADDR, (uint8_t*)sn, 16);
}

void I2CNT3H1X01::WriteBinConfig(uint8_t bin)
{
	uint8_t buff[16] = {0xEE, 0x00};

	buff[1] = bin;

	writeMem(USER_MEM_BIN_ADDR, buff, 16);
}

uint8_t I2CNT3H1X01::ReadBinConfig()
{
	uint8_t buff[16] = {0x00};

	readMem(USER_MEM_BIN_ADDR, buff, 16);

	if(buff[0] == 0xEE)
	{
		return buff[1];  //EEPRON Writed
	}
	else
	{
		return 0x00;   //EEPROM Not writed
	}
}

bool I2CNT3H1X01::readMem(int addr, uint8_t *dest, int size)
{
	 i2c.stopCond();
	 i2c.startCond();

	 //log("readMem start\n");

	 if (!i2c.txByte(i2cAddr | I2C_WRITE)) // slave address + WRITE operation bit
	 {
		 i2c.stopCond();
		 if (debug) log("readMem NT3H1X01_ERROR_SA\n");
		 return false;
	 }

	 if (!i2c.txByte(addr)) // pointer register
	 {
		 i2c.stopCond();
		 if (debug) log("readMem NT3H1X01_ERROR_PTR\n");
		 return false;
	 }

	 i2c.stopCond();

	 i2c.startCond();

	 if (!i2c.txByte(i2cAddr | I2C_READ)) // slave address + READ operation bit
	 {
		 i2c.stopCond();
		 if (debug) log("readMem NT3H1X01_ERROR_SA_2\n");
		 return false;
	 }

	 for (int i = 0; i < size; i++)
	 {
		 i2c.rxByte(dest[i], (i == size - 1));
	 }

	 i2c.stopCond();

	 //log("NT3H1X01_read result:\n");

	 if (debug) hexdump8(dest, size);
	 return true;
}

bool I2CNT3H1X01::writeMem(int addr, uint8_t *dest, int size)
{
	i2c.stopCond();
	i2c.startCond();
	//log("writeMem start\n");

	if (!i2c.txByte(i2cAddr | I2C_WRITE)) // slave address + WRITE operation bit
	{
		i2c.stopCond();
		if (debug) log("writeMem NT3H1X01_ERROR_SA\n");
		return false;
	}

	if (!i2c.txByte(addr)) // pointer register
	{
		i2c.stopCond();
		if (debug) log("writeMem NT3H1X01_ERROR_PTR\n");
		return false;
	}

	for (int i = 0; i < size; i++)
	{
		if (!i2c.txByte(dest[i])) {
			if (debug) log("writeMem NT3H1X01_ERROR_WD\n");
		}
	}

	i2c.stopCond();


	if (debug) hexdump8(dest, size);
	return true;
}



