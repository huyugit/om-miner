#ifndef PWC_BLOCK_H
#define PWC_BLOCK_H

#include "cmn_defines.h"
#include "cmn_block.h"


struct PwcUniqConfig
{
    uint8_t  spiSeq;
    uint32_t en2Mask;
    uint16_t btcMask;

    uint8_t dummyAlignToInt32[1];

    PwcUniqConfig() {
        memset(this, 0, sizeof(PwcUniqConfig));
        btcMask = 0x7ff;
    }

    void dump()
    {
        log("PwcUniqConfig:\n");
        log("  spiSeq:      %d\n", spiSeq);
        log("  en2Mask:     0x%04x\n", en2Mask);
        log("  btcMask:     %d\n", btcMask);
    }
}
__attribute__ ((packed));


class PwcNonceArrayBlock
{
public:
    static const uint32_t MAX_LEN = 10;

    PwcNonceData nonces[MAX_LEN];
    uint32_t crc32[MAX_LEN];
    uint32_t txTime[MAX_LEN];

    void dump()
    {
        log("NonceArrayBlock:\n");
        for (uint32_t i = 0; i < MAX_LEN; i++)
        {
            log("  nonce[%d]: ", i); nonces[i].dump();
        }
    }
}
__attribute__ ((packed));


#endif // PWC_BLOCK_H
