#ifndef HUAWEI_R48XX_MGR_H
#define HUAWEI_R48XX_MGR_H

#include "stm32f4xx_can.h"
#include "eltek_cmn.h"
#include "huawei_r48xx_rectifier.h"
#include "IPwrModule.h"
#include "rectifier.h"
#if defined(POWER_SUPPLY_USE_HUAWEI_R48XX)

class huaWeiR48xxMgr : public IPwrModule
{
public:
    huaWeiR48xxMgr();

    virtual void init();

	virtual void process100ms();
	virtual PsuSpec* getRectifierSpecById(uint8_t id);
	virtual PsuSpec* getRectifierSpecByIndex(uint8_t id);
	virtual PsuInfo* getRectifierInforById(uint8_t id);		
	virtual PsuInfo* getRectifierInforByIndex(uint8_t id);	
	virtual uint8_t getNumRectifiers();
	virtual uint16_t getRectMeasuredCurrentbyId(uint8_t id);
	virtual uint16_t getRectMeasuredVoltagebyId(uint8_t id);
	virtual bool checkPowerCond(HBWorkCond*		pHBWorkCond);
	
	void processDescMsg(const CanRxMsg* msg);
	void processRxMsg(const CanRxMsg &msg);
	void processLogonMsg(const CanRxMsg* msg);
	void processRtDataMsg(const CanRxMsg* msg); // 0x40
	void processModuleInfoMsg(const CanRxMsg* msg); // 0x50
    //void processStatusMsg(const CanRxMsg* msg);
    //void processGenMsg(const CanRxMsg* msg);
	
    void sendDefaultVoltageReq();
    void sendDefaultVoltage();
	void sendSetOutputVoltage(uint16_t vol);
	void sendSetDefaultOutputVoltage();
	void sendGetDataReq();
	void sendGetInfoReq();
	void sendGetDescReq(uint32_t addr);	
    void sendTurnOnOff(uint8_t id, bool on);
	void sendRectifierReq();

	void setHuaWeiR48xxEnabled(void);

#if defined(POWER_SUPPLY_SUPPORT_MULTI_MODULE)
	virtual void* rectifierBySerial(uint64_t serial);
    virtual void* rectifierById(uint8_t  id);
	virtual void* rectifierByIndex(uint8_t	index);
	void* rectifiers[MAX_PSU_PER_SLAVE];
#else
	huaWeiR48xxRectifier* rectifierBySerial(uint64_t serial);
    huaWeiR48xxRectifier* rectifierById(uint8_t  id);
	huaWeiR48xxRectifier rectifiers[MAX_PSU_PER_SLAVE];
#endif
    
    uint8_t numRectifiers;
    
	bool enabled;
    uint16_t measuredVoltage;
	char data[2][512];
	uint16_t size[2];
	
	

	uint8_t debug;

private:
	void transferDate(huaWeiR48xxRectifier *rect, char *date);
	uint16_t transferStrToInt(char *str);
};
extern huaWeiR48xxMgr g_huaWeiR48xxMgr;

#endif

#endif
