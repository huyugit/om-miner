/*******************************************************************************
*  file    : eltekcontrol.cpp
*  created : 06.04.2015
*  author  : Slyshyk Oleksiy
*******************************************************************************/

#include <algorithm>
#include <stdio.h>

#include "eltek_mgr.h"
#include "power_mgr.h"
#include "eltek_msg.h"
#include "stm32f4xx_can.h"
#include "can.h"
#include "mytime.h"
#include "master_gate.h"
#include "board_mgr.h"
#include "ms_error.h"

EltekMgr g_eltekMgr;


static const EltekGenMessage reqMinorAlarm  (15, 0, 1, 1);
static const EltekGenMessage reqMajorAlarm  (15, 0, 1, 2);

static const EltekGenMessage reqProdDesc    (15, 0, 10, 0);
static const EltekGenMessage reqProdPart    (15, 0, 10, 1);
static const EltekGenMessage reqProdVer     (15, 0, 10, 3);
static const EltekGenMessage reqProdYear    (15, 0, 10, 5);
static const EltekGenMessage reqProdMonth   (15, 0, 10, 6);
static const EltekGenMessage reqProdDay     (15, 0, 10, 7);

static const EltekGenMessage reqGreenLed    (5, 0, 33, 0);
static const EltekGenMessage reqYellowLed   (5, 0, 32, 0);
static const EltekGenMessage reqRedLed      (5, 0, 31, 0);

static const EltekGenMessage reqUpTime      (15, 0,100, 0);
static const EltekGenMessage reqFanSpeedRef ( 8, 0, 35, 0);
static const EltekGenMessage reqFanSpeed    ( 8, 1, 23, 0);
static const EltekGenMessage reqDefVoltage  (7, 0, 37, 5);

static EltekGenMessage reqSetStatusInterval(15, 0, 1, 6);
static EltekGenMessage reqTurnOnOff(11, 0, 38, 1);


bool EltekMgr::debugOn = false;


EltekMgr::EltekMgr()
#if defined(POWER_SUPPLY_SUPPORT_MULTI_MODULE)
    : enabled(true),
#else
	: enabled(false),
#endif
      measuredVoltage(0),
      lastHiVoltageTime(0),
      lastHiVoltageDropTime(0)
{
	numRectifiers = 0; 					/* Very important. Because it, I wasted half of day. */
}

void EltekMgr::init()
{
    log("INIT: init CAN bus\n");

    Can::init();
	rectifiers[0] = new EltekRectifier();
	rectifiers[1] = new EltekRectifier();
    sendLoginRequest();
}

void EltekMgr::printShort()
{
    uint32_t now = getMiliSeconds();

    log("*** PSU: ON=%u CFG=%uV  HVPNG=%d/%d ms  SET=%uV GET=%uV  ST=%s (%u ms)\n",
        g_psuConfig.powerOn, g_psuConfig.voltage,
        now - lastHiVoltageTime,
        now - lastHiVoltageDropTime,
        g_powerMgr.setVoltage, measuredVoltage,
        powerStateToStr(g_powerMgr.state), now - g_powerMgr.stateTime);
}

