#include "EnvManager.h"

#include "app/Application.h"
#include "env/GpioPolling.h"
#include "pool/StratumPool.h"
#include "stats/MasterStat.h"

#include "events/EventManager.h"

#include "test/ServerTest.h"

#include "config/Config.h"
#include "base/MinMax.h"
#include "sys/writer/StreamWriter.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>


EnvManager g_envManager;


int EnvManager::slaveCount = 0;

bool EnvManager::nextSlaveId(int &id)
{
    const uint32_t slaveMask = Application::config()->slaveMask;
    
    id++;
    while (id < slaveCount)
    {
        if (slaveMask & (1 << id))
        {
            return true;
        }
        id++;
    }
    return false;
}


EnvManager::EnvManager()
    : tempNum(0), tempMin(0), tempMax(0), tempAvg(0)
{}

void EnvManager::init()
{
}

void EnvManager::runPollingIteration()
{
    recalcTemperatureStats();
}

void EnvManager::printEnvStats()
{
    StreamWriter wr(stdout);
    printEnvStats(wr);
}

void EnvManager::printEnvStats(Writer &wr)
{
    wr.printf("Temp(C) (min/avr/max): %d / %d / %d\n", tempMin, tempAvg, tempMax);
}

//---------------------------------------------------------
// Temperature
//---------------------------------------------------------

void EnvManager::recalcTemperatureStats()
{
    tempNum = 0;
    tempMin = 0;
    tempMax = 0;
    tempAvg = 0;

    for (int i = 0; i < MAX_SLAVE_COUNT; i++)
    {
        for (int j = 0; j < MAX_BOARD_PER_SLAVE; j++)
        {
            BoardStat &board = g_masterStat.getSlave(i).getBoardStat(j);
            if (!board.isFound()) continue;

            for (int k = 0; k < board.spec.getNumTmp(); k++)
            {
                int32_t t = board.info.boardTemperature[k];

                if (tempNum > 0)
                {
                    tempMin = min(tempMin, t);
                    tempMax = max(tempMax, t);
                }
                else
                {
                    tempMin = t;
                    tempMax = t;
                }

                tempNum++;
                tempAvg += t;
            }
        }
    }

    if (tempNum > 0)
        tempAvg /= tempNum;
}
