#ifndef MS_DATA_PSU_H
#define MS_DATA_PSU_H

#include <stdint.h>
#include "ms_packet_ids.h"
#include "format.hpp"
#include "cmn_block.h"
#include "eltek_cmn.h"


struct PsuConfig
{
    static const uint8_t MS_ID = MS_ID_PsuConfig;

    bool noPsu;
    uint8_t masterPowerState;

    bool powerOn;

    bool fanSpeedUser;
    uint8_t fanSpeedValue;

    uint16_t voltage;
    uint16_t defaultVoltage;
    uint16_t minimalVoltage;
    uint16_t delaySec;

    uint16_t initialVoltage;
    uint16_t lowPowerTime;      // seconds to hold initial power

    uint16_t voltageStepUp;     // + santi volts per second
    uint16_t voltageStepDown;   // - santi volts per second

    uint16_t currentLimit;
    uint16_t currentLimitDelta;


    PsuConfig()
        : noPsu(false),
          masterPowerState(0),
          powerOn(true),
          fanSpeedUser(false),
          fanSpeedValue(100),
          voltage(4200),
          defaultVoltage(4200),
          delaySec(5),
          lowPowerTime(1)
    {
        setMinimalVoltage(42.0);
        setInitialVoltage(44.0);

        setVoltageStepUp(0.1);
        setVoltageStepDown(0.5);

        setCurrentLimit(70.0);
        setCurrentLimitDelta(5.0);
    }

    inline void setVoltage(double volts) { voltage = volts * 100; }
    inline double getVoltage() const { return (double)voltage / 100; }

    inline void setMinimalVoltage(double volts) { minimalVoltage = volts * 100; }
    inline double getMinimalVoltage() const { return (double)minimalVoltage / 100; }

    inline void setInitialVoltage(double volts) { initialVoltage = volts * 100; }
    inline double getInitialVoltage() const { return (double)initialVoltage / 100; }

    inline void setVoltageStepUp(double volts) { voltageStepUp = volts * 100; }
    inline double getVoltageStepUp() const { return (double)voltageStepUp / 100; }

    inline void setVoltageStepDown(double volts) { voltageStepDown = volts * 100; }
    inline double getVoltageStepDown() const { return (double)voltageStepDown / 100; }

    inline void setCurrentLimit(double ampers) { currentLimit = ampers * 10; }
    inline double getCurrentLimit() const { return (double)currentLimit / 10; }

    inline void setCurrentLimitDelta(double ampers) { currentLimitDelta = ampers * 10; }
    inline double getCurrentLimitDelta() const { return (double)currentLimitDelta / 10; }

    void dump() const
    {
        log("noPsu=%u, powerOn=%u, voltage=%u, defaultVoltage=%u, delaySec=%u\n",
            noPsu, powerOn, voltage, defaultVoltage, delaySec);
    }
}
__attribute__ ((packed));


struct PsuMgrInfo
{
    bool        powerOn;
    uint16_t    setVoltage;
    uint8_t     state;
    uint32_t    stateSec;

    PsuMgrInfo()
        : powerOn(0),
          setVoltage(0),
          state(0),
          stateSec(0)
    {}

    inline double getSetVoltage() const { return (double)setVoltage / 100; }
    inline PowerState getState() const { return PowerState(state); }

    void dump()
    {
        log("powerOn=%u, setVoltage=%u\n",
            powerOn, setVoltage);
    }

}
__attribute__ ((packed));


struct PsuInfo
{
    uint32_t id;
	char 	 serial[48];

#ifdef POWER_SUPPLY_USE_HUAWEI_R48XX
	uint16_t measuredInPower;
#endif

	uint16_t ratedCurrent;  //chenbo add 20180329, detect power type(3200/4000W)
	
    uint16_t measuredCurrent;
    uint16_t measuredVoltage;
    uint16_t measuredInVoltage;

    uint8_t  condition;
    int8_t  tempIn;
    int8_t  tempOut;

    uint32_t numStatus;
    uint32_t lastStatusTime;

    uint16_t minorAlarm;
    uint16_t majorAlarm;

    uint16_t defaultVoltage;

    bool     greenLed;
    bool     yellowLed;
    bool     redLed;

    uint32_t upTime;
    uint16_t fanSpeedRef;
    uint16_t fanSpeed;

	
    inline double getVOut() const { return (double)measuredVoltage / 100; }
    inline double getIOut() const { return (double)measuredCurrent / 10; }
    inline double getPOut() const { return getVOut() * getIOut(); }
    inline double getVIn()  const { return (double)measuredInVoltage; }

    bool operator<(const PsuInfo& other)
    {
        if (lastStatusTime < other.lastStatusTime)
            return true;

        if (lastStatusTime == other.lastStatusTime && numStatus == other.numStatus)
            return true;

        return false;
    }

    void dump()
    {
        log("PsuInfo: t=%u, num=%u, id=%u, serial=%s, U=%u sV, I=%u dA\n",
            lastStatusTime, numStatus, id, serial, measuredVoltage, measuredCurrent);
    }
}
__attribute__ ((packed));


struct PsuInfoArray
{
    static const uint8_t MS_ID = MS_ID_PsuInfoArray;

    PsuMgrInfo psuMgrInfo;

    uint8_t len;
    PsuInfo psuInfo[MAX_PSU_PER_SLAVE];

    PsuInfoArray() {
        memset(this, 0, sizeof(PsuInfoArray));
    }

    void dump()
    {
        log("PsuInfoArray: %d elements\n", len);
        for (int i = 0; i < len; i++)
        {
            log("  psuInfo[%d]: ", i); psuInfo[i].dump();
        }
    }
}
__attribute__ ((packed));


struct PsuSpec
{
    static const uint8_t MS_ID = MS_ID_PsuSpec;

    uint32_t id;

	char	 serial[48];

    char        prodDesc[27];
    char        prodPart[12];
    char        prodVer [6];

    uint16_t    prodYear;
    uint8_t     prodMonth;
    uint8_t     prodDay;

    void dump()
    {
        log("PsuSpec: id=%u, serial=%s\n",
            id, serial);
    }
}
__attribute__ ((packed));


#endif // MS_DATA_PSU_H
