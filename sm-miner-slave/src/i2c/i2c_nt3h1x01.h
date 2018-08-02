#ifndef I2C_NT3H1X01_H
#define I2C_NT3H1X01_H

#include <stdint.h>
#include "i2c_sw.h"

#define I2C_BLOCK_ADDRESS_START  0x00

#define USER_MEM_I2C_BLOCK_ADDR		I2C_BLOCK_ADDRESS_START+0x01
#define USER_MEM_HASH_SN_ADDR		I2C_BLOCK_ADDRESS_START+0x02
#define USER_MEM_BIN_ADDR			I2C_BLOCK_ADDRESS_START+0x03

class I2CNT3H1X01
{
public:
	I2CNT3H1X01(I2CSw &_i2c, uint8_t addr);
	void WriteOSCConfig(uint8_t OSC);
	uint8_t ReadOSCConfig();
	void WriteSNforHash(char * sn);
	void ReadSNforHash(char * sn);
	void WriteBinConfig(uint8_t bin);
	uint8_t ReadBinConfig();
	
	bool debug;

private:
    I2CSw &i2c;
    uint8_t i2cAddr;

    bool readMem(int addr, uint8_t* dest, int size);
    bool writeMem(int addr, uint8_t* dest, int size);
};
#endif
