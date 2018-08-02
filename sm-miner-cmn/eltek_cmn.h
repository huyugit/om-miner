#ifndef ELTEK_CMN_H
#define ELTEK_CMN_H

#include <stdint.h>


struct EltekSerialToStr
{
    char str[8*2 + 1];
    EltekSerialToStr(uint64_t uid);
};


#define ALARM_OVSLockOut            (1 << 0)
#define ALARM_ModuleFailPrimary     (1 << 1)
#define ALARM_ModuleFailSecondary   (1 << 2)
#define ALARM_HighMains             (1 << 3)
#define ALARM_LowMains              (1 << 4)
#define ALARM_HighTemp              (1 << 5)
#define ALARM_LowTemp               (1 << 6)
#define ALARM_CurrentLim            (1 << 7)
#define ALARM_InternalVolt          (1 << 8)
#define ALARM_ModuleFail            (1 << 9)
#define ALARM_Fan1SpeedLow          (1 << 10)
#define ALARM_Fan2SpeedLow          (1 << 11)
#define ALARM_LowOutputVolt         (1 << 12)
#define ALARM_MAX                   12


enum EltekConditionType
{
    COND_Error          = 0,
    COND_Normal         = 1,
    COND_Minor_Alarm    = 2,
    COND_Major_Alarm    = 3,
    COND_Disabled       = 4,
    COND_Disconnected   = 5,
    COND_Major_Low      = 8,
    COND_Minor_Low      = 9,
    COND_Major_High     = 10,
    COND_Minor_High     = 11,
};


const char* eltekAlarmToStr(int alarmId);
const char* eltekConditionToStr(int condition);


struct EltekLedsToStr
{
    char str[4];
    EltekLedsToStr(bool g, bool y, bool r);
};


struct EltekAlarmSetToStr
{
    char str[256];
    EltekAlarmSetToStr(uint16_t majorAlarm, uint16_t minorAlarm);

private:
    static bool appendAlarm(char* &p, int &n, uint16_t alarm);
    static bool appendStr(char* &p, int &n, const char* data);
};


enum PowerState
{
    STATE_INITIAL_POWER = 0,
    STATE_FULL_POWER,
    STATE_POWER_OFF,
    STATE_MAX
};

const char* powerStateToStr(PowerState state);

#endif // ELTEK_CMN_H
