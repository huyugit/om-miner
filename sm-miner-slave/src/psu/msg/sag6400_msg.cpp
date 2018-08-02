#include <stdio.h>
#include <cstring>
#include "sag6400_msg.h"
#include "format.hpp"

static char   sag_version[SAG6400_DATA_LEN] = { 0 };

static char   sag_product_date_h[SAG6400_DATA_LEN] = { 0 };
static char   sag_product_date_l[SAG6400_DATA_LEN] = { 0 };

static char   sag_serail_number_h[SAG6400_DATA_LEN] = { 0 };
static char   sag_serail_number_l[SAG6400_DATA_LEN] = { 0 };

static char   sag_part_h[SAG6400_DATA_LEN] = { 0 };
static char   sag_part_l[SAG6400_DATA_LEN] = { 0 };

static char   sag_product_model_h[SAG6400_DATA_LEN] = { 0 };
static char   sag_product_model_l[SAG6400_DATA_LEN] = { 0 };


uint16_t SagRXMSG::sagMsgProfileID(const CanRxMsg & msg)
{
	uint16_t profileID;

	profileID = (msg.ExtId>>16)&0x0000FFFF;
	return profileID;
}

uint8_t SagRXMSG::sagMsgSrcAddr(const CanRxMsg& msg)
{
	uint8_t srcAddr;

	srcAddr = msg.ExtId & 0xFF;
	return srcAddr;
}

uint8_t SagRXMSG::sagMsgDesAddr	(const CanRxMsg& msg)
{
	uint8_t desAddr;

	desAddr = (msg.ExtId>>8)&0x00FF;
	return desAddr;
}

SagStatusMSG::SagStatusMSG(CanRxMsg & msg)
    :debug(false)
{
	sag_prof_id = SagRXMSG::sagMsgProfileID(msg);
	sag_src_addr = SagRXMSG::sagMsgSrcAddr(msg);
	sag_des_addr = SagRXMSG::sagMsgDesAddr(msg);
	
	sag_cmd_id = msg.Data[0];

	if (debug)
	{
		log("SagStatusMSG:%s,%s,%s,%s,%s,%s,%s\n", 
				sag_version, 
				sag_product_date_h, sag_product_date_l, 
				sag_product_model_h, sag_product_model_l,
				sag_serail_number_h,
				sag_serail_number_l);
	}

	if (SAG6400_VA_CMD1 == sag_cmd_id)
	{
	    ((uint8_t*)(&sag_measured_current))[0]       = msg.Data[3];
	    ((uint8_t*)(&sag_measured_current))[1]       = msg.Data[2];
	    ((uint8_t*)(&sag_measured_voltage))[0]       = msg.Data[5];
	    ((uint8_t*)(&sag_measured_voltage))[1]       = msg.Data[4];
	}

	if (SAG6400_INFO_CMD2 == sag_cmd_id)
	{
		((uint8_t*)(&sag_measured_in_voltage))[0] = msg.Data[3];
		((uint8_t*)(&sag_measured_in_voltage))[1] = msg.Data[2];
		sag_measured_in_voltage = sag_measured_in_voltage / 100;
	}

	if (SAG6400_TEMP_CMD3 == sag_cmd_id)
	{
		int16_t temp;

	    ((uint8_t*)(&temp))[0]       = msg.Data[3];
	    ((uint8_t*)(&temp))[1]       = msg.Data[2];
		sag_env_temp = temp/100;
			
	    ((uint8_t*)(&temp))[0]       = msg.Data[5];
	    ((uint8_t*)(&temp))[1]       = msg.Data[4];
		sag_modle_temp = temp/100;
	}

	if (SAG6400_PRODUCT_MODEL_CMD10 == sag_cmd_id)
	{
		strncpy(sag_product_model_h, (char *)(&(msg.Data[2])), SAG6400_DATA_LEN);
	}
	
	if (SAG6400_PRODUCT_MODEL_CMD11 == sag_cmd_id)
	{
		strncpy(sag_product_model_l, (char *)(&(msg.Data[2])), SAG6400_DATA_LEN);
	}
	
	if (SAG6400_PART_CMD12 == sag_cmd_id)
	{
		strncpy(sag_part_h, (char *)(&(msg.Data[2])), SAG6400_DATA_LEN);
	}
	
	if (SAG6400_PART_CMD13 == sag_cmd_id)
	{
		strncpy(sag_part_l, (char *)(&(msg.Data[2])), SAG6400_DATA_LEN);
	}
	
	if (SAG6400_VERSION_CMD14 == sag_cmd_id)
	{
		strncpy(sag_version, (char *)(&(msg.Data[2])), SAG6400_DATA_LEN);
	}
	
	if (SAG6400_SERIAL_NUMBER_CMD15 == sag_cmd_id)
	{
		strncpy(sag_serail_number_h, (char *)(&(msg.Data[2])), SAG6400_DATA_LEN);
	}
	
	if (SAG6400_SERIAL_NUMBER_CMD16 == sag_cmd_id)
	{
		strncpy(sag_serail_number_l, (char *)(&(msg.Data[2])), SAG6400_DATA_LEN);
	}
	
	if (SAG6400_PRODUCT_DATE_CMD17 == sag_cmd_id)
	{
		strncpy(sag_product_date_h, (char *)(&(msg.Data[2])), SAG6400_DATA_LEN);
	}
	
	if (SAG6400_PRODUCT_DATE_CMD18 == sag_cmd_id)
	{
		strncpy(sag_product_date_l, (char *)(&(msg.Data[2])), SAG6400_DATA_LEN);
	}


	((uint8_t*)(&sag_status))[0]       = msg.Data[7];
    ((uint8_t*)(&sag_status))[1]       = msg.Data[6];
}

