#ifndef SAG_MSG_H
#define SAG_MSG_H

#include <array>
#include <cstdint>

#include "sag6400_rectifier.h"
#include "stm32f4xx_can.h"

#define  SAG6400_DATA_LEN				8

#define  SAG6400_CMD0    				0      	/* send to SAG */
#define  SAG6400_VA_CMD1    			1		/* read Voltage and Current from sag */
#define  SAG6400_INFO_CMD2    			2		/* read info from sag */
#define  SAG6400_TEMP_CMD3    			3     	/* read temp from sag */
#define  SAG6400_PRODUCT_MODEL_CMD10	0x10	/* read product model data high part */
#define  SAG6400_PRODUCT_MODEL_CMD11	0x11	/* read product model data low part */
#define  SAG6400_PART_CMD12				0x12	/* read part data high part */
#define  SAG6400_PART_CMD13				0x13	/* read part data low part */
#define  SAG6400_VERSION_CMD14			0x14	/* read sag6400 psu version */
#define  SAG6400_SERIAL_NUMBER_CMD15	0x15	/* read sag6400 serial number data high part */
#define  SAG6400_SERIAL_NUMBER_CMD16	0x16	/* read sag6400 serial number data low part */
#define  SAG6400_PRODUCT_DATE_CMD17		0x17	/* read sag6400 product date data high part */
#define  SAG6400_PRODUCT_DATE_CMD18		0x18	/* read sag6400 product date data low part */

class SagRXMSG
{
public:
    static uint16_t sagMsgProfileID(const CanRxMsg& msg);
    static uint8_t sagMsgSrcAddr(const CanRxMsg& msg);
    static uint8_t sagMsgDesAddr    (const CanRxMsg& msg);
};

class SagStatusMSG
{
public:
	SagStatusMSG(CanRxMsg & msg);

	uint16_t GetSagCurrent(void);
	uint16_t GetSagVoltage(void);
	uint16_t GetSagStatus(void);
	uint16_t GetSagInVoltage(void);

	int8_t GetSagEnvTemp(void);
	int8_t GetSagModleTemp(void);

	int GetSagPart(SagRectifier *rect);
	int GetSagDesc(SagRectifier *rect);
	int GetSagSerialNumber(SagRectifier *rect);
	int GetSagDate(SagRectifier *rect);
	uint16_t transferStrToInt(char *str);
	int GetSagVersion(char *dest, int len);

private:
	uint16_t sag_prof_id;
	uint8_t sag_src_addr;
	uint8_t sag_des_addr;
	uint8_t pad;
	uint8_t sag_cmd_id;
	uint16_t sag_measured_current;
    uint16_t sag_measured_voltage;
	uint16_t sag_measured_in_voltage;
	uint16_t sag_status;

	int8_t sag_env_temp;
	int8_t sag_modle_temp;
    bool    debug;

/*
	char   sag_version[SAG6400_DATA_LEN];
	
	char   sag_product_date_h[SAG6400_DATA_LEN];
	char   sag_product_date_l[SAG6400_DATA_LEN];

	char   sag_serail_number_h[SAG6400_DATA_LEN];
	char   sag_serail_number_l[SAG6400_DATA_LEN];

	char   sag_part_h[SAG6400_DATA_LEN];
	char   sag_part_l[SAG6400_DATA_LEN];

	char   sag_product_model_h[SAG6400_DATA_LEN];
	char   sag_product_model_l[SAG6400_DATA_LEN];
*/

};


class Sag6400GenMessage
{
public:
	Sag6400GenMessage();
	Sag6400GenMessage(uint16_t profileID, uint8_t srcAddr, uint8_t desAddr);

	void SagSetData(void);
	void SagSetData(uint16_t current, uint16_t voltage);

	CanTxMsg SagCanTx(void);

	static const uint8_t SAG_MSG_MAX_DATA_SIZE      = 10;
private:
	uint16_t sag_prof_id;
	uint8_t sag_src_addr;
	uint8_t sag_des_addr;
	uint8_t data[SAG_MSG_MAX_DATA_SIZE];
};
#endif