void EltekMgr::printInfo()
{
    uint32_t now = getMiliSeconds();

    if (!enabled)
    {
        log("*** PSU (no can): ON=%u CFG=%uV MST=%s\n",
            g_psuConfig.powerOn, g_psuConfig.voltage,
            powerStateToStr((PowerState)g_psuConfig.masterPowerState));
        return;
    }

    printShort();

    log("-----------------------------------------------------\n");
    for (int i = 0; i < numRectifiers; ++i)
    {
    #if defined(POWER_SUPPLY_SUPPORT_MULTI_MODULE)
        EltekRectifier &r = *(EltekRectifier*)rectifiers[i];
	#else
		EltekRectifier &r = rectifiers[i];
	#endif

        log("PSU[%u]: %s, part: [%10s], version: [%1s], prodDate: %04u-%02u-%02u, desc: [%s]\n",
            r.info.id,
            r.info.serial,
            r.spec.prodPart, r.spec.prodVer,
            r.spec.prodYear, r.spec.prodMonth, r.spec.prodDay,
            r.spec.prodDesc);
    }

    log("-----------------------------------------------------\n");
    for (int i = 0; i < numRectifiers; ++i)
    {
        EltekRectifier &r = *(EltekRectifier*)rectifiers[i];
        int age = (int)now - r.info.lastStatusTime;
        if (age < 0) age = 0;

        log("PSU[%u]: AGE: %3u, NUM: %3u, SE: %3u, REQ: %2u, Tin: %2u, Tout: %2u, Vout: %4u, A:%u, Vin: %3u, DVO: %4u, LED: %s, UT: %u, FAN: %u/%u, cons: [%s]\n",
            r.info.id,
            age / 1000,
            r.info.numStatus,
            r.seqErrors,
            r.requests.size(),
            r.info.tempIn,
            r.info.tempOut,
            r.info.measuredVoltage,
            r.info.measuredCurrent,
            r.info.measuredInVoltage,
            r.info.defaultVoltage,
            EltekLedsToStr(r.info.greenLed, r.info.yellowLed, r.info.redLed).str,
            r.info.upTime,
            r.info.fanSpeedRef, r.info.fanSpeed,
            eltekConditionToStr(r.info.condition));

        if (r.requests.size() > 0)
        {
            log("PSU[%u]: REQ: ", r.info.id);
            uint32_t m = r.requests.mask;
            for (int idx = 0; idx < ELTEK_DATA_NUM; idx++)
            {
                if (m & (1 << idx)) log("%d ", idx);
            }
            log("\n");
        }
    }

    for (int i = 0; i < numRectifiers; ++i)
    {
        EltekRectifier &r = *(EltekRectifier*)rectifiers[i];
        if (r.info.majorAlarm || r.info.minorAlarm)
        {
            log("PSU[%u]: %s\n", r.info.id,
                EltekAlarmSetToStr(r.info.majorAlarm, r.info.minorAlarm).str);
        }
    }
}

void EltekMgr::process100ms()
{
    if (!enabled) return;

    for (int i = 0; i < 5; i++)
    {
        if (Can::rxMsgSize())
        {
            CanRxMsg msg = Can::rxPop();
            processRxMsg(msg);
        }
        else {
            break;
        }
    }

    g_powerMgr.step();


    uint16_t voltage = g_powerMgr.calcSetVoltage();
    uint32_t now = getMiliSeconds();

    static uint32_t statusCfgTime = 0;
    if (now - statusCfgTime > 10*1000)
    {
        statusCfgTime = now;
        sendStatusInterval(0, 17*5); // 200ms * 5 = 1 sec

        static uint32_t num = 0;
        if (num < 5) {
            num++;
            sendDefaultVoltage();
        }

        sendDefaultVoltageReq();
    }

    if (1)
    {
        static uint32_t lastTime = 0;
        if (now - lastTime > 1000)
        {
            lastTime = now;
            if (numRectifiers == 0) {
                sendLoginRequest();
            }
            sendSystemStatus(voltage);
            sendFanSpeedX();
        }
    }

    if (voltage > 0)
    {
        static uint32_t lastTime = 0;
        if (now - lastTime > 1000)
        {
            lastTime = now;
        }
        else {
            sendRectifierRequests();
        }
    }
    else
    {
        static uint32_t lastTime = 0;
        if (now - lastTime > 1900)
        {
            lastTime = now;
            sendTurnOnOff(0, false);
        }
    }
}

PsuSpec *EltekMgr::getRectifierSpecById(uint8_t id)
{
	EltekRectifier* rect = (EltekRectifier*)rectifierById(id);
	return &(rect->spec);
}

PsuSpec* EltekMgr::getRectifierSpecByIndex(uint8_t id)
{
	EltekRectifier* rect = (EltekRectifier*)rectifierByIndex(id);
	return &(rect->spec);
}

PsuInfo* EltekMgr::getRectifierInforById(uint8_t id)
{
	EltekRectifier* rect = (EltekRectifier*)rectifierById(id);
	return &(rect->info);
}

