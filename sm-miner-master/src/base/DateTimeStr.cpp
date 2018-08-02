#include "DateTimeStr.h"

#include <ctime>
#include <cstdio>

DateTimeStr::DateTimeStr()
{
    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);

    snprintf(str, sizeof(str), "%04d-%02d-%02d %02d:%02d:%02d",
             1900+timeinfo->tm_year, timeinfo->tm_mon+1, timeinfo->tm_mday,
             timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
}
