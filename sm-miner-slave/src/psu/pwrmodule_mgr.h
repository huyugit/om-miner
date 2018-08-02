#ifndef PWRMODULE_MGR_H
#define PWRMODULE_MGR_H
#include <algorithm>
#include <stdio.h>
#include "array.hpp"

#include "utils.h"
#include "IPwrModule.h"
#include "IPwrCheck.h"

class pwrModuleMgr
{
public:
	pwrModuleMgr();
	void process100ms();	
	void init();
	void setEnable();
	void setHBWorkCond(uint8_t typeIndex);
	HBWorkCond*		pHBWorkCond; 
	IPwrModule*		pModuleMgr;
private:
	bool			autoCheckFlag;
	bool			enabled;
	uint8_t			moduleIndex;// 0 --- eltek; 1 ----huawei; 2 ---- sag

	Array<IPwrCheck*, 5>	pwrCheck;
	Array<IPwrModule*, 5>	pwrModule;
};
extern pwrModuleMgr g_ModuleMgr;
#endif
