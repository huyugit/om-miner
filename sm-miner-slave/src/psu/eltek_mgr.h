/*******************************************************************************
*  file    : eltekcontrol.hpp
*  created : 06.04.2015
*  author  : Slyshyk Oleksiy
*******************************************************************************/

#ifndef ELTEK_MGR_H
#define ELTEK_MGR_H

#include "stm32f4xx_can.h"
#include "eltek_cmn.h"
#include "eltek_rectifier.h"
#include "IPwrModule.h"


class EltekMgr : public IPwrModule
{
public:
    EltekMgr();

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

    void printShort();
    void printInfo();
    void sendRectifierRequests();
    void processRxMsg(const CanRxMsg &msg);

    void processLogonMsg(const CanRxMsg* msg);
    void processStatusMsg(const CanRxMsg* msg);
    void processGenMsg(const CanRxMsg* msg);

    void sendSystemStatus(uint16_t voltage);
    void sendLoginRequest();
    void sendDefaultVoltageReq();
    void sendDefaultVoltage();
    void sendTurnOnOff(uint8_t id, bool on);
    void sendStatusInterval(uint8_t id, uint8_t interval);
    void sendFanSpeedX();

#if defined(POWER_SUPPLY_SUPPORT_MULTI_MODULE)
	virtual void* rectifierBySerial(uint64_t serial);
    virtual void* rectifierById(uint8_t  id);
	virtual void* rectifierByIndex(uint8_t  index);
#else
    EltekRectifier* rectifierBySerial(uint64_t serial);
    EltekRectifier* rectifierById(uint8_t  id);
#endif

    bool isHv(uint16_t voltage);
    bool isHvDetected();


    bool enabled;

    uint8_t numRectifiers;
	#if defined(POWER_SUPPLY_SUPPORT_MULTI_MODULE)
	void* rectifiers[MAX_PSU_PER_SLAVE];
	#else
    EltekRectifier rectifiers[MAX_PSU_PER_SLAVE];
	#endif


    uint16_t measuredVoltage;

    uint32_t lastHiVoltageTime;
    uint32_t lastHiVoltageDropTime;

    static bool debugOn;
};

extern EltekMgr g_eltekMgr;

#endif // ELTEK_MGR_H
