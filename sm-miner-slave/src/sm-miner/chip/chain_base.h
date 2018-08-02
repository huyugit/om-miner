#ifndef CHAIN_BASE_H
#define CHAIN_BASE_H

#include "pwr_chip_packet.h"
#include "stm_multi_spi.h"


enum PwcReadResult
{
    PWC_READ_OK             = 0,
    PWC_READ_TIMEOUT        = 1,
    PWC_READ_ERROR          = 2,
};


class ChainBase
{
public:
    uint8_t spiId, spiLen;
    uint8_t currentSeq;
    uint8_t parritySchema;

protected:
    ChainBase();

    //--- Chain Iteration functions

    void gotoFirstChip();
    void gotoNextChip();

    void gotoChip(uint8_t seq);

    //--- Chain Management functions

    uint8_t seqToParity(uint32_t seq);
    uint8_t seqToAddr(uint32_t seq);

    void sendBreak(uint8_t flags = 0);

    void sendNextLR(uint8_t flags = 0);
    void sendNext(uint8_t flags = 0);

    //--- Read / Write functions

    PwcReadResult readReg(uint8_t flags, uint32_t regAddr, uint32_t &regValue);
    PwcReadResult read(uint8_t flags, void *virtualPtr, uint32_t size, void *dest);

    bool writeReg(uint8_t flags, uint32_t regAddr, uint32_t regValue);
    static bool writeRegToSpi(uint8_t spiId, uint8_t flags, uint32_t regAddr, uint32_t regValue);

    bool write(uint8_t flags, void *virtualPtr, uint32_t size, const void *src);
    static bool writeToSpi(uint8_t spiId, uint8_t flags, void *virtualPtr, uint32_t size, const void *src);

    //--- Low Level functions

    void resetLine();
    static void resetLineToSpi(uint8_t spiId);

    void send(PwrChipPacket& packet);
    static void sendToSpi(uint8_t spiId, PwrChipPacket& packet);

    SpiRxResult receive(uint32_t size);
};

#endif // CHAIN_BASE_H
