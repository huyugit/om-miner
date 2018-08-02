#ifndef HBT_MGR_H
#define HBT_MGR_H

#include <stdint.h>
#include "board_mgr.h"


class HbtMgr
{
public:
    HbtMgr();
    void init();

    void poll();

private:
    uint32_t boardMaskTypeA;
    uint32_t boardMaskTypeB;


    BoardMgr& getBoard(int boardId);

    void storePwrSwTest(int index);

    void setPower(uint32_t mask, bool on);
    void testOcp(uint32_t boardMask);
    void testTmpAlert(uint32_t boardMask);
    void testBoardTmpAlert(uint8_t boardId, uint8_t tmpId);
    void runTests();
    void doTest(uint32_t boardMask);
    void doFanTest();

    void printInfo();
};


extern HbtMgr g_hbtMgr;

#endif // HBT_MGR_H
