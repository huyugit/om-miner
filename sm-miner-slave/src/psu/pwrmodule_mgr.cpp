#include "pwrmodule_mgr.h"
#include "huawei_r48xx_mgr.h"
#include "master_gate.h"
#include "eltek_mgr.h"
#include "eltek_check.h"
#include "huawei_r48xx_check.h"
#include "sag6400_check.h"
#include "sag6400_mgr.h"
#include "common.h"

pwrModuleMgr g_ModuleMgr;
HBWorkCond g_HashBoardWorkCond[3] = {
	{	
		/* Eltek 3000W */
		10,			/* PSU Power output low threshold PsuPoutLowTh*/
		3050,		/* PSU Power output high threshold PsuPoutHighTh */
		0.1,		/* PSU Current output low threshold PsuIoutLowTh */
		51,			/* PSU Voltage output high threshold PsuVoutHighTh */
		70,			/* PSU Current output high threshold PsuIoutHighTh*/
		70,			/* PSU Fan Duty ratio low threshold PsuFanSpeedLowTh*/
		5400,		/* Fan RPM low threshold FanRpmLowTh */
		1,			/* Fan status check enable FanFlag */
		1,			/* Hash board check enable HashTempFlag */
		85,			/* Hash board temperature high threshold */
		1,			/* PSU status check enable PsuFlag */
		1,			/* Fan work in auto speed mode FanWorkMode */
		85			/* PSU Temperature high threshold PsuTempOutHighTh */
	},
	{
		/* HuaweiR48xx 3000W */
		10,			/* PSU Power output low threshold PsuPoutLowTh*/
		3050,		/* PSU Power output high threshold PsuPoutHighTh */
		0.1,			/* PSU Current output low threshold PsuIoutLowTh */
		51,			/* PSU Voltage output high threshold PsuVoutHighTh */
		70,			/* PSU Current output high threshold PsuIoutHighTh*/
		70,			/* PSU Fan Duty ratio low threshold PsuFanSpeedLowTh*/
		5400,		/* Fan RPM low threshold FanRpmLowTh */
		1,			/* Fan status check enable FanFlag */
		1,			/* Hash board check enable HashTempFlag */
		85,			/* Hash board temperature high threshold */
		1,			/* PSU status check enable PsuFlag */
		1,			/* Fan work in auto speed mode FanWorkMode */
		85			/* PSU Temperature high threshold PsuTempOutHighTh */
	},
	{
		/* Sag6400 3000W */
		10,			/* PSU Power output low threshold PsuPoutLowTh*/
		3050,		/* PSU Power output high threshold PsuPoutHighTh */
		0.1,			/* PSU Current output low threshold PsuIoutLowTh */
		51,			/* PSU Voltage output high threshold PsuVoutHighTh */
		70,			/* PSU Current output high threshold PsuIoutHighTh*/
		70,			/* PSU Fan Duty ratio low threshold PsuFanSpeedLowTh*/
		5400,		/* Fan RPM low threshold FanRpmLowTh */
		1,			/* Fan status check enable FanFlag */
		1,			/* Hash board check enable HashTempFlag */
		85,			/* Hash board temperature high threshold */
		1,			/* PSU status check enable PsuFlag */
		1,			/* Fan work in auto speed mode FanWorkMode */
		85			/* PSU Temperature high threshold PsuTempOutHighTh */
	}
};

pwrModuleMgr::pwrModuleMgr()
{
	autoCheckFlag	= true;
	enabled			= false;
	pModuleMgr		= nullptr;
	pHBWorkCond 	= nullptr;
	moduleIndex		= 0;
}

void pwrModuleMgr::init()
{
	pwrCheck[0] 	= new EltekCheck();
	pwrCheck[1] 	= new huaweiR48xxCheck();
	
	pwrModule[0]	= new EltekMgr();

#if defined(POWER_SUPPLY_USE_HUAWEI_R48XX)
	pwrModule[1]	= new huaWeiR48xxMgr();
#endif

	pwrCheck[2] 	= new sag6400Check();
	pwrModule[2]	= new Sag6400Mgr();

	Can::init();
}

void pwrModuleMgr::setHBWorkCond(uint8_t typeIndex)
{
	pHBWorkCond = &g_HashBoardWorkCond[typeIndex];
}

void pwrModuleMgr::process100ms()
{
	if(enabled == false) return;

	if(autoCheckFlag == true)
	{
		static uint8_t checkcnt = 0;
		autoCheckFlag = pwrCheck[moduleIndex]->autoCheckModule();

		if(true == autoCheckFlag)
		{
			if(checkcnt > 5)
			{
				checkcnt = 0;
				moduleIndex++;
			}
			checkcnt++;
		}

		if(false == autoCheckFlag)
		{
			pModuleMgr = pwrModule[moduleIndex];
			pModuleMgr->init();
			setHBWorkCond(moduleIndex);
		}
	}
	else
	{
		pModuleMgr->process100ms();
	}
}

void pwrModuleMgr::setEnable()
{
	enabled = (g_masterGate.masterData.slaveId == 0);
}

