#include "data_feild.h"
#include "utils.h"
#include <map>
#include "format.hpp"
#include "huawei_r48xx_mgr.h"
#include "pwrmodule_mgr.h"

#if defined(POWER_SUPPLY_USE_HUAWEI_R48XX)

dataFeild::dataFeild()
{

}

dataFeild::dataFeild(uint16_t sId)
	:sigId(sId)
{

}

uint16_t dataFeild::getSingalId(void)
{
	return sigId;
}

inputPowerFeild::inputPowerFeild()
	:dataFeild(0x0170)
{

}

uint32_t inputPowerFeild::parseDataFeild(uint32_t addr, uint32_t data0, uint32_t data1)
{
	uint32_t power = 0;

	data0 = data0; //just clear compile warning
	power = SwapEndian(data1);
	power = (power) >> 10;

	huaWeiR48xxRectifier* rect = (huaWeiR48xxRectifier*)g_ModuleMgr.pModuleMgr->rectifierById((uint8_t)addr);
	if(rect)
	{
		rect->info.measuredInPower = (uint16_t)power;
	}

	return 0;
}

inputFreqFeild::inputFreqFeild()
	:dataFeild(0x0171)
{

}

uint32_t inputFreqFeild::parseDataFeild(uint32_t addr, uint32_t data0, uint32_t data1)
{
	return addr + data0 + data1; //just clear compile warning
}

inputCurrentFeild::inputCurrentFeild()
	:dataFeild(0x0172)
{

}

uint32_t inputCurrentFeild::parseDataFeild(uint32_t addr, uint32_t data0, uint32_t data1)
{
	uint32_t volt = 0;
	
	data0 = data0; //just clear compile warning
	volt = SwapEndian(data1);
	volt = (volt) >> 10;

	huaWeiR48xxRectifier* rect = (huaWeiR48xxRectifier*)g_ModuleMgr.pModuleMgr->rectifierById((uint8_t)addr);
	if(rect)
	{
//		rect->info.measuredInVoltage = rect->info.measuredInPower / volt;
	}

	return volt;
}

outPowerFeild::outPowerFeild()
	:dataFeild(0x0173)
{

}

uint32_t outPowerFeild::parseDataFeild(uint32_t addr, uint32_t data0, uint32_t data1)
{
	uint32_t power = 0;

	data0 = data0; //just clear compile warning	
	power = SwapEndian(data1);
	power = (power * 100) >> 10;

	huaWeiR48xxRectifier* rect = (huaWeiR48xxRectifier*)g_ModuleMgr.pModuleMgr->rectifierById((uint8_t)addr);
	if(rect)
	{
		rect->info.measuredInPower = (uint16_t)power;
	}

	return 0;
}

rtEfficiencyFeild::rtEfficiencyFeild()
	:dataFeild(0x0174)
{

}

uint32_t rtEfficiencyFeild::parseDataFeild(uint32_t addr, uint32_t data0, uint32_t data1)
{
	return addr + data0 + data1; //just clear compile warning
}

outputVoltageFeild::outputVoltageFeild()
	:dataFeild(0x0175)
{

}

uint32_t outputVoltageFeild::parseDataFeild(uint32_t addr, uint32_t data0, uint32_t data1)
{
	uint32_t volt = 0;

	data0 = data0; //just clear compile warning
	volt = SwapEndian(data1);
	volt = (volt*100) >> 10;

	huaWeiR48xxRectifier* rect = (huaWeiR48xxRectifier*)g_ModuleMgr.pModuleMgr->rectifierById((uint8_t)addr);
	if(rect)
	{
		rect->info.measuredVoltage = (uint16_t)volt;
	}

	return volt;
}

inputVoltageFeild::inputVoltageFeild()
       :dataFeild(0x0178)
{

}

uint32_t inputVoltageFeild::parseDataFeild(uint32_t addr, uint32_t data0, uint32_t data1)
{
       uint32_t volt = 0;

       data0 = data0; //just clear compile warning
       volt = SwapEndian(data1);
       volt = (volt*100) >> 10;

       huaWeiR48xxRectifier* rect = (huaWeiR48xxRectifier*)g_ModuleMgr.pModuleMgr->rectifierById((uint8_t)addr);
       if(rect)
       {
               rect->info.measuredInVoltage = (uint16_t)volt / 100;
//             rect->info.measuredVoltage = (uint16_t)volt;
       }

       log("parseDataFeild: data0: %x, data1: %x; inputVoltage: %x\n", data0, data1, volt);

        return volt;
 }