PsuInfo* EltekMgr::getRectifierInforByIndex(uint8_t id)
{
	EltekRectifier* rect = (EltekRectifier*)rectifierByIndex(id);
	return &(rect->info);
}

uint8_t EltekMgr::getNumRectifiers()
{
	return numRectifiers;
}

uint16_t EltekMgr::getRectMeasuredCurrentbyId(uint8_t id)
{
	EltekRectifier* rect = (EltekRectifier*)rectifierById(id);
	return rect->info.measuredCurrent;
}

uint16_t EltekMgr::getRectMeasuredVoltagebyId(uint8_t id)
{
	EltekRectifier* rect = (EltekRectifier*)rectifierById(id);
	return rect->info.measuredVoltage;
}

bool EltekMgr::checkPowerCond(HBWorkCond*		pHBWorkCond)
{
	uint32_t errCode = 0;

	if(getNumRectifiers() < MAX_PSU_PER_SLAVE)
	{
		errCode = MS_SET_HW_ERR_CODE(ERR_PWR, ERR_TYPE_RET_LESS, 0);
		g_slaveError.saveErrorCode(errCode);
	}
	for(uint32_t num = 0; num < getNumRectifiers(); num++)
	{
		double PSU_Pout = getRectifierInforByIndex(num)->getPOut();
		double PSU_Vout = getRectifierInforByIndex(num)->getVOut();
		double PSU_Iout = getRectifierInforByIndex(num)->getIOut();
		uint16_t PSU_fanspeed = getRectifierInforByIndex(num)->fanSpeed;
		int8_t PSU_tempOut = getRectifierInforByIndex(num)->tempOut;

		if (PSU_Pout <= pHBWorkCond->PsuPoutLowTh)
		{
			errCode = MS_SET_HW_ERR_CODE(ERR_PWR, ERR_TYPE_PSU_P_LOW, num);
			g_slaveError.saveErrorCode(errCode);
		}
		if (PSU_Pout > pHBWorkCond->PsuPoutHighTh)
		{
			errCode = MS_SET_HW_ERR_CODE(ERR_PWR, ERR_TYPE_PSU_P_HIGH, num);
			g_slaveError.saveErrorCode(errCode);
		}
		if (PSU_Iout <= pHBWorkCond->PsuIoutLowTh)
		{
			errCode = MS_SET_HW_ERR_CODE(ERR_PWR, ERR_TYPE_PSU_I_LOW, num);
			g_slaveError.saveErrorCode(errCode);
		}
		if (PSU_Iout > pHBWorkCond->PsuIoutHighTh)
		{
			errCode = MS_SET_HW_ERR_CODE(ERR_PWR, ERR_TYPE_PSU_I_HIGH, num);
			g_slaveError.saveErrorCode(errCode);
		}
		if (PSU_Vout > pHBWorkCond->PsuVoutHighTh)
		{
			errCode = MS_SET_HW_ERR_CODE(ERR_PWR, ERR_TYPE_PSU_U_HIGH, num);
			g_slaveError.saveErrorCode(errCode);
		}
		if (PSU_fanspeed < pHBWorkCond->PsuFanSpeedLowTh)
		{
			errCode = MS_SET_HW_ERR_CODE(ERR_PWR, ERR_TYPE_PSU_SPD_LOW, num);
			g_slaveError.saveErrorCode(errCode);
		}
		if (PSU_tempOut > pHBWorkCond->PsuTempOutHighTh)
		{
			errCode = MS_SET_HW_ERR_CODE(ERR_PWR, ERR_TYPE_PSU_TMP_HIGH, num);
			g_slaveError.saveErrorCode(errCode);
		}			
		
	}
	
	if (errCode != 0)
		return false;
	else
		return true;
}

