#ifndef SAG6400_MGR_H
#define SAG6400_MGR_H

#include "stm32f4xx_can.h"
#include "sag6400_rectifier.h"
#include "IPwrModule.h"

class Sag6400Mgr : public IPwrModule
{
public:
	Sag6400Mgr();

	virtual void init();
	virtual void process100ms();

	virtual PsuSpec* getRectifierSpecById(uint8_t id);
	virtual PsuSpec* getRectifierSpecByIndex(uint8_t id);
	virtual PsuInfo* getRectifierInforById(uint8_t id);		
	virtual PsuInfo* getRectifierInforByIndex(uint8_t id);
	virtual void setEnable();
	virtual uint8_t getNumRectifiers();
	virtual uint16_t getRectMeasuredCurrentbyId(uint8_t id);
	virtual uint16_t getRectMeasuredVoltagebyId(uint8_t id);
	virtual bool checkPowerCond(HBWorkCond*		pHBWorkCond);
	
	void sagprocess100ms(void);
	void sagprocessRxMsg(CanRxMsg msg);
	void processSagLogonMsg(const CanRxMsg msg);

#if defined(POWER_SUPPLY_SUPPORT_MULTI_MODULE)
	virtual void* rectifierBySerial(uint64_t serial);
    virtual void* rectifierById(uint8_t  id);
	virtual void* rectifierByIndex(uint8_t  index);
#else
	SagRectifier* sagrectifierById(uint8_t id);
#endif
	void sendSetDefaultOutputRequest(void);
	void sendSetVoltageOutputRequest(uint16_t voltage);
	void setSagEnabled(void);
	bool getSagEnabled(void);

	uint8_t numRectifiers;
#if defined(POWER_SUPPLY_SUPPORT_MULTI_MODULE)
	void* rectifiers[MAX_PSU_PER_SLAVE];
#else
	SagRectifier rectifiers[MAX_PSU_PER_SLAVE];
#endif
private:
	//uint16_t sag_measured_current;
    //uint16_t sag_measured_voltage;
	//uint16_t sag_status;
	bool enabled;
	uint8_t debug;	
};

extern Sag6400Mgr g_sag6400Mgr;
#endif
