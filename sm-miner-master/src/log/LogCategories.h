#ifndef LOG_CATEGORIES_H
#define LOG_CATEGORIES_H
/*
 * Declares a set of log category name constants.
 */


namespace LogCategories
{
    const char* const c_appLogCtg           = "app";
    const char* const c_configLogCtg        = "conf";
    const char* const c_gpioLogCtg          = "gpio";
    const char* const c_lcdLogCtg           = "lcd";
    const char* const c_stratumLogCtg       = "stratum";
    const char* const c_eventLogCtg         = "event";
    const char* const c_telnetLogCtg        = "telnet";

}

#endif  // LOG_CATEGORIES_H