void EltekMgr::sendRectifierRequests()
{
    for (uint32_t i = 0; i < numRectifiers; ++i)
    {
        EltekRectifier &rect = *(EltekRectifier*)rectifiers[i];

        // re-request state of leds periodically
        if (getMiliSeconds() - rect.reqLedsTime > 1000)
        {
            rect.reqLedsTime = getMiliSeconds();

            rect.requests.push(ELTEK_GREEN_LED);
            rect.requests.push(ELTEK_YELLOW_LED);
            rect.requests.push(ELTEK_RED_LED);
            rect.requests.push(ELTEK_UP_TIME);
            rect.requests.push(ELTEK_FAN_SPPED);
            rect.requests.push(ELTEK_FAN_SPPED_REF);
        }

        if (rect.requests.isEmpty())
            continue;

        if (!rect.info.numStatus)
            continue;

        for (int numReq = 0; numReq < 2; numReq++)
        {
            uint8_t reqId;
            if (!rect.requests.pop(reqId)) break;

            // Special case: turn ON rectifier
            // (send request will be triggered again automatically)
            if (reqId == ELTEK_TURN_ON)
            {
                sendTurnOnOff(rect.info.id, true);
                continue;
            }

            if (rect.requests.isEmpty())
            {
                rect.requests.push(ELTEK_GREEN_LED);
            }

            rect.requests.push(reqId);

            EltekGenMessage msg;
            const char* str = "N/A";

            switch (reqId)
            {
            case ELTEK_ALARM_MINOR:     msg = reqMinorAlarm;    str = "reqMinorAlarm";      break;
            case ELTEK_ALARM_MAJOR:     msg = reqMajorAlarm;    str = "reqMajorAlarm";      break;
            case ELTEK_PROD_DESC:       msg = reqProdDesc;      str = "reqProdDesc";        break;
            case ELTEK_PROD_PART:       msg = reqProdPart;      str = "reqProdPart";        break;
            case ELTEK_PROD_VER:        msg = reqProdVer;       str = "reqProdVer";         break;
            case ELTEK_PROD_YEAR:       msg = reqProdYear;      str = "reqProdYear";        break;
            case ELTEK_PROD_MONTH:      msg = reqProdMonth;     str = "reqProdMonth";       break;
            case ELTEK_PROD_DAY:        msg = reqProdDay;       str = "reqProdDay";         break;
            case ELTEK_GREEN_LED:       msg = reqGreenLed;      str = "reqGreenLed";        break;
            case ELTEK_YELLOW_LED:      msg = reqYellowLed;     str = "reqYellowLed";       break;
            case ELTEK_RED_LED:         msg = reqRedLed;        str = "reqRedLed";          break;
            case ELTEK_UP_TIME:         msg = reqUpTime;        str = "reqUpTime";          break;
            case ELTEK_FAN_SPPED_REF:   msg = reqFanSpeedRef;   str = "reqFanSpeedRef";     break;
            case ELTEK_FAN_SPPED:       msg = reqFanSpeed;      str = "reqFanSpeed";        break;
            default:                    STOP(); continue;
            }

            msg.setSystemIndex(rect.info.id);

            if (debugOn) log("%6u: PSU[%u]: send %s\n", getMiliSeconds(), msg.systemIndex, str);
            Can::send(msg.canTx());
        }
    }
}

void EltekMgr::processRxMsg(const CanRxMsg &msg)
{
    uint32_t msgType = EltekRxMsg::msgType(msg);
    uint32_t variant = EltekRxMsg::variant(msg);

    if (msgType == EMT_Status_LogOn && variant == ESV_Login_Request)
    {
        processLogonMsg(&msg);
    }
    else if (msgType == EMT_Status_LogOn && variant == ESV_Status)
    {
        processStatusMsg(&msg);
    }
    else if (msgType == EMT_GenMessage)
    {
        processGenMsg(&msg);
    }
    else
    {
        log("EltekMgr: UNEXPECTED msgType: %i, variant: %i\n",
            msgType, variant);
    }
}

