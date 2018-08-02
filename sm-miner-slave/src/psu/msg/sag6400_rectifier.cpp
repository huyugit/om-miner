#include "sag6400_rectifier.h"

SagRectifier::SagRectifier()
{
	info.id = 0;
	info.serial[0] = '\0';

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

