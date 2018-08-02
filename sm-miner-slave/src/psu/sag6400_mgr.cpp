#include <algorithm>
#include <stdio.h>

#include "sag6400_mgr.h"
#include "stm32f4xx_can.h"
#include "can.h"
#include "mytime.h"
#include "master_gate.h"
#include "board_mgr.h"
#include "sag6400_msg.h"
#include "power_mgr.h"
#include "board_mgr.h"
#include "ms_error.h"
Sag6400Mgr g_sag6400Mgr;

Sag6400Mgr::Sag6400Mgr()
{
	enabled = true;
	numRectifiers = 0;
	debug = false;	
}

void Sag6400Mgr::init(void)
{
#if defined(POWER_SUPPLY_SUPPORT_MULTI_MODULE)
	rectifiers[0] = new SagRectifier();
	rectifiers[1] = new SagRectifier();
#endif

	Can::init();
}

void Sag6400Mgr::process100ms()
{
	sagprocess100ms();
}

PsuSpec* Sag6400Mgr::getRectifierSpecById(uint8_t id)
{
	SagRectifier* rect = (SagRectifier*)rectifierById(id);
	return &(rect->spec);
}

PsuSpec* Sag6400Mgr::getRectifierSpecByIndex(uint8_t id)
{
	SagRectifier* rect = (SagRectifier*)rectifierByIndex(id);
	return &(rect->spec);
}

PsuInfo* Sag6400Mgr::getRectifierInforById(uint8_t id)
{
	SagRectifier* rect = (SagRectifier*)rectifierById(id);
	return (&rect->info);
}

PsuInfo* Sag6400Mgr::getRectifierInforByIndex(uint8_t id)
{
	SagRectifier* rect = (SagRectifier*)rectifierByIndex(id);
	return (&rect->info);
}

void Sag6400Mgr::setEnable()
{

}

uint8_t Sag6400Mgr::getNumRectifiers()
{
	return numRectifiers;
}

uint16_t Sag6400Mgr::getRectMeasuredCurrentbyId(uint8_t id)
{
	SagRectifier* rect = (SagRectifier*)rectifierById(id);
	return rect->info.measuredCurrent;
}

uint16_t Sag6400Mgr::getRectMeasuredVoltagebyId(uint8_t id)
{
	SagRectifier* rect = (SagRectifier*)rectifierById(id);
	return rect->info.measuredVoltage;
}

void Sag6400Mgr::sagprocess100ms(void)
{
	if (debug)
	{
		log("sagprocess100ms: enabled = %d\n", enabled);
	}

	if (!enabled) return;
	
	for (int i = 0; i < 5; i++)
	{
		if (Can::rxMsgSize())
		{
			CanRxMsg msg = Can::rxPop();
			sagprocessRxMsg(msg);
			processSagLogonMsg(msg);
		}
		else {
			break;
		}
	}

	g_powerMgr.step();

	uint16_t voltage = g_powerMgr.calcSetVoltage();
    uint32_t now = getMiliSeconds();

	static uint32_t statusCfgTime = 0;

	if (debug)
	{
		log("process100ms: statusCfgTime = %d,now = %d\n", statusCfgTime,now);
	}
	
    if (now - statusCfgTime > 10*1000)
    {
        statusCfgTime = now;

        static uint32_t num = 0;
        if (num < 5) {
            num++;
            sendSetDefaultOutputRequest();
        }
    }

	if (1)
    {
        static uint32_t lastTime = 0;
        if (now - lastTime > 1000)
        {
            lastTime = now;
            if (debug)
            {
            	log("process100ms chenbo voltage = %d\n",voltage);
            }
			
            sendSetVoltageOutputRequest(voltage);
        }
    }

}