void EltekMgr::processLogonMsg(const CanRxMsg *msg)
{
    EltekLogOnMsg lgm(*msg);

    log("EltekMgr: LOGON request: serial %s\n",
        EltekSerialToStr(lgm.serial()).str);

    EltekRectifier* rect = (EltekRectifier*)rectifierBySerial(lgm.serial());
    if (!rect)
    {
        if (numRectifiers >= MAX_PSU_PER_SLAVE)
        {
            log("EltekMgr: can not accept LOGON (limit reached)\n");
            return;
        }

        rect = (EltekRectifier*)rectifiers[numRectifiers++];
        rect->info.id       = numRectifiers;
        strncpy(rect->info.serial, EltekSerialToStr(lgm.serial()).str, 47);
		rect->info.ratedCurrent = 50;

        rect->spec.id       = rect->info.id;
        strncpy(rect->spec.serial, rect->info.serial, 47);

        log("EltekMgr: new PSU registered, total = %u\n", numRectifiers);
    }

    log("EltekMgr: send LOGON response: id = %u\n", rect->info.id);
    Can::send( lgm.logOnId(rect->info.id) );
}

void EltekMgr::processStatusMsg(const CanRxMsg *msg)
{
    EltekStatusMsg smg = EltekStatusMsg(*msg);
    if (debugOn)
    {
        log("%6u: RX PSU[%u]: inAir:%u, outAir:%u, V:%u, A:%u, mainsV:%u, cons: [%s]\n",
            getMiliSeconds(),
            smg.systemIndex(),
            smg.in_air_temperature(),
            smg.out_air_temperature(),
            smg.measured_voltage(),
            smg.measured_current(),
            smg.measured_mains_voltage(),
            eltekConditionToStr(smg.condition()));
    }

    EltekRectifier* rect = (EltekRectifier*)rectifierById(smg.systemIndex());
    if (rect)
    {
        bool hvPrev = isHv(rect->info.measuredInVoltage);

        rect->info.numStatus++;
        rect->info.lastStatusTime       = getMiliSeconds();

        rect->info.condition            = smg.condition();
        rect->info.tempIn               = smg.in_air_temperature();
        rect->info.tempOut              = smg.out_air_temperature();
        rect->info.measuredVoltage      = smg.measured_voltage();
        rect->info.measuredCurrent      = smg.measured_current();
        rect->info.measuredInVoltage    = smg.measured_mains_voltage();

        if (rect->info.measuredVoltage > 20000)
        {
            // psu Eltek reports 650V output voltage on failure
            // we are zero such values to avoid missunderstanhding
            rect->info.measuredVoltage = 0;
        }

        bool hvNow = isHv(rect->info.measuredInVoltage);

        if (rect->info.condition == COND_Disabled && g_powerMgr.setVoltage)
        {
            rect->requests.push(ELTEK_TURN_ON);
        }

        if (rect->info.condition == COND_Minor_Alarm)
        {
            rect->requests.push(ELTEK_ALARM_MINOR);
        }
        else {
            rect->info.minorAlarm = 0;
        }

        if (rect->info.condition == COND_Major_Alarm)
        {
            rect->requests.push(ELTEK_ALARM_MAJOR);
        }
        else {
            rect->info.majorAlarm = 0;
        }

        measuredVoltage = rect->info.measuredVoltage;

        if (hvNow) {
            lastHiVoltageTime = getMiliSeconds();
        }
        if (hvPrev && !hvNow)
        {
            lastHiVoltageDropTime = getMiliSeconds();
        }
    }
}

