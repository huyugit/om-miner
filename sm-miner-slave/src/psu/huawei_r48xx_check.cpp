#include "huawei_r48xx_check.h"
#include "huawei_r48xx_msg.h"
#include "can.h"

huaweiR48xxCheck::huaweiR48xxCheck()
{

}

bool huaweiR48xxCheck::autoCheckModule(void)
{
	bool check = false;
	for (int i = 0; i < 5; i++)
    {
        if (Can::rxMsgSize())
        {
            CanRxMsg msg = Can::rxPop();
            //processRxMsg(msg);
            check = processAutoIdentify(msg);
        }
        else {
            break;
        }
    }

	if(check)
	{
		return false;
	}
	else
	{
		sendGetInfoReq();
		return true;
	}
}

bool huaweiR48xxCheck::processAutoIdentify(CanRxMsg& msg)
{
	uint32_t ProtId	= huaWeiR48xxRxMsg::r48xxMsgProtocolId(msg);
	uint32_t CmdId	= huaWeiR48xxRxMsg::r48xxMsgCmdId(msg);

	if(ProtId == HUAWEI_R48XX_PROTOCOL_ID && CmdId == HUAWEI_R48XX_MSG_INFO_ID)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void huaweiR48xxCheck::sendGetInfoReq()
{
	huaWeiR48xxGenMsg msg = huaWeiR48xxGenMsg(HUAWEI_R48XX_PROTOCOL_ID, 0x00, HUAWEI_R48XX_MSG_INFO_ID);

	msg.huaWeiR48xxSetSrc(0x01);
	msg.huaWeiR48xxSetCnt(0x00);

	msg.huaWeiR48xxSetData(0x00, 0x00);
	msg.huaWeiR48xxSetRev(0x3F);
	//msg.huaWeiR48xxCanTx();

	Can::send(msg.huaWeiR48xxCanTx());
}