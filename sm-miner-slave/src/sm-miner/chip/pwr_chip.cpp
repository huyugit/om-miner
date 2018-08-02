#include "pwr_chip.h"

#include "multy_board_mgr.h"
#include "master_gate.h"
#include "board_revisions.h"
#include "utils.h"


PwrChip::PwrChip()
    : onSlaveId(0), boardId(0), onBoardId(0),
      spiId(0), spiSeq(0),
      uniqConfigSeq(0),
      uniqConfigCrc(0),
      toMasterTxId(0),
      pwcStatSeqTx(0)
{
    spiData.found           = false;
    spiData.uniqConfigLoaded = false;
    spiData.pwcTest         = PWC_TEST_NA;

    spiData.memVendor       = 0;
    spiData.memConst        = 0;
    spiData.memUniq         = 0;
    spiData.memTotal        = 0;

    spiData.pwcStatTotal    = 0;
    spiData.pwcStatOk       = 0;
    spiData.pwcStatShift    = 0;

    testRes.clear();
}

PwrChipChain &PwrChip::getChain() const
{
    return g_multyBoardMgr.getChain(spiId);
}

void PwrChip::updateInfo(uint32_t seqTx, const PwcSharedData &psd)
{
    pwcStatSeqTx = seqTx;
    pwcSharedData = psd;
}

uint32_t PwrChip::getPwcMask() const
{
    BoardMgr &board = g_multyBoardMgr.getBoard(boardId);
    return BoardRevisions::getPwcMask(board.spec, spiSeq);
}

uint32_t PwrChip::getEN2Mask() const
{
    uint32_t mask = g_masterGate.masterData.slaveId;
    mask = (mask << 8) | onSlaveId;
    return mask;
}

uint32_t PwrChip::getPowerLineId() const
{
    BoardMgr &board = g_multyBoardMgr.getBoard(boardId);
    uint8_t pwrLen = board.spec.pwrLen;

    return (pwrLen > 0 ? onBoardId / pwrLen : 0);
}

PwcUniqConfig& PwrChip::getUniqConfig()
{
    static PwcUniqConfig uniqConfig;
    //BoardMgr &board = g_multyBoardMgr.getBoard(boardId);

    uniqConfig.spiSeq       = spiSeq;
    uniqConfig.en2Mask      = getEN2Mask();
    uniqConfig.btcMask      = getPwcMask();

    return uniqConfig;
}

void PwrChip::printPwcInfo()
{
    log("PWC[%d/%2d]: CFG:%d STAT: %4d/%4d (E: %2d%%) shift: %d SEQ:%4d MTX:%4d T:%3d CPU:%2d UART:%3d\n",
        spiId, spiSeq,
        spiData.uniqConfigLoaded,
        spiData.pwcStatOk, spiData.pwcStatTotal,
        calcPercent(spiData.pwcStatTotal - spiData.pwcStatOk, spiData.pwcStatTotal),
        spiData.pwcStatShift,
        pwcStatSeqTx,
        toMasterTxId,
        pwcSharedData.totalTime,
        pwcSharedData.cpuFreq,
        pwcSharedData.uartSpeed);
}

void PwrChip::printPwcSolInfo()
{
    log("PWC[%d/%2d]:  CFG:%d S/E: %5d/%5d FIX:%3d J:%5d T:%3d\n",
        spiId, spiSeq,
        spiData.uniqConfigLoaded,
        pwcSharedData.totalSolutions,
        pwcSharedData.totalErrors,
        pwcSharedData.totalFixes,
        pwcSharedData.totalJobsDone,
        pwcSharedData.totalTime,
        pwcSharedData.cpuFreq,
        pwcSharedData.uartSpeed);
}

void PwrChip::printPwcTestInfo()
{
    log("PWC[%d/%2d]: PWC=%u BTC=0x%08x\n",
        spiId, spiSeq, testRes.pwcResult, testRes.btcResults);
}
