#ifndef DATA_MGR_H
#define DATA_MGR_H

#if defined(POWER_SUPPLY_USE_HUAWEI_R48XX)

#include <stdint.h>
#include "array.hpp"
#include "data_feild.h"
#include <map>

class dataMgr
{
public:
	dataMgr();
	void init(void);
	void regDataFeild(uint16_t sigId, dataFeild* feild);
	void procDataFeild(uint32_t addr, uint16_t sigId, uint32_t data0, uint32_t data1);
	
private:
	uint8_t getArrayIndex(uint16_t sigId);
	
private:
	//Array<dataFeild*, 30>		arrayDtFeild;
	std::map<uint16_t, dataFeild*>	mapDtFeild;
//bb[0]=NULL;
};
extern dataMgr g_RtdataMgr;

#endif

#endif
