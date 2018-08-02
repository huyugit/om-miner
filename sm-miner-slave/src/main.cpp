#include "mother_board.h"

#include "format.hpp"
#include "multy_board_mgr.h"


int main(void)
{
    g_motherBoard.init();

    log("\n");
    log("*********************************************\n");
    log("running minerRun()...\n");
    log("*********************************************\n");
    log("\n");

    g_multyBoardMgr.minerRun();

    return 0;
}

void * __dso_handle = 0;
