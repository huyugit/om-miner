#ifndef IPWRMODULE_H
#define IPWRMODULE_H

#include "stm32f4xx_can.h"
#include "eltek_cmn.h"
#include "ms_data_psu.h"
#include "rectifier.h"

struct HBWorkCond{
	double 			PsuPoutLowTh;
	double 			PsuPoutHighTh;
	double 			PsuIoutLowTh;
	double 			PsuVoutHighTh;
	double 			PsuIoutHighTh;
	uint16_t 		PsuFanSpeedLowTh;
	uint16_t 		FanRpmLowTh;
	uint8_t 		FanFlag;
	uint8_t 		HashTempFlag;
	int8_t			HashTempHi;
	uint8_t 		PsuFlag;
	uint8_t 		FanWorkMode;
	int8_t 			PsuTempOutHighTh;
}
__attribute__ ((packed));

class IPwrModule
{
public:
	virtual void init(){};
	virtual void process100ms(){};

	virtual PsuSpec* getRectifierSpecById(uint8_t id){id = id; return nullptr;};
	virtual PsuSpec* getRectifierSpecByIndex(uint8_t id){id = id; return nullptr;};
	virtual PsuInfo* getRectifierInforById(uint8_t id){id = id; return nullptr;};		
	virtual PsuInfo* getRectifierInforByIndex(uint8_t id){id = id; return nullptr;};
	virtual void setEnable(){};
	virtual uint8_t getNumRectifiers(){return 0;};
	virtual uint16_t getRectMeasuredCurrentbyId(uint8_t id){id = id; return 0;};
	virtual uint16_t getRectMeasuredVoltagebyId(uint8_t id){id = id; return 0;};
	virtual bool checkPowerCond(HBWorkCond*		pHBWorkCond){return (nullptr == pHBWorkCond)?false:true;};
	virtual void* rectifierBySerial(uint64_t serial){serial = serial; return nullptr;};
    virtual void* rectifierById(uint8_t id){id= id; return nullptr;};
	virtual void* rectifierByIndex(uint8_t     index){index = index; return nullptr;};
};

#endif

