/*
 * Record error information in log files
 */

#include "BflLog.h"

Bfllog gMsErrorLog;
Bfllog gStratumLog;

Bfllog::Bfllog() throw()
	:logFile(nullptr)
{

}

Bfllog::~Bfllog() throw()
{
	if(logFile){
		fclose(logFile);
	}
}

int Bfllog::initLogFile(const char *logPath)
{
	logFile = fopen(logPath, "a");
	if(nullptr == logFile){
		return -1;
	}

	return 0;
}

void Bfllog::writeLog(uint32_t errorCode)
{
	struct tm *tm_p = NULL;
	time_t tm;

	if(nullptr != logFile){
		memset(&tm, 0, sizeof(time_t));
		time(&tm);
		tm_p = localtime(&tm);

		/* filter bootup useless log */
		if((tm_p->tm_year + 1900) >= 2018)
		{
			fprintf(logFile, "%d-%d-%d %d:%d:%d %x\n", tm_p->tm_year + 1900, 
										  tm_p->tm_mon+1, 
										  tm_p->tm_mday,
										  tm_p->tm_hour,
										  tm_p->tm_min,
										  tm_p->tm_sec,
										  errorCode);
		}
		fflush(logFile);
	}
}
