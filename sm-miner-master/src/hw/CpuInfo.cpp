#include "CpuInfo.h"

#include <stdio.h>
#include <string.h>

#include "except/SystemException.h"


CpuInfo g_cpuInfo;


CpuInfo::CpuInfo()
    : cpuType(CPU_NA)
{}

void CpuInfo::init()
{
    const char* fileName = "/proc/cpuinfo";
    FILE* fp = fopen(fileName, "r");
    if (!fp) {
        throw SystemException("CpuInfo: unable to open %s", fileName);
    }

    char buffer[4*1024];
    size_t bytes_read = fread(buffer, 1, sizeof(buffer), fp);
    fclose (fp);

    if (bytes_read == 0 || bytes_read == sizeof(buffer)) {
        throw SystemException("CpuInfo: %s: file read error %s", fileName);
    }

    buffer[bytes_read] = '\0';

    char* match = strstr(buffer, "Hardware");
    if (!match) {
        throw SystemException("CpuInfo: can not find 'Hardware' item");
    }

    char hwid[1024];
    sscanf(match, "Hardware : %s", hwid);

    printf("CpuInfo: hardware id: %s\n", hwid);

    if (strstr(hwid, "BCM")) {
        cpuType = CPU_RPI;
        printf("CpuInfo: CPU_RPI\n");
    }
    else if (strstr(hwid, "sun8i")) {
        cpuType = CPU_OPI;
        printf("CpuInfo: CPU_OPI\n");
    }
    else if (strstr(hwid, "Altera")) {
        cpuType = CPU_SOC;
        printf("CpuInfo: CPU_SOC\n");
    }
    else {
        throw SystemException("CpuInfo: unexpected hardware id");
    }
}
