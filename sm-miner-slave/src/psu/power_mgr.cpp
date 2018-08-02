#include "power_mgr.h"
#include "master_gate.h"
#include "multy_board_mgr.h"
#include "mytime.h"
#include "utils.h"
#include "pwrmodule_mgr.h"

PowerMgr g_powerMgr;
PowerMgr::PowerMgr()
    : state(STATE_INITIAL_POWER),
      stateTime(0),
      stableVOutTime(0),
      setVoltage(0)
{
}

void PowerMgr::step()
{
    uint32_t now = getMiliSeconds();

    PowerState prevState = state;
    state = nextState();

    if (prevState != state)
    {
        log("PowerMgr: state %s", powerStateToStr(state));
        stateTime = now;
    }

    info.powerOn        = g_psuConfig.powerOn;
    info.setVoltage     = setVoltage;
    info.state          = state;
    info.stateSec       = now - stateTime;
}

PowerState PowerMgr::nextState()
{
    uint32_t now = getMiliSeconds();
    bool powerCondition = g_psuConfig.powerOn;

    // common checks for all states
    if (!powerCondition)
    {
        if (state == STATE_POWER_OFF ||
            state == STATE_INITIAL_POWER)
        {
            if (state != STATE_POWER_OFF)
            {
                if (!g_psuConfig.powerOn) {
                    log("PowerMgr: power off *");
                }
            }

            return STATE_POWER_OFF;
        }
    }

    switch (state)
    {
	    case STATE_INITIAL_POWER:
	    {
	        if (g_ModuleMgr.pModuleMgr->getNumRectifiers() == 0)
	            return state;

	        if (!isVoltageStable()) {
	            stableVOutTime = now;
	        }

	        if (now - stableVOutTime < g_psuConfig.lowPowerTime * 1000)
	            return state;

	        if (!isVoltageStable())
	            return state;

	        break;
	    }
	    case STATE_FULL_POWER:
	    {
	        if (!powerCondition)
	            return STATE_POWER_OFF;

	        if (!isCurrentOk())
	            return STATE_POWER_OFF;

	        return state; // loop back
	        break;
	    }
	    case STATE_POWER_OFF:
	    {
	        break;
	    }
		case STATE_MAX:
		{
			break;
		}
    }

    // by default go to next sequential state
    return nextSequentialState();
}

bool PowerMgr::isPowerDetected()
{
    if (g_psuConfig.noPsu) {
        return true;
    }

    if (g_masterGate.masterData.slaveId > 0) {
        return (g_psuConfig.masterPowerState == STATE_FULL_POWER);
    }

    return (state == STATE_FULL_POWER);
}


PowerState PowerMgr::nextSequentialState()
{
    return PowerState(state < STATE_MAX-1 ? state + 1 : state);
}

uint16_t PowerMgr::calcSetVoltage()
{
    if (state == STATE_INITIAL_POWER)
    {
        setVoltage = g_psuConfig.initialVoltage;
    }
    else if (state == STATE_FULL_POWER)
    {
        setVoltage = calcFullPowerVoltage();

        if (setVoltage > 0)
        {
            if (setVoltage > g_psuConfig.voltage)
            {
                setVoltage = g_psuConfig.voltage;
            }
            if (setVoltage < g_psuConfig.minimalVoltage)
            {
                setVoltage = g_psuConfig.minimalVoltage;
            }
        }
    }
    else {
        setVoltage = 0;
    }

    return setVoltage;
}

uint16_t PowerMgr::calcFullPowerVoltage()
{
    const uint16_t limit1 = g_psuConfig.currentLimit - g_psuConfig.currentLimitDelta;
    const uint16_t limit2 = g_psuConfig.currentLimit + g_psuConfig.currentLimitDelta;

    bool limitReached = false;

	log("calcFullPowerVoltage limit1 = %d,limit2 = %d\n",limit1,limit2);
    for (int i = 0; i < g_ModuleMgr.pModuleMgr->getNumRectifiers(); ++i)
    {
        uint16_t current = g_ModuleMgr.pModuleMgr->getRectifierInforByIndex(i)->measuredCurrent;

		log("calcFullPowerVoltage i = %d,current = %d\n",i,current);
        if (current > limit2)
        {
            return setVoltage - g_psuConfig.voltageStepDown;
        }
        if (current > limit1)
        {
            limitReached = true;
        }
    }

    if (limitReached)
    {
        return setVoltage;
    }

	log("calcFullPowerVoltage setVoltage = %d,voltage = %d\n",setVoltage,g_psuConfig.voltage);
    if (setVoltage + g_psuConfig.voltageStepUp < g_psuConfig.voltage)
    {
        return setVoltage + g_psuConfig.voltageStepUp;
    }

    return setVoltage;
}

bool PowerMgr::isVoltageStable()
{
    for (uint32_t i = 0; i < g_ModuleMgr.pModuleMgr->getNumRectifiers(); i++)
    {
        if (ABS(g_ModuleMgr.pModuleMgr->getRectifierInforByIndex(i)->measuredVoltage - setVoltage) > 1 * 100)
        {
            return false;
        }
    }
    return true;
}

bool PowerMgr::isCurrentOk()
{
    return true;
}

