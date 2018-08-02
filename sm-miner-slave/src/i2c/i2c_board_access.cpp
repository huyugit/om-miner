#include "i2c_board_access.h"

#include "mother_board.h"
#include "multy_board_mgr.h"


Mutex I2CBoardAccess::sharedAccess;


I2CBoardAccess::I2CBoardAccess(uint8_t _boardId)
    : boardId(_boardId)
{
    sharedAccess.lock();
}

I2CBoardAccess::~I2CBoardAccess()
{
    sharedAccess.unlock();
}

I2CSw& I2CBoardAccess::i2c() const
{
    return g_motherBoard.i2cBoards[boardId];
}

I2CReg I2CBoardAccess::ocpReg() const
{
    return I2CReg( i2c() );
}

I2CTmp75 I2CBoardAccess::tmp75(uint8_t addr) const
{
    BoardMgr &board = g_multyBoardMgr.getBoard(boardId);
    addr += board.spec.getTmpOffset();

    return I2CTmp75( i2c(), addr);
}

//chenbo add begin 20180108
I2CNT3H1X01 I2CBoardAccess::nt3h1x01(uint8_t addr) const
{
	addr = addr;
    return I2CNT3H1X01( i2c(), 1);
}
//chenbo add end

