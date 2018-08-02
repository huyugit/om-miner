#ifndef SERVERTEST_H
#define SERVERTEST_H

#include <stdint.h>


class ServerTest
{
public:
    ServerTest();
    void run();

    void clearTestStat();

    int testStep, testType;

    int numSlave, numSlaveOk;
    int numBoard, numBoardOk;

private:
    void waitPowerOn();
    void waitFanOn();

    void runTest();
    bool checkResults();
    bool checkMasterResults();
    bool checkSlaveResults();
    bool checkFanResults();

    bool execCmd(uint32_t cmdFlags);
    void doCooling();

    int cmdId;
    bool flagMasterTest;
    bool flagSlaveTest;
    bool flagFanTest;
    bool flagHashBoardTest;
};

extern ServerTest g_serverTest;

#endif // SERVERTEST_H
