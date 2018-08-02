#include "sag6400_check.h"
#include "sag6400_msg.h"
#include "stm32f4xx_can.h"
#include "can.h"

sag6400Check::sag6400Check()
{

}

bool sag6400Check::autoCheckModule(void)
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

bool sag6400Check::processAutoIdentify(CanRxMsg& msg)
{
	uint16_t ProtId = SagRXMSG::sagMsgProfileID(msg);
	
	if(ProtId == 0x163)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void sag6400Check::sendLoginRequest()
{

}