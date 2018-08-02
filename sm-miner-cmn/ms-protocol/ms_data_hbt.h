#ifndef MS_DATA_HBT_H
#define MS_DATA_HBT_H

#include <stdint.h>
#include "cmn_block.h"
#include "ms_defines.h"
#include "ms_data_fan.h"


#define HBT_CMD_A_ON        0x0001
#define HBT_CMD_A_OFF       0x0002
#define HBT_CMD_A_TEST      0x0004
#define HBT_CMD_B_ON        0x0010
#define HBT_CMD_B_OFF       0x0020
#define HBT_CMD_B_TEST      0x0040
#define HBT_CMD_FAN_TEST    0x0100


struct SlaveHbtConfig
{
    uint16_t cmdId;
    uint16_t cmdFlags;

    uint16_t testBtcTime;
    uint16_t testFanDelay;

    SlaveHbtConfig()
        : cmdId(0),
          cmdFlags(0),
          testBtcTime(3000),
          testFanDelay(500)
    {}

    void dump()
    {
        log("SlaveHbtConfig:\n");
    }
}
__attribute__ ((packed));


struct BoardPwrSwTest
{
    bool        pwrOn;
    uint32_t    voltage;
    uint16_t    currents[MAX_PL_PER_BOARD];

    void dump()
    {
        log("pwrOn=%u, voltage=%u", pwrOn, voltage);

        for (int i = 0; i < MAX_PL_PER_BOARD; i++)
            log(", I%d=%u", i, currents[i]);

        log("\n");
    }
}
__attribute__ ((packed));


struct BoardPwrSwTestArr
{
    static const size_t NUM = 4;
    BoardPwrSwTest items[NUM];

    void dump()
    {
        log("BoardPwrSwTestArr:\n");
        for (size_t i = 0; i < NUM; i++) {
            log("  item[%d]: ", i); items[i].dump();
        }
    }
}
__attribute__ ((packed));


struct SlaveTestInfo
{
    uint16_t ackId;
    uint16_t ackRes;

    FanTestArr fanTest;

    void dump()
    {
        log("SlaveTestInfo:\n");
        log("  ackId:       0x%08x\n", ackId);
    }
}
__attribute__ ((packed));


#define TAS_NA              0
#define TAS_OK              1
#define TAS_ERR_BEGIN_I     2
#define TAS_ERR_WRITE       3
#define TAS_ERR_OFF_I       4

struct BoardTest
{
    BoardPwrSwTestArr pwrSwTest;
    uint8_t testOcpOff;
    uint8_t tmpAlertStatus[MAX_TMP_PER_BOARD];

    void dump()
    {
        log("BoardTest:\n");
    }
}
__attribute__ ((packed));

#endif // MS_DATA_HBT_H