void EltekMgr::processGenMsg(const CanRxMsg *canMsg)
{
    EltekGenMessage msg(*canMsg);

    EltekRectifier* rect = (EltekRectifier*)rectifierById(msg.systemIndex);
    if (!rect)
    {
        log("EltekMgr: rectifier not found (id=%u, subsystem=%u, subsystemIndex=%u)!\n",
            msg.systemIndex, msg.subSystem, msg.subSystemIndex);
        return;
    }

//    log("%6u: processGenMsg: systemIndex = %u, messageIndex = %u, start = %u\n",
//        getMiliSeconds(), msg.systemIndex, msg.messageIndex, msg.isStartOfFrame);

    if (msg.messageIndex != 0)
    {
        EltekGenMessage& msgSeq = rect->msgSeq;

        if (!msgSeq.addMessage(msg))
        {
            rect->seqErrors++;
            return;
        }

        if (!msgSeq.isComplete)
            return;

        msg = msgSeq;
    }

    if (msg.isResponeTo(reqMinorAlarm))
    {
    	uint16_t minorAlarm = msg.data[0];
		minorAlarm = (minorAlarm<<8) | msg.data[1];
		rect->onMinorAlarm(minorAlarm);
    }
    else if (msg.isResponeTo(reqMajorAlarm))
    {
        uint16_t majorAlarm = msg.data[0];
		majorAlarm = (majorAlarm<<8) | msg.data[1];
        rect->onMajorAlarm(majorAlarm);
    }
    else if (msg.isResponeTo(reqProdDesc))
    {
        rect->onProdDesc( (char*)msg.data );
    }
    else if (msg.isResponeTo(reqProdPart))
    {
        rect->onProdPart( (char*)msg.data );
    }
    else if (msg.isResponeTo(reqProdVer))
    {
        rect->onProdVer( (char*)msg.data );
    }
    else if (msg.isResponeTo(reqProdYear))
    {
        uint16_t prodYear = msg.data[0];
		prodYear = (prodYear<<8) | msg.data[1];
        rect->onProdYear(prodYear);
    }
    else if (msg.isResponeTo(reqProdMonth))
    {
        rect->onProdMonth(  msg.data[0] );
    }
    else if (msg.isResponeTo(reqProdDay))
    {
        rect->onProdDay( msg.data[0] );
    }
    else if (msg.isResponeTo(reqGreenLed))
    {
        rect->onGreenLed( msg.data[0] );
    }
    else if (msg.isResponeTo(reqYellowLed))
    {
        rect->onYellowLed( msg.data[0] );
    }
    else if (msg.isResponeTo(reqRedLed))
    {
        rect->onRedLed( msg.data[0] );
    }

    else if (msg.isResponeTo(reqUpTime))
    {
        uint32_t upTime = msg.data[0];
		upTime = (upTime<<8) | msg.data[1];
		upTime = (upTime<<8) | msg.data[2];
		upTime = (upTime<<8) | msg.data[3];
        rect->onUpTime(upTime);
    }
    else if (msg.isResponeTo(reqFanSpeedRef))
    {
        uint16_t fanSpeedRef = msg.data[0];
		fanSpeedRef = (fanSpeedRef<<8) | msg.data[1];
        rect->onFanSpeedRef(fanSpeedRef);
    }
    else if (msg.isResponeTo(reqFanSpeed))
    {
        uint16_t fanSpeed = msg.data[0];
		fanSpeed = (fanSpeed<<8) | msg.data[1];
        rect->onFanSpeed(fanSpeed);
    }
    else if (msg.isResponeTo(reqSetStatusInterval))
    {}
    else if (msg.isResponeTo(reqTurnOnOff))
    {}
    else if (msg.isResponeTo(reqDefVoltage))
    {
        uint16_t defaultVoltage = msg.data[0];
		defaultVoltage = (defaultVoltage<<8) | msg.data[1];
        rect->onDefaultVoltage(defaultVoltage);
    }
    else
    {
        log("UNEXPECTED GEN MESSAGE: "); msg.dumpAddr();
    }
}

void* EltekMgr::rectifierBySerial(uint64_t serial)
{
	for (uint32_t i = 0; i < numRectifiers; i++)
	{
		if (strncmp(getRectifierInforByIndex(i)->serial, EltekSerialToStr(serial).str, 48) == 0)
		{
			return rectifiers[i];
		}
	}

	return nullptr;
}

void* EltekMgr::rectifierById(uint8_t  id)
{
	for (uint32_t i = 0; i < numRectifiers; i++)
	{
		if (getRectifierInforByIndex(i)->id == id)
		{
			return rectifiers[i];
		}
	}

	return nullptr;
}

void* EltekMgr::rectifierByIndex(uint8_t	index)
{
	return rectifiers[index];
}

void EltekMgr::sendSystemStatus(uint16_t voltage)
{
    EltekControlSystemStatusMsg msg;
    msg.setMeasuredOutputVoltage(voltage);
    msg.setOutputVoltageReference(voltage);

    if (debugOn) log("EltekMgr: send SystemStatusMsg(V=%u)\n", g_psuConfig.voltage);
    Can::send(msg.canMessage());
}

