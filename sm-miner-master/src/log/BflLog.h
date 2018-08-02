#ifndef BFLLOG_H
#define BFLLOG_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/un.h>
#include <fcntl.h>
#include <unistd.h>

#define MS_ERROR_LOG	"/var/tmp/ms-error.log"
#define NET_ERROR_LOG	"/var/tmp/stratum-error.log"

class Bfllog {
public:
	Bfllog() throw();
    ~Bfllog() throw();
	int initLogFile(const char *logPath);
	void writeLog(uint32_t errorCode);

private:
	FILE *logFile;
};

extern Bfllog gMsErrorLog;
extern Bfllog gStratumLog;
#endif
