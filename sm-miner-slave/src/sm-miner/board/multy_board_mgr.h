#ifndef MULTY_CHIP_BOARD_H
#define MULTY_CHIP_BOARD_H

#include <stdint.h>
#include "board_mgr.h"
#include "pwr_chip.h"
#include "pwr_chip_chain.h"
#include "stm_multi_spi.h"

class MultyBoardMgr
{
public:
    MultyBoardMgr();

    BoardMgr& getBoard(int boardId);
    inline PwrChipChain& getChain(uint8_t spi) { return chains[spi]; }

    PwrChip& getChip(uint32_t index);
    PwrChip& getBoardChip(uint8_t boardId, uint32_t index);
    PwrChip& getSpiGridChip(uint8_t boardId, uint8_t spiLine, uint8_t spiSeq);
    PwrChip& getPwrGridChip(uint8_t boardId, uint8_t pwrLine, uint8_t pwrSeq);

    void configureBoardsAuto();
    void configureBoardsBySpec(uint8_t slotMask, const BoardSpec &spec);
    void reconfigureGrid();

    void activateCan();
    void updateBoardsInfo();
    void processPwrSw();

	void setOSCconfig();  //chenbo add 20180108

    void runLoop();

    void manageOcp();
    void writeOcp();
    void writeTmpAlert();

    void printInfo();
    void printSummaryInfo();
    void printAllChipsInfo();
    void printAllChipsSolInfo();
    void printAllChipsTestInfo();
    void printSystemInfo();

    void testPowerSwitch();

    void minerRun();
	int8_t getHSBMaxTemp(void);

    uint32_t numChips;

    uint8_t spiNum, spiLen;
    uint32_t spiMask;

    uint32_t cmnConfigBroadcasts;
    uint32_t miningDataBroadcasts;

    uint32_t totalSolutions;
    uint32_t totalSolutionsByDiff;
    uint32_t totalJobsDone;
	uint32_t hashBoardPowerDownFlag;

	int8_t maxHSBMaxTemp;

protected:
    BoardMgr boards[MAX_BOARD_PER_SLAVE];
    uint32_t boardStartIndex[MAX_BOARD_PER_SLAVE];

    PwrChipChain chains[MAX_SPI_PER_SLAVE];

    PwrChip chips[MAX_PWC_PER_SLAVE];
};


extern MultyBoardMgr g_multyBoardMgr;

#endif // MULTY_CHIP_BOARD_H
