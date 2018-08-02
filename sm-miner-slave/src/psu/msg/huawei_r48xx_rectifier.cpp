#include <cstring>
#include "huawei_r48xx_mgr.h"
#include "huawei_r48xx_rectifier.h"
#include "format.hpp"

huaWeiR48xxRectifier::huaWeiR48xxRectifier()
{
	info.id = 0;
	info.serial[0] = '\0';
#ifdef POWER_SUPPLY_USE_HUAWEI_R48XX
	info.measuredInPower = 0;
#endif
	info.measuredCurrent = 0;
	info.measuredVoltage = 0;
	info.measuredInVoltage = 0;
	info.ratedCurrent = 0;

	info.condition = 0;
	info.tempIn = 0;
	info.tempOut = 0;

	info.numStatus = 0;
	info.lastStatusTime = 0;
	
	info.minorAlarm = 0;
	info.majorAlarm = 0;


	spec.serial[0] = '\0';
	spec.prodDesc[0] = '\0';
	spec.prodPart[0] = '\0';
	spec.prodVer[0] = '\0';
}

