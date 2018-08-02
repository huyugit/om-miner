#ifndef HUAWEI_R48XX_MSG_H
#define HUAWEI_R48XX_MSG_H

#include <array>
#include <cstdint>

#include "stm32f4xx_can.h"
#include "data_feild.h"

#define	HUAWEI_R48XX_PROTOCOL_ID		0x21

#define	HUAWEI_R48XX_MSG_CONTROL_ID		0x80
#define	HUAWEI_R48XX_MSG_CONFIG_ID		0x81
#define	HUAWEI_R48XX_MSG_QUERY_ID		0x82
#define	HUAWEI_R48XX_MSG_DATA_ID		0x40
#define	HUAWEI_R48XX_MSG_INFO_ID		0x50
#define HUAWEI_R48XX_MSG_DESC_ID		0xD2

enum errorType
{
	TYPE_OK,
	TYPE_PARAMETER_ERROR,
	TYPE_CMD_ERROR,
	TYPE_IDENTIFYING_ADDR,
	TYPE_RFID_NOT_WRITE,
	TYPE_RFID_READ_ERROR,
	TYPE_RECTIFY_LOAD_INTERRUPT,
	TYPE_RECTIFY_AUTO_VOL,
	TYPE_ADDRESS_CLASH
};

class huaWeiR48xxRxMsg
{
public:
	static uint32_t r48xxMsgProtocolId(const CanRxMsg & msg);
	static uint32_t r48xxMsgAddress(const CanRxMsg & msg);
	static uint32_t r48xxMsgCmdId(const CanRxMsg & msg);
	static uint32_t r48xxMsgFromSource(const CanRxMsg & msg);
	static uint32_t r48xxMsgCount(const CanRxMsg & msg);
	static uint32_t r48xxMsgDataLow4Bytes(const CanRxMsg & msg);
	static uint32_t r48xxMsgDataHigh4Bytes(const CanRxMsg & msg);
};

class huaWeiR48xxRtDataMsg
{
public:
	huaWeiR48xxRtDataMsg();

	void rtMsgParse(const CanRxMsg &msg);
	void rtDataParse(void);

private:
	//msg
	uint32_t	msgProtoId;
	uint32_t	msgAddr;
	uint32_t	msgCmdId;
	uint32_t	msgFromSrc;
	uint32_t	msgRev;
	uint32_t	msgCnt;
	uint32_t	msgData0;
	uint32_t	msgData1;

	//data
	uint16_t	dataSigId;
};

class huaWeiR48xxGenMsg
{
public:
	huaWeiR48xxGenMsg();
	huaWeiR48xxGenMsg(uint32_t protoId, uint32_t addr, uint32_t cmdId);

	void huaWeiR48xxSetData(void);
	void huaWeiR48xxSetData(uint32_t data0, uint32_t data1);
	CanTxMsg huaWeiR48xxCanTx(void);

	void huaWeiR48xxSetSrc(uint32_t src);
	void huaWeiR48xxSetCnt(uint32_t cnt);
	void huaWeiR48xxSetRev(uint32_t rev);

	//static const uint8_t MSG_MAX_DATA_SIZE      = 10;
private:
	uint32_t	msgProtoId;
	uint32_t	msgAddr;
	uint32_t	msgCmdId;
	uint32_t	msgFromSrc;
	uint32_t	msgRev;
	uint32_t	msgCnt;
	
	//uint8_t 	msgData[MSG_MAX_DATA_SIZE];
	uint32_t	msgData0;
	uint32_t	msgData1;
};
#endif