void Sag6400Mgr::sagprocessRxMsg(CanRxMsg msg)
{
	SagStatusMSG statMsg = SagStatusMSG(msg);

	if (debug)
	{
		log("Sag6400Mgr: %x, %x,%x,%x,%x,%x,%x,%x,%x\n",
            msg.ExtId, 
            msg.Data[0],
            msg.Data[1],
            msg.Data[2],
            msg.Data[3],
            msg.Data[4],
            msg.Data[5],
            msg.Data[6],
            msg.Data[7]);
	}
	
	SagRectifier* rect = (SagRectifier*)rectifierById(SagRXMSG::sagMsgSrcAddr(msg));
	if (rect)
	{
		if (SAG6400_VA_CMD1 == msg.Data[0])
		{
			rect->info.measuredVoltage      = statMsg.GetSagVoltage();
        	rect->info.measuredCurrent      = (statMsg.GetSagCurrent())/10;
		}

		if (SAG6400_INFO_CMD2 == msg.Data[0])
		{
			rect->info.measuredInVoltage	= statMsg.GetSagInVoltage();
		}
		
		if (SAG6400_TEMP_CMD3 == msg.Data[0])
		{
			rect->info.tempIn               = statMsg.GetSagEnvTemp();
			rect->info.tempOut              = statMsg.GetSagModleTemp();
		}		

		if (SAG6400_PRODUCT_MODEL_CMD10 == msg.Data[0])
		{
			statMsg.GetSagDesc(rect);
		}

		if (SAG6400_PRODUCT_MODEL_CMD11 == msg.Data[0])
		{
			statMsg.GetSagDesc(rect);
		}

		if (SAG6400_PART_CMD12 == msg.Data[0])
		{
			statMsg.GetSagPart(rect);
		}

		if (SAG6400_PART_CMD13 == msg.Data[0])
		{
			statMsg.GetSagPart(rect);
		}

		if (SAG6400_VERSION_CMD14 == msg.Data[0])
		{
			statMsg.GetSagVersion(rect->spec.prodVer, 6);
		}

		if (SAG6400_SERIAL_NUMBER_CMD15 == msg.Data[0])
		{
			statMsg.GetSagSerialNumber(rect);
		}

		if (SAG6400_SERIAL_NUMBER_CMD16 == msg.Data[0])
		{
			statMsg.GetSagSerialNumber(rect);
		}

		if (SAG6400_PRODUCT_DATE_CMD17 == msg.Data[0])
		{
			statMsg.GetSagDate(rect);
		}

		if (SAG6400_PRODUCT_DATE_CMD18 == msg.Data[0])
		{
			statMsg.GetSagDate(rect);
		}

		
		if (debug)
		{
			log("Sag6400Mgr: %d,%d,%d,%d,%x-%x-%x,%s\n", 
				rect->info.measuredVoltage,
                rect->info.measuredCurrent,
                rect->info.tempIn,
                rect->info.tempOut,
                rect->spec.prodYear,
                rect->spec.prodMonth,
                rect->spec.prodDay,
                rect->spec.prodVer);

		}
	}
}

void Sag6400Mgr::processSagLogonMsg(const CanRxMsg msg)
{
	SagRectifier* rect = (SagRectifier*)rectifierById(SagRXMSG::sagMsgSrcAddr(msg));

    if (!rect)
    {
        if (numRectifiers >= MAX_PSU_PER_SLAVE)
        {
            log("Sag6400Mgr: can not accept LOGON (limit reached)\n");
            return;
        }

		rect = (SagRectifier*)rectifiers[numRectifiers++];
        rect->info.id       = SagRXMSG::sagMsgSrcAddr(msg);
		rect->info.condition  = 1;
		rect->info.ratedCurrent	= 50; //default 3000W

        rect->spec.id       = rect->info.id;
		
        log("Sag6400Mgr: new PSU registered, total = %u\n", numRectifiers);
    }
}

void* Sag6400Mgr::rectifierBySerial(uint64_t serial)
{
	serial = serial; //just clear compile warning
	return nullptr;
}

void* Sag6400Mgr::rectifierById(uint8_t  id)
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

void* Sag6400Mgr::rectifierByIndex(uint8_t  index)
{
	return rectifiers[index];
}

void Sag6400Mgr::sendSetDefaultOutputRequest(void)
{
	Sag6400GenMessage msg = Sag6400GenMessage(0x163, 0xf0, 0x00);

	msg.SagSetData();

	if (debug)
	{
		log("Sag6400: send first defaut output Request\n");
	}
	
	Can::send(msg.SagCanTx());
	Can::send2(msg.SagCanTx());
}

void Sag6400Mgr::sendSetVoltageOutputRequest(uint16_t voltage)
{
	Sag6400GenMessage msg = Sag6400GenMessage(0x163, 0xf0, 0x00);;

	msg.SagSetData(0, voltage);

	if (debug)
	{
		log("Sag6400: send voltage output Request\n");
	}
	
	Can::send(msg.SagCanTx());
	Can::send2(msg.SagCanTx());
}

void Sag6400Mgr::setSagEnabled(void)
{
	if (debug)
	{
		log("Sag6400: setSagEnabled slaveId = %d\n",g_masterGate.masterData.slaveId);
	}
	
	enabled = (g_masterGate.masterData.slaveId == 0);
}

bool Sag6400Mgr::getSagEnabled(void)
{
	return enabled;
}

bool Sag6400Mgr::checkPowerCond(HBWorkCond*		pHBWorkCond)
{
	uint32_t errCode = 0;
	if (numRectifiers < MAX_PSU_PER_SLAVE)
	{
		errCode = MS_SET_HW_ERR_CODE(ERR_PWR, ERR_TYPE_RET_LESS, 0);
		g_slaveError.saveErrorCode(errCode);	
	}
	
	for (uint32_t num = 0; num < numRectifiers; num++)
	{
		double PSU_Pout = ((SagRectifier*)rectifiers[num])->info.getPOut();
		double PSU_Vout = ((SagRectifier*)rectifiers[num])->info.getVOut();
		double PSU_Iout = ((SagRectifier*)rectifiers[num])->info.getIOut();
		int8_t PSU_tempOut = ((SagRectifier*)rectifiers[num])->info.tempOut;

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