inputTempFeild::inputTempFeild()
	:dataFeild(0x0180)
{

}

uint32_t inputTempFeild::parseDataFeild(uint32_t addr, uint32_t data0, uint32_t data1)
{
	int32_t temp = 0;

	data0 = data0; //just clear compile warning
	temp = SwapEndian(data1);
	temp = temp >> 10;

	huaWeiR48xxRectifier* rect = (huaWeiR48xxRectifier*)g_ModuleMgr.pModuleMgr->rectifierById((uint8_t)addr);
	if(rect)
	{
		rect->info.tempIn = temp;
	}

	return temp;
}

//signal ID:	0x0181
outputTempFeild::outputTempFeild()
	:dataFeild(0x0181)
{

}

uint32_t outputTempFeild::parseDataFeild(uint32_t addr, uint32_t data0, uint32_t data1)
{
	int32_t temp = 0;

	data0 = data0; //just clear compile warning
	temp = SwapEndian(data1);
	temp = temp >> 10;
	
	huaWeiR48xxRectifier* rect = (huaWeiR48xxRectifier*)g_ModuleMgr.pModuleMgr->rectifierById((uint8_t)addr);
	if(rect)
	{
		rect->info.tempOut = temp;
	}

	return temp;
}



outputCurrentFeild::outputCurrentFeild()
	:dataFeild(0x0182)
{

}

uint32_t outputCurrentFeild::parseDataFeild(uint32_t addr, uint32_t data0, uint32_t data1)
{
	uint32_t temp = 0;

	data0 = data0; //just clear compile warning
	temp = SwapEndian(data1);
	temp = (temp * 10) >> 10;

	huaWeiR48xxRectifier* rect = (huaWeiR48xxRectifier*)g_ModuleMgr.pModuleMgr->rectifierById((uint8_t)addr);
	if(rect)
	{
		rect->info.measuredCurrent = (uint16_t)temp;
	}
	
	return temp;
}

//signal ID:	0x0001
characterFeild::characterFeild()
	:dataFeild(0x0001)
{

}

uint32_t characterFeild::parseDataFeild(uint32_t addr, uint32_t data0, uint32_t data1)
{
	uint32_t ratedCurr = 0;

	data0 = data0; //just clear compile warning
	ratedCurr = SwapEndian(data1);
	ratedCurr = ((ratedCurr >> 16)&0x03FF)>>1;

	huaWeiR48xxRectifier* rect = (huaWeiR48xxRectifier*)g_ModuleMgr.pModuleMgr->rectifierById((uint8_t)addr);
	if(rect)
	{
		rect->info.ratedCurrent = (uint16_t)ratedCurr;
	}

	return ratedCurr;
}


//signal ID:	0x0005
versionFeild::versionFeild()
	:dataFeild(0x0005)
{


}

uint32_t versionFeild::parseDataFeild(uint32_t addr, uint32_t data0, uint32_t data1)
{
	char version[8] = { 0 };

	version[0] = ((char *)&data1)[0] + 0x30;
	version[1] = ((char *)&data1)[1] + 0x30;
	version[2] = ((char *)&data1)[2] + 0x30;
	version[3] = ((char *)&data1)[3] + 0x30;
	version[4] = ((char *)&data0)[2] + 0x30;
	version[5] = ((char *)&data0)[3] + 0x30;

	log("versionFeild %x, %x, addr = %x,  %s, %x, %x, %x, %x, %x, %x\n", data1, data0, addr, version, 
		version[0], version[1], version[2],
		version[3], version[4], version[5]);
	
	huaWeiR48xxRectifier* rect = (huaWeiR48xxRectifier*)g_ModuleMgr.pModuleMgr->rectifierById((uint8_t)addr);
	if(rect)
	{
		strncpy(rect->spec.prodVer, version, 5);
		rect->spec.prodVer[5] = '\0';
	}

	return 0;
}


#endif
