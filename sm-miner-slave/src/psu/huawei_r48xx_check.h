#ifndef HUAWEI_R48XX_CHECK_H
#define HUAWEI_R48XX_CHECK_H

#include "stm32f4xx_can.h"
#include "IPwrCheck.h"

class huaweiR48xxCheck : public IPwrCheck
{
public:
	huaweiR48xxCheck();

	virtual bool autoCheckModule(void);
private:
	bool processAutoIdentify(CanRxMsg& msg);
	void sendGetInfoReq();
};
#endif

