#include <cstring>
#include "huawei_r48xx_msg.h"
#include "format.hpp"
#include "data_mgr.h"

#if defined(POWER_SUPPLY_USE_HUAWEI_R48XX)
uint32_t huaWeiR48xxRxMsg::r48xxMsgProtocolId(const CanRxMsg & msg)
{
	uint32_t protocolId;

	protocolId = (msg.ExtId>>23)&0x0000003F;
	return protocolId;
}

uint32_t huaWeiR48xxRxMsg::r48xxMsgAddress(const CanRxMsg & msg)
{
	uint32_t addr;

	addr = (msg.ExtId>>16)&0x0000007F;
	return addr;
}

uint32_t huaWeiR48xxRxMsg::r48xxMsgCmdId(const CanRxMsg & msg)
{
	uint32_t cmdId;

	cmdId = (msg.ExtId>>8)&0x000000FF;
	return cmdId;
}

uint32_t huaWeiR48xxRxMsg::r48xxMsgFromSource(const CanRxMsg & msg)
{
	uint32_t fromSource;

	fromSource = (msg.ExtId>>7)&0x00000001;
	return fromSource;
}

uint32_t huaWeiR48xxRxMsg::r48xxMsgCount(const CanRxMsg & msg)
{
	uint32_t count;

	count = (msg.ExtId)&0x00000001;
	return count;
}

uint32_t huaWeiR48xxRxMsg::r48xxMsgDataLow4Bytes(const CanRxMsg & msg)
{
	uint32_t low4Bytes = msg.Data[3];

	low4Bytes = (low4Bytes << 8) | msg.Data[2];
	low4Bytes = (low4Bytes << 8) | msg.Data[1];
	low4Bytes = (low4Bytes << 8) | msg.Data[0];

	return low4Bytes;
}

uint32_t huaWeiR48xxRxMsg::r48xxMsgDataHigh4Bytes(const CanRxMsg & msg)
{
	uint32_t high4Bytes = msg.Data[7];
	
	high4Bytes = (high4Bytes << 8) | msg.Data[6];
	high4Bytes = (high4Bytes << 8) | msg.Data[5];
	high4Bytes = (high4Bytes << 8) | msg.Data[4];

	return high4Bytes;
}

huaWeiR48xxRtDataMsg::huaWeiR48xxRtDataMsg()
{

}

void huaWeiR48xxRtDataMsg::rtMsgParse(const CanRxMsg & msg)
{
	msgProtoId	= huaWeiR48xxRxMsg::r48xxMsgProtocolId(msg);
	msgAddr		= huaWeiR48xxRxMsg::r48xxMsgAddress(msg);
	msgCmdId	= huaWeiR48xxRxMsg::r48xxMsgCmdId(msg);
	msgFromSrc	= huaWeiR48xxRxMsg::r48xxMsgFromSource(msg);
	msgCnt		= huaWeiR48xxRxMsg::r48xxMsgCount(msg);

	msgData0	= huaWeiR48xxRxMsg::r48xxMsgDataLow4Bytes(msg);
	msgData1	= huaWeiR48xxRxMsg::r48xxMsgDataHigh4Bytes(msg);

	dataSigId	= (((msg.Data[0]<<8)&0xFF00) | msg.Data[1]);
}

void huaWeiR48xxRtDataMsg::rtDataParse(void)
{
	//log("huaWeiR48xxRtDataMsg saddr: %x, dataSigId= %x\n",msgAddr, dataSigId);

	g_RtdataMgr.procDataFeild(msgAddr, dataSigId, msgData0, msgData1);
}

huaWeiR48xxGenMsg::huaWeiR48xxGenMsg()
{

}

huaWeiR48xxGenMsg::huaWeiR48xxGenMsg(uint32_t protoId, uint32_t addr, uint32_t cmdId)
	:msgProtoId(protoId),
	msgAddr(addr),
	msgCmdId(cmdId)
{

}

void huaWeiR48xxGenMsg::huaWeiR48xxSetData(void)
{
	
}

void huaWeiR48xxGenMsg::huaWeiR48xxSetData(uint32_t data0, uint32_t data1)
{
	msgData0 = data0;
	msgData1 = data1;
}

CanTxMsg huaWeiR48xxGenMsg::huaWeiR48xxCanTx(void)
{
	CanTxMsg msg;

	msg.IDE = CAN_Id_Extended;
    msg.RTR = CAN_RTR_Data;
	
	msg.ExtId = 0;
	msg.ExtId = ((msgProtoId<<23)&0x1F800000) 
			|((msgAddr<<16)&0x007F0000) 
			|((msgCmdId<<8)&0x0000FF00)
			|((msgFromSrc<<7)&0x00000080)
			|((msgRev<<1)&0x0000007E)
			|(msgCnt & 0x00000001);

	//log("huaWeiR48xxCanTx ExtId = %x\n", msg.ExtId);
	msg.DLC = 8;
	
	::memcpy(msg.Data, (uint8_t*)&msgData0, 4);
	::memcpy(msg.Data+4, (uint8_t*)&msgData1, 4);

	return msg;

}

void huaWeiR48xxGenMsg::huaWeiR48xxSetSrc(uint32_t src)
{
	msgFromSrc = src;
}

void huaWeiR48xxGenMsg::huaWeiR48xxSetCnt(uint32_t cnt)
{
	msgCnt = cnt;
}

void huaWeiR48xxGenMsg::huaWeiR48xxSetRev(uint32_t rev)
{
	msgRev = rev;
}
#endif
