#include <algorithm>
#include <stdio.h>

#include "utils.h"
#include "huawei_r48xx_mgr.h"
#include "power_mgr.h"
#include "huawei_r48xx_msg.h"
#include "stm32f4xx_can.h"
#include "can.h"
#include "mytime.h"
#include "master_gate.h"
#include "data_feild.h"
#include "data_mgr.h"
#include "board_mgr.h"
#include "ms_error.h"


#if defined(POWER_SUPPLY_USE_HUAWEI_R48XX)
huaWeiR48xxMgr g_huaWeiR48xxMgr;

huaWeiR48xxMgr::huaWeiR48xxMgr()
	:numRectifiers(0),
	enabled(true),
	measuredVoltage(0),
	debug(false)
{

}

void huaWeiR48xxMgr::init()
{
	Can::init();
	rectifiers[0] = new huaWeiR48xxRectifier();
	rectifiers[1] = new huaWeiR48xxRectifier();
	size[0] = 0;
	size[1] = 0;
	g_RtdataMgr.init();
}

void huaWeiR48xxMgr::process100ms()
{
	if (!enabled) return;

	log("huaWeiR48xxMgr process100ms: %d", Can::rxMsgSize());
	for (int i = 0; i < 20; i++)   
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

	uint32_t infoNow = getMiliSeconds();
	static uint32_t infoTime = 0;
	if(numRectifiers < MAX_PSU_PER_SLAVE)
	{
		if(infoNow - infoTime > 5*1000)
		{
			infoTime = infoNow;
			sendGetInfoReq();
		}

		return;
	}
	else
	{
		if(infoNow - infoTime > 30*1000)
		{
			infoTime = infoNow;
			sendGetInfoReq();
		}
	}
	
	uint32_t descNow = getMiliSeconds();
	static uint32_t descTime = 0;
	static uint8_t desc_index = 0;
	if(numRectifiers < MAX_PSU_PER_SLAVE)
	{

		if(descNow - descTime > 5*1000)
		{
			descTime = descNow;
			if (numRectifiers > 0)
			{
				sendGetDescReq(getRectifierInforByIndex(numRectifiers - 1)->id);
			}
		}

		return;
	}
	else
	{
		if(descNow - descTime > 7*1000)
		{
			descTime = descNow;
			sendGetDescReq(getRectifierInforByIndex(desc_index++)->id);
			if (desc_index >= numRectifiers)
			{
				desc_index = 0;
			}
		}
	}

	g_powerMgr.step();
	uint16_t voltage = g_powerMgr.calcSetVoltage();
    uint32_t now = getMiliSeconds();

	static uint32_t statusCfgTime = 0;
    if (now - statusCfgTime > 10*1000)
    {
        statusCfgTime = now;

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
            sendSetOutputVoltage(voltage);
			sendGetDataReq();
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
            sendRectifierReq();
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

PsuSpec* huaWeiR48xxMgr::getRectifierSpecById(uint8_t id)
{
	huaWeiR48xxRectifier* rect = (huaWeiR48xxRectifier*)rectifierById(id);
	return &(rect->spec);
}

PsuSpec* huaWeiR48xxMgr::getRectifierSpecByIndex(uint8_t index)
{
	huaWeiR48xxRectifier* rect = (huaWeiR48xxRectifier*)rectifierByIndex(index);
	return &(rect->spec);
}

PsuInfo* huaWeiR48xxMgr::getRectifierInforById(uint8_t id)
{
	huaWeiR48xxRectifier* rect = (huaWeiR48xxRectifier*)rectifierById(id);
	return &(rect->info);
}

PsuInfo* huaWeiR48xxMgr::getRectifierInforByIndex(uint8_t index)
{
	huaWeiR48xxRectifier* rect = (huaWeiR48xxRectifier*)rectifierByIndex(index);
	return &(rect->info);
}

uint8_t huaWeiR48xxMgr::getNumRectifiers()
{
	return numRectifiers;
}

uint16_t huaWeiR48xxMgr::getRectMeasuredCurrentbyId(uint8_t id)
{
	huaWeiR48xxRectifier* rect = (huaWeiR48xxRectifier*)rectifierById(id);
	return rect->info.measuredCurrent;
}

uint16_t huaWeiR48xxMgr::getRectMeasuredVoltagebyId(uint8_t id)
{
	huaWeiR48xxRectifier* rect = (huaWeiR48xxRectifier*)rectifierById(id);
	return rect->info.measuredVoltage;
}

bool huaWeiR48xxMgr::checkPowerCond(HBWorkCond*		pHBWorkCond)
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

		if(PSU_Pout <= pHBWorkCond->PsuPoutLowTh)
		{
			errCode = MS_SET_HW_ERR_CODE(ERR_PWR, ERR_TYPE_PSU_P_LOW, num);
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
		if(getRectifierInforByIndex(num)->ratedCurrent > 55)
		{
			if (PSU_Pout > pHBWorkCond->PsuPoutHighTh + 600)
			{
				errCode = MS_SET_HW_ERR_CODE(ERR_PWR, ERR_TYPE_PSU_P_HIGH, num);
				g_slaveError.saveErrorCode(errCode);
			}
			if (PSU_Vout > pHBWorkCond->PsuVoutHighTh + 2.5)
			{
				errCode = MS_SET_HW_ERR_CODE(ERR_PWR, ERR_TYPE_PSU_U_HIGH, num);
				g_slaveError.saveErrorCode(errCode);
			}
		}
		else
		{
			if (PSU_Pout > pHBWorkCond->PsuPoutHighTh)
			{
				errCode = MS_SET_HW_ERR_CODE(ERR_PWR, ERR_TYPE_PSU_P_HIGH, num);
				g_slaveError.saveErrorCode(errCode);
			}
			if (PSU_Vout > pHBWorkCond->PsuVoutHighTh)
			{
				errCode = MS_SET_HW_ERR_CODE(ERR_PWR, ERR_TYPE_PSU_U_HIGH, num);
				g_slaveError.saveErrorCode(errCode);
			}
		}
	}
	
	if (errCode != 0)
		return false;
	else
		return true;
}

void huaWeiR48xxMgr::processRxMsg(const CanRxMsg &msg)
{
	uint32_t	msgId = huaWeiR48xxRxMsg::r48xxMsgCmdId(msg);

	if(debug)
	{
		log("processRxMsg StdId = 0x%x, ExtId = 0x%x. msg data:\n",msg.StdId,msg.ExtId);
		for(uint8_t i = 0; i < 8; i++)
		{
			log("huawei R48xx data[%d] = 0x%x\n",i,msg.Data[i]);
		}
	}
	
	if(msgId == HUAWEI_R48XX_MSG_DATA_ID)
	{
		processRtDataMsg(&msg);
	}
	else if(msgId == HUAWEI_R48XX_MSG_CONTROL_ID)
	{
		//set single module
	}
	else if(msgId == HUAWEI_R48XX_MSG_CONFIG_ID)
	{
	}
	else if (msgId == HUAWEI_R48XX_MSG_QUERY_ID)
	{
		
	}
	else if (msgId == HUAWEI_R48XX_MSG_DESC_ID)
	{
		processDescMsg(&msg);
	}
	else if(msgId == HUAWEI_R48XX_MSG_INFO_ID)
	{
		if(numRectifiers < MAX_PSU_PER_SLAVE)
		{
			processLogonMsg(&msg);
		}
		
		processModuleInfoMsg(&msg);
	}
}

void* huaWeiR48xxMgr::rectifierBySerial(uint64_t serial)
{
#if 0
    for (uint32_t i = 0; i < numRectifiers; i++)
    {
        if (rectifiers[i].info.serial == serial)
        {
            return &rectifiers[i];
        }
    }
#endif
	serial = serial; //just clear compile warning
    return nullptr;
}

void* huaWeiR48xxMgr::rectifierById(uint8_t     id)
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

void* huaWeiR48xxMgr::rectifierByIndex(uint8_t	index)
{
	return rectifiers[index];
}

uint16_t huaWeiR48xxMgr::transferStrToInt(char *str)
{
	int i;
	int len = 0;
	
	uint16_t value = 0;
	if (NULL == str)
	{
		return 0;
	}

	len = strlen(str);
	for(i = 0; i < len; i++)
	{
		if((str[i] >= '0') && (str[i] <= '9'))
		{
			value = value * 10 + str[i] - '0';
		}	
	}

	return value;
}

void huaWeiR48xxMgr::transferDate(huaWeiR48xxRectifier *rect, char *date)
{
	char *start = NULL;
	char *p = NULL;

	if ((NULL == rect) || (NULL == date))
	{
		return ;
	}

	start = date;
	p = strchr(start, '-');
	if (p)
	{
		*p = '\0';
		rect->spec.prodYear = transferStrToInt(start);
		start = p + 1;
		p = NULL;
	}
	else
	{
		return ;
	}

	p = strchr(start, '-');
	if (p)
	{
		*p = '\0';
		rect->spec.prodMonth = transferStrToInt(start);
		start = p + 1;
		p = NULL;
	}
	else
	{
		return ;
	}
	
	rect->spec.prodDay = transferStrToInt(start);
}

void huaWeiR48xxMgr::processDescMsg(const CanRxMsg *msg)
{
	uint32_t addr = huaWeiR48xxRxMsg::r48xxMsgAddress(*msg);
	
	huaWeiR48xxRectifier* rect = (huaWeiR48xxRectifier*)rectifierById((uint8_t)addr);
    if (rect)
    {
    	if (debug)
    	{
			log("%x, data:%x, %x, %x, %x, %x, %x, %x, %x, addr: %x, size: %x\n", 
								msg->ExtId,
								msg->Data[0], 
								msg->Data[1],
								msg->Data[2],
								msg->Data[3],
								msg->Data[4],
								msg->Data[5],
								msg->Data[6],
								msg->Data[7],
								addr, size[addr -1]);
    	}

		if (size[addr - 1] + 6 >= 512)
		{
			log("processDescMsg: size:%x", size[addr - 1]);
			return ;
		}
		
		memcpy(&(data[addr - 1][size[addr - 1]]), &(msg->Data[2]), 6);
		size[addr -1] += 6;
		if (!huaWeiR48xxRxMsg::r48xxMsgCount(*msg))
		{
		/*
		 * 接收华为电源模块上报的模块信息，r48xxMsgCount返回0表示数据接收完毕，开始解析；
		 * 先取一行，然后查找关键字，符合关键字，再相应取出信息 
		 * 原始的数据如下：
		 * 	/$[ArchivesInfo Version]
		 *	/$ArchivesInfoVersion=3.0
		 *	[Board Properties]
		 *	BoardType=EN1MRC5G1A1
		 *	BarCode=2102310FFABTG4002898
		 *	Item=02310FFA
		 *	Description=Function Module,R4850G2,EN1MRC5G1A1,1U 3000W Rectifier,Power Only
		 *	Manufactured=2016-04-11
		 *	VendorName=Huawei
		 *	IssueNumber=00
		 *	CLEICode=
		 *	BOM=
		 * 过滤后获取的数据就是
		 * Description BoardType Manufactured BarCode
		 * 的相应数据
		 *
		 */
			char *start = NULL;
			char *p = NULL;
			data[addr - 1][size[addr - 1]] = '\0';
			size[addr - 1] = 0;

			if (debug)
			{
				log("processDescMsg: %s\n", data[addr - 1]);
			}
			
			for (start = data[addr - 1]; (p = strchr(start, '\n')); start = p + 1)
			{
				char *tmp = NULL;
				
				*p = '\0';
				tmp = strchr(start, '=');
				if (tmp)
				{
					*tmp = '\0';
					if (strncmp(start, "Description", strlen("Description")) == 0)
					{
						tmp++;
						strncpy(rect->spec.prodDesc, tmp, 26);
						rect->spec.prodDesc[26] = '\0';

						if (debug)
						{
							log("processDescMsg: Description: %s\n", tmp);
						}
						
					}
					if (strncmp(start, "BoardType", strlen("BoardType")) == 0)
					{
						tmp++;
						strncpy(rect->spec.prodPart, tmp, 11);
						rect->spec.prodPart[11] = '\0';

						if (debug)
						{
							log("processDescMsg: BoardType: %s\n", tmp);
						}
						
					}
					if (strncmp(start, "Manufactured", strlen("Manufactured")) == 0)
					{
						tmp++;
						transferDate(rect, tmp);

						if (debug)
						{
							log("processDescMsg: Manufactured: %s\n", tmp);
						}
						
					}
					if (strncmp(start, "BarCode", strlen("BarCode")) == 0)
					{
						tmp++;
						strncpy(rect->spec.serial, tmp, 47);
						rect->spec.serial[47] = '\0';
						strncpy(rect->info.serial, tmp, 47);
						rect->info.serial[47] = '\0';

						if (debug)
						{
							log("processDescMsg: barcode: %s\n", tmp);
						}
						
					}
				}
			}

		}

   }
}

void huaWeiR48xxMgr::processLogonMsg(const CanRxMsg *msg)
{
	uint32_t addr = huaWeiR48xxRxMsg::r48xxMsgAddress(*msg);

	huaWeiR48xxRectifier* rect = (huaWeiR48xxRectifier*)rectifierById((uint8_t)addr);
    if (!rect)
    {
        if (numRectifiers >= MAX_PSU_PER_SLAVE)
        {
            log("huaWeiR48xxMgr: can not accept LOGON (limit reached)\n");
            return;
        }

        rect = (huaWeiR48xxRectifier*)rectifiers[numRectifiers++];
        rect->info.id       = addr;
        //rect->info.serial   = lgm.serial();
        rect->info.condition = 1;

        rect->spec.id       = rect->info.id;
//        rect->spec.serial   = rect->info.serial;
    }
	
}

void huaWeiR48xxMgr::processRtDataMsg(const CanRxMsg * msg)
{
	huaWeiR48xxRtDataMsg rtDataMsg;

	rtDataMsg.rtMsgParse(*msg);
	rtDataMsg.rtDataParse();
	
}

void huaWeiR48xxMgr::processModuleInfoMsg(const CanRxMsg * msg)
{
	huaWeiR48xxRtDataMsg rtDataMsg;
	
	rtDataMsg.rtMsgParse(*msg);
	rtDataMsg.rtDataParse();
	
}

void huaWeiR48xxMgr::sendDefaultVoltageReq()
{

}

void huaWeiR48xxMgr::sendDefaultVoltage()
{

}

void huaWeiR48xxMgr::sendSetOutputVoltage(uint16_t vol)
{
	huaWeiR48xxGenMsg msg = huaWeiR48xxGenMsg(HUAWEI_R48XX_PROTOCOL_ID, 0x00, HUAWEI_R48XX_MSG_CONTROL_ID);
	uint32_t data0 = 0x00,data1 = 0x00;

	msg.huaWeiR48xxSetSrc(0x01);
	msg.huaWeiR48xxSetCnt(0x00);
	msg.huaWeiR48xxSetRev(0x3F);

	data0 = 0x01000000;
	data1 = vol;
	data1 = data1*1024/100;

	data0 = SwapEndian(data0);
	data1 = SwapEndian(data1);

	msg.huaWeiR48xxSetData(data0, data1);

	CanTxMsg txMsg = msg.huaWeiR48xxCanTx();

	if(debug)
	{
		for(uint8_t i = 0; i < 8; i++)
		{
			log("huawei byte[%d] = 0x%x\n", i, txMsg.Data[i]);
		}
	}

	Can::send(txMsg);
}

void huaWeiR48xxMgr::sendSetDefaultOutputVoltage()
{

}

void huaWeiR48xxMgr::sendGetDataReq()
{
	huaWeiR48xxGenMsg msg = huaWeiR48xxGenMsg(HUAWEI_R48XX_PROTOCOL_ID, 0x00, HUAWEI_R48XX_MSG_DATA_ID);
	
	msg.huaWeiR48xxSetSrc(0x01);
	msg.huaWeiR48xxSetCnt(0x00);

	msg.huaWeiR48xxSetData(0x00, 0x00);
	msg.huaWeiR48xxSetRev(0x3F);

	Can::send(msg.huaWeiR48xxCanTx());
}

void huaWeiR48xxMgr::sendGetInfoReq()
{
	huaWeiR48xxGenMsg msg = huaWeiR48xxGenMsg(HUAWEI_R48XX_PROTOCOL_ID, 0x00, HUAWEI_R48XX_MSG_INFO_ID);

	msg.huaWeiR48xxSetSrc(0x01);
	msg.huaWeiR48xxSetCnt(0x00);

	msg.huaWeiR48xxSetData(0x00, 0x00);
	msg.huaWeiR48xxSetRev(0x3F);
	//msg.huaWeiR48xxCanTx();

	Can::send(msg.huaWeiR48xxCanTx());
}

void huaWeiR48xxMgr::sendGetDescReq(uint32_t addr)
{
	huaWeiR48xxGenMsg msg = huaWeiR48xxGenMsg(HUAWEI_R48XX_PROTOCOL_ID, addr, HUAWEI_R48XX_MSG_DESC_ID);

	if (addr < MAX_PSU_PER_SLAVE)
	{
		size[addr - 1] = 0;
	}

	msg.huaWeiR48xxSetSrc(0x01);
	msg.huaWeiR48xxSetCnt(0x00);

	msg.huaWeiR48xxSetData(0x00, 0x00);
	msg.huaWeiR48xxSetRev(0x3F);

	Can::send(msg.huaWeiR48xxCanTx());
}


void huaWeiR48xxMgr::sendTurnOnOff(uint8_t id, bool on)
{
	id = id;
	on = on;
}

void huaWeiR48xxMgr::sendRectifierReq()
{

}

void huaWeiR48xxMgr::setHuaWeiR48xxEnabled(void)
{
	if(debug) log("huaWeiR48xxMgr: setSagEnabled slaveId = %d\n",g_masterGate.masterData.slaveId);
	enabled = (g_masterGate.masterData.slaveId == 0);
}
#endif