uint16_t SagStatusMSG::GetSagCurrent(void)
{
	return sag_measured_current;
}

uint16_t SagStatusMSG::GetSagVoltage(void)
{
	return sag_measured_voltage;
}

uint16_t SagStatusMSG::GetSagInVoltage(void)
{
	return sag_measured_in_voltage;
}

uint16_t SagStatusMSG::GetSagStatus(void)
{
	return sag_status;
}

int8_t SagStatusMSG::GetSagEnvTemp(void)
{
	return sag_env_temp;
}

int8_t SagStatusMSG::GetSagModleTemp(void)
{
	return sag_modle_temp;
}

int SagStatusMSG::GetSagVersion(char *dest, int len)
{
	if ((NULL == dest) || (len <= 0))
	{
		return -1;
	}

	strncpy(dest, sag_version, len);

	if (debug)
	{
		log("SagStatusMSG: version %s", dest);
	}

	return 0;
}


uint16_t SagStatusMSG::transferStrToInt(char *str)
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


int SagStatusMSG::GetSagDate(SagRectifier *rect)
{
    char *start = NULL;
    char *p = NULL;
	char date[SAG6400_DATA_LEN * 2] = { 0 };

    if ((NULL == rect) \
        || (strlen(sag_product_date_h) <= 0) \
        || (strlen(sag_product_date_l) <= 0))
    {
        return -1;
    }

	strncpy(date, sag_product_date_h, SAG6400_DATA_LEN);
	strcat(date, sag_product_date_l);

	if (debug)
	{
		log("SagStatusMSG: date : %s", date);
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
        return -2;
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
        return -3;
    }

    rect->spec.prodDay = transferStrToInt(start);
	return 0;
}

int SagStatusMSG::GetSagSerialNumber(SagRectifier *rect)
{
	char serial_number[SAG6400_DATA_LEN * 2] = { 0 };

    if ((NULL == rect) \
        || (strlen(sag_serail_number_h) <= 0) \
        || (strlen(sag_serail_number_l) <= 0))
    {
        return -1;
    }

	strncpy(serial_number, sag_serail_number_h, SAG6400_DATA_LEN);
	strcat(serial_number, sag_serail_number_l);

	strncpy(rect->spec.serial, serial_number, 48);
	strncpy(rect->info.serial, serial_number, 48);

	if (debug)
	{
		log("SagStatusMSG: serial %s,%s\n", rect->spec.serial, rect->info.serial);
	}

	return 0;
}

int SagStatusMSG::GetSagPart(SagRectifier *rect)
{
	char part[SAG6400_DATA_LEN * 2] = { 0 };

    if ((NULL == rect) \
        || (strlen(sag_part_h) <= 0) \
        || (strlen(sag_part_l) <= 0))
    {
        return -1;
    }

	strncpy(part, sag_part_h, SAG6400_DATA_LEN);
	strcat(part, sag_part_l);

	strncpy(rect->spec.prodPart, part, 12);

	if (debug)
	{
		log("SagStatusMSG: part %s", part);
	}

	return 0;
}

int SagStatusMSG::GetSagDesc(SagRectifier *rect)
{
	char desc[SAG6400_DATA_LEN * 2] = { 0 };

    if ((NULL == rect) \
        || (strlen(sag_product_model_h) <= 0) \
        || (strlen(sag_product_model_l) <= 0))
    {
        return -1;
    }

	strncpy(desc, sag_product_model_h, SAG6400_DATA_LEN);
	strcat(desc, sag_product_model_l);

	strncpy(rect->spec.prodDesc, desc, 27);

	if (debug)
	{
		log("SagStatusMSG: desc %s", desc);
	}

	return 0;
}



Sag6400GenMessage::Sag6400GenMessage()
	:sag_prof_id(0x163),
	sag_src_addr(0xf0),
	sag_des_addr(0)
{

}

Sag6400GenMessage::Sag6400GenMessage(
    uint16_t profileID, 
    uint8_t srcAddr, 
    uint8_t desAddr)
{
	sag_prof_id = profileID;
	sag_src_addr = srcAddr;
	sag_des_addr = desAddr;
}

CanTxMsg Sag6400GenMessage::SagCanTx(void)
{
	CanTxMsg msg;

	msg.IDE = CAN_Id_Extended;
    msg.RTR = CAN_RTR_Data;
	
	msg.ExtId = 0;
	msg.ExtId = ((sag_prof_id<<16)&0xFFFF0000) \
                | ((sag_des_addr<<8)&0xff00) \
                | (sag_src_addr);

	log("SagCanTx ExtId = %x\n", msg.ExtId);
	msg.DLC = 8;
	::memcpy(msg.Data, data, msg.DLC);

	return msg;
}

void Sag6400GenMessage::SagSetData(void)
{
	SagSetData(0, 4400); //default 0A, 44V
}
void Sag6400GenMessage::SagSetData(uint16_t current, uint16_t voltage)
{
	data[0] = SAG6400_CMD0;
	data[1] = 0xf0;

	data[2] = ((uint8_t*)(&current))[1];
	data[3] = ((uint8_t*)(&current))[0];

	data[4] = ((uint8_t*)(&voltage))[1];
	data[5] = ((uint8_t*)(&voltage))[0];

	log("SagSetData  data:%x,data:%x,data:%x,data:%x,data:%x,data:%x\n",
        data[0],data[1],data[2],data[3],data[4],data[5]);
}


