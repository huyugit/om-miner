#ifndef BOARD_MGR_H
#define BOARD_MGR_H

#include <stdint.h>
#include "mytime.h"
#include "polling_timer.h"
#include "ms_data.h"

#define ERR_HB_TEMP		0x1
#define ERR_PWR			0x2
#define ERR_FAN			0x3

#define HSB_TEMP_LOW 35
#define HSB_TEMP_HIGH 65
#define FAN_SPEED_BASE 2500
#define FAN_CTL_STEP 120
#define PWM_CTL_BASE 102
#define PWM_FULL_SPEED 255
#define PWM_CTL_STEP 5
#define AUTO_SPEED_MODE 1
#define FAN_FULL_SPEED 0

class BoardMgr
{
public:
    BoardMgr();
    void init(uint8_t boardId);

    void validateParams();

    void autoConfigure();
    void configure(const BoardSpec &manualSpec);
    void configureNone();

    void updateInfo();

    uint32_t getBoardRevision();
    uint8_t getBoardTemp(uint8_t addr);

    uint32_t getBoardVoltage();
    uint32_t getBoardCurrent(int pwrLine);

	void setBoardOSCconfig(uint8_t OSC);    //chenbo add 20180108
	void getBoardOSCconfig();    //chenbo add 20180108
	void setBoardSNforHash(char * sn_str); //chenbo add 20180123
	void getBoardSNforHash(); //chenbo add 20180123
	void setBoardBinforHash(uint8_t bin);    //gezhihua add 20180423
	void getBoardBinforHash();    //gezhihua add 20180423

    void onBoardConfig(BoardConfig &newConfig);
	bool checkBoardWorkCond(int *false_reason, int init_flag);
    void processPwrSw();
    void storePwrSwTest(int index);

    bool writeTmpAlert(uint8_t tmpId, uint8_t value=0xff);
	int checkFanSpeedThreshold(uint8_t temp);

	int8_t getBoardMaxTemp(void);

    void dump();

public:
    uint8_t boardId;

    BoardInfo info;
    BoardSpec spec;
    BoardConfig config;
    BoardTest test;

    // PwrSwitch monitoring
    uint32_t psLowTime;
    uint32_t psLowNum;

    uint32_t coolingStartTime;
    uint32_t coolingTotalTime;

    bool     heaterErrPrev;
};

#endif // BOARD_MGR_H
