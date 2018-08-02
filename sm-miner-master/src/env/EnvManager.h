#ifndef ENV_MANAGER_H
#define ENV_MANAGER_H

#include "base/PollTimer.h"
#include "sys/writer/Writer.h"

#include <stdint.h>


class EnvManager
{
public:
    static int slaveCount;
    static bool nextSlaveId(int &id);

public:
    EnvManager();

    void init();

    void runPollingIteration();

    void printEnvStats();
    void printEnvStats(Writer &wr);

    bool isKeyPressed(uint8_t keyCode);

    //---------------------------------------------------------
    // Temperature
    //---------------------------------------------------------
public:
    int32_t  getFanBoardTemp() const { return 0; }
    uint32_t getTempNum() const { return tempNum; }
    int32_t getTempMin() const { return tempMin; }
    int32_t getTempMax() const { return tempMax; }
    int32_t getTempAvg() const { return tempAvg; }

private:
    uint32_t tempNum;
    int32_t tempMin;
    int32_t tempMax;
    int32_t tempAvg;

    void recalcTemperatureStats();
};

extern EnvManager g_envManager;

#endif // ENV_MANAGER_H
