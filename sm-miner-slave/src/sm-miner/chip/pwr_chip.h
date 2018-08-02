#ifndef PWR_CHIP_H
#define PWR_CHIP_H

#include "mytime.h"
#include "cmn_block.h"
#include "pwc_block.h"
#include "ms_data.h"


class PwrChipChain;

class PwrChip
{
public:
    PwrChip();

    PwrChipChain& getChain() const;

    void updateInfo(uint32_t seqTx, const PwcSharedData &psd);

    uint32_t getPwcMask() const;
    uint32_t getEN2Mask() const;
    uint32_t getPowerLineId() const;
    PwcUniqConfig& getUniqConfig();

    void printPwcInfo();
    void printPwcSolInfo();
    void printPwcTestInfo();

public:
    uint8_t onSlaveId;

    uint8_t boardId;
    uint8_t onBoardId;

    uint8_t spiId, spiSeq;
    uint8_t pwrId, pwrSeq;

    PwcSpiData spiData;

    uint32_t uniqConfigSeq;
    uint32_t uniqConfigCrc;

    uint32_t toMasterTxId;

    uint32_t pwcStatSeqTx;
    PwcSharedData pwcSharedData;

    PwcQuickTestRes testRes;
};

#endif // PWR_CHIP_H
