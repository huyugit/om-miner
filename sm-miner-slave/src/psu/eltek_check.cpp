#include "eltek_check.h"
#include "eltek_msg.h"
#include "stm32f4xx_can.h"
#include "can.h"

EltekCheck::EltekCheck()
{

}

bool EltekCheck::autoCheckModule(void)
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
		sendLoginRequest();
		return true;
	}
}

bool EltekCheck::processAutoIdentify(CanRxMsg& msg)
{
	uint32_t sysType = EltekRxMsg::systemType(msg);
	uint32_t msgType = EltekRxMsg::msgType(msg);
    uint32_t variant = EltekRxMsg::variant(msg);

	if((sysType == 0x05)
		&&((msgType == 0x01)||(msgType == 0x02))
		&&((variant == 0x00)||(variant == 0x01)||(variant == 0x02)))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void EltekCheck::sendLoginRequest()
{
    EltekGenMessage msg;
    msg.subSystem = 15;
    msg.subSystemIndex = 1;
    msg.setFunction(21);

    Can::send(msg.canTx());
    Can::send2(msg.canTx());
}