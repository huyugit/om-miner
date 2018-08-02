#include "eltek_cmn.h"

#include <cstdio>
#include <cstring>


EltekSerialToStr::EltekSerialToStr(uint64_t uid)
{
    uint8_t *p = (uint8_t*)&uid;
    for (int i = 0; i < 8; i++)
    {
        sprintf(str+i*2, "%02X", p[7 - i]);
    }
}


const char *eltekAlarmToStr(int alarmId)
{
    switch (alarmId)
    {
    case 0:     return "OVSLockOut";
    case 1:     return "ModuleFailPrimary";
    case 2:     return "ModuleFailSecondary";
    case 3:     return "HighMains";
    case 4:     return "LowMains";
    case 5:     return "HighTemp";
    case 6:     return "LowTemp";
    case 7:     return "CurrentLim";
    case 8:     return "InternalVolt";
    case 9:     return "ModuleFail";
    case 10:    return "Fan1SpeedLow";
    case 11:    return "Fan2SpeedLow";
    case 12:    return "LowOutputVolt";
    default:    return "UNKNOWN";
    }
}


const char *eltekConditionToStr(int condition)
{
    switch (condition)
    {
    case COND_Error:        return "Error";
    case COND_Normal:       return "Normal";
    case COND_Minor_Alarm:  return "MinorAlarm";
    case COND_Major_Alarm:  return "MajorAlarm";
    case COND_Disabled:     return "Disabled";
    case COND_Disconnected: return "Disconnected";
    case COND_Major_Low:    return "MajorLow";
    case COND_Minor_Low:    return "MinorLow";
    case COND_Major_High:   return "MajorHigh";
    case COND_Minor_High:   return "MinorHigh";
    default: return "UNKNOWN";
    }
}


EltekLedsToStr::EltekLedsToStr(bool g, bool y, bool r)
{
    snprintf(str, sizeof(str), "%s%s%s",
             g ? "G" : "-",
             y ? "Y" : "-",
             r ? "R" : "-");
}


EltekAlarmSetToStr::EltekAlarmSetToStr(uint16_t majorAlarm, uint16_t minorAlarm)
{
    memset(str, 0, sizeof(str));

    char *p = str;
    int n = sizeof(str);

    if (majorAlarm)
    {
        if (!appendStr(p, n, "MAJOR: ")) return;
        if (!appendAlarm(p, n, majorAlarm)) return;
    }

    if (minorAlarm)
    {
        if (!appendStr(p, n, "MINOR: ")) return;
        if (!appendAlarm(p, n, minorAlarm)) return;
    }
}

bool EltekAlarmSetToStr::appendAlarm(char* &p, int &n, uint16_t alarm)
{
    for (int i = 0; i <= ALARM_MAX; i++)
        if (alarm & (1 << i))
        {
            if (!appendStr(p, n, eltekAlarmToStr(i)))
                return false;
            if (!appendStr(p, n, " "))
                return false;
        }
    return true;
}

bool EltekAlarmSetToStr::appendStr(char* &p, int &n, const char* data)
{
    int res = snprintf(p, n, data);

    if (res < 0 && res >= n)
        return false;

    p += res;
    n -= res;

    return true;
}


const char *powerStateToStr(PowerState state)
{
    switch (state)
    {
    case STATE_INITIAL_POWER:   return "INITIAL_POWER";
    case STATE_FULL_POWER:      return "FULL_PWER";
    case STATE_POWER_OFF:       return "POWER_OFF";
    case STATE_MAX:             return "N/A";
    default:                    return "N/A";
    }
}
