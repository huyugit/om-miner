#ifndef POWERMGR_H
#define POWERMGR_H

#include <stdint.h>
#include "ms-protocol/ms_data.h"
#include "eltek_cmn.h"


class PowerMgr
{
public:
    PowerMgr();

    void step();

    uint16_t calcSetVoltage();
    uint16_t calcFullPowerVoltage();

    bool isPowerDetected();

//private:
    PowerState nextState();
    PowerState nextSequentialState();

    bool isVoltageStable();
    bool isCurrentOk();

//private:
    PowerState state;
    uint32_t stateTime;
    uint32_t stableVOutTime;

    uint16_t setVoltage;

    PsuMgrInfo info;
};

extern PowerMgr g_powerMgr;

#endif // POWERMGR_H