void EltekMgr::sendLoginRequest()
{
    EltekGenMessage msg;
    msg.subSystem = 15;
    msg.subSystemIndex = 1;
    msg.setFunction(21);

    log("EltekMgr: send askLoginRequest\n");
    Can::send(msg.canTx());
    Can::send2(msg.canTx());
}

void EltekMgr::sendDefaultVoltageReq()
{
    EltekGenMessage msg = reqDefVoltage;
    msg.setFunction(0);

    log("EltekMgr: send sendDefaultVoltageReq()\n");
    Can::send(msg.canTx());
}

void EltekMgr::sendDefaultVoltage()
{
    EltekGenMessage msg = reqDefVoltage;
    msg.setFunction(1);

    uint16_t voltage = g_psuConfig.defaultVoltage;

    ASSERT(sizeof(voltage) == 2, "ERROR");
    msg.setData((uint8_t*)&voltage, sizeof(voltage));

    log("EltekMgr: send setDefaultVoltage(nv=%u)\n", voltage);
    Can::send(msg.canTx());
}


void EltekMgr::sendTurnOnOff(uint8_t id, bool on)
{
    EltekGenMessage &msg = reqTurnOnOff;
    msg.setFunction(1);
    msg.setSystemIndex(id);

    if (on) {
        uint8_t d[] = {1, 0, 0, 0, 0};
        msg.setData(d, sizeof(d));
    }
    else {
        uint8_t d[] = {1, 255, 1, 1, 0};
        msg.setData(d, sizeof(d));
    }

    log("EltekMgr: send sendTurnOnOff(id=%u, on=%u)\n", id, on);
    Can::send(msg.canTx());
}

void EltekMgr::sendStatusInterval(uint8_t id, uint8_t interval)
{
    EltekGenMessage &msg = reqSetStatusInterval;
    msg.setFunction(1);
    msg.setSystemIndex(id);

    ASSERT(sizeof(interval) == 1, "ERROR");
    msg.setData(&interval, 1);

    log("EltekMgr: send setStatusInterval(id=%u, in=%d)\n", id, interval);
    Can::send(msg.canTx());
}

void EltekMgr::sendFanSpeedX()
{
    if (!g_psuConfig.fanSpeedUser)
        return;

    for (int i = 0; i < numRectifiers; ++i)
    {
        EltekRectifier &r = *(EltekRectifier*)rectifierByIndex(i);

        EltekGenMessage msg = reqFanSpeedRef;
        msg.setFunction(1);
        msg.setDataElement(1);

        msg.setSystemIndex(r.info.id);

        uint8_t speed = g_psuConfig.fanSpeedValue;

        if (r.isPsuHe) {
            uint8_t d[] = {0x01, 0x04, 0x00, speed, 0x00};
            msg.setData(d, sizeof(d));
        }
        else if (r.isPsuHeDc1) {
            uint8_t d[] = {0x01, 0x7f, speed, 0x00, 0x00};
            msg.setData(d, sizeof(d));
        }
        else {
            continue;
        }

        log("EltekMgr: send sendFanSpeedX\n");
        Can::send(msg.canTx());

//        CanTxMsg m2 = msg.canTx();
//        log("0x%08x  0x%02x 0x%02x 0x%02x  0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
//            m2.ExtId,
//            m2.Data[0], m2.Data[1], m2.Data[2], m2.Data[3],
//            m2.Data[4], m2.Data[5], m2.Data[6], m2.Data[7]);
    }
}


bool EltekMgr::isHv(uint16_t voltage)
{
    return (voltage > 100) && (voltage  < 300);
}

bool EltekMgr::isHvDetected()
{
    if (lastHiVoltageTime == 0)
    {
        return false;
    }
    else if (lastHiVoltageDropTime < lastHiVoltageTime)
    {
        return true;
    }
    else {
        return (getMiliSeconds() - lastHiVoltageDropTime < 5*1000);
    }
}
