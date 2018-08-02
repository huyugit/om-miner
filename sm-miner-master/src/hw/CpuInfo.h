#ifndef CPUINFO_H
#define CPUINFO_H


class CpuInfo
{
public:
    enum CpuType {
        CPU_RPI = 0,
        CPU_OPI,
        CPU_SOC,
        CPU_NA,
    };

    CpuInfo();
    void init();

    CpuType cpuType;
};

extern CpuInfo g_cpuInfo;

#endif // CPUINFO_H
