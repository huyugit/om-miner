#include "data_mgr.h"


#if defined(POWER_SUPPLY_USE_HUAWEI_R48XX)

dataMgr g_RtdataMgr;
dataMgr::dataMgr()
{
	//memset(arrayDtFeild, NULL, 30);
}

void dataMgr::init(void)
{
	dataFeild* feild = new outputVoltageFeild();
	regDataFeild(0x0175, feild);

	feild = new inputTempFeild();
	regDataFeild(0x180, feild);

	feild = new outPowerFeild();
	regDataFeild(0x173, feild);

	feild = new inputCurrentFeild();
	regDataFeild(0x172, feild);

	feild = new outputCurrentFeild();
	regDataFeild(0x182, feild);

	feild = new inputPowerFeild();
	regDataFeild(0x170, feild);

	feild = new characterFeild();
	regDataFeild(0x001, feild);

	feild = new versionFeild();
	regDataFeild(0x005, feild);

        feild = new inputVoltageFeild();
        regDataFeild(0x178, feild);


}

void dataMgr::regDataFeild(uint16_t sigId, dataFeild* feild)
{
	//arrayDtFeild[getArrayIndex(sigId)] = feild;
	//mapDtFeild[sigId] = feild;
	mapDtFeild.insert(std::pair<uint16_t, dataFeild*>(sigId, feild));
}

void dataMgr::procDataFeild(uint32_t addr, uint16_t sigId, uint32_t data0, uint32_t data1)
{
	//dataFeild* pdtFeild = arrayDtFeild[getArrayIndex(sigId)];

	std::map<uint16_t, dataFeild*>::iterator it =  mapDtFeild.find(sigId);
	if(it != mapDtFeild.end())
	{
       if(it->second != 0)
			it->second->parseDataFeild(addr, data0, data1);
	}
}

uint8_t dataMgr::getArrayIndex(uint16_t sigId)
{
	return (uint8_t)(sigId - 0x0170);
}

#endif

