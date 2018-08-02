#ifndef EXCHANGE_ZONE_H
#define EXCHANGE_ZONE_H

#include "cmn_defines.h"
#include "cmn_block.h"
#include "pwc_block.h"


/* Explanation of host <=> pwc data exchange.
 * Possible cases:
 *
 * 1) broadcasted data (atomic exchange).
 * Simple case for data structures of <= 4 bytes.
 *
 * 2) broadcasted data (crc protection).
 * Master sends data and crc periodically. Slave detects crc change,
 * copies data into internal buffer and calculates crc. On crc
 * mismatch it makes 100ms pause. Master must not change data too
 * frequently.
 *
 * 3) sequence protected data
 * Master periodically writes block, sequence, crc. Slave detects
 * change in seq number, calculates crc. If data is ok, it sets
 * RX seq to TX seq.
 */

struct EzSeqRegs {
    uint32_t tx;
    uint32_t rx;

    void dump() {
        log("tx/rx=%d/%d\n", tx, rx);
    }
}
__attribute__ ((packed));


struct EzCrcRegs {
    uint32_t crc32;

    void dump() {
        log("crc32 = 0x%08x\n", crc32);
    }
}
__attribute__ ((packed));


struct ExchangeZone
{
    //-------------------------------------------
    // exchange coounter (pwc => host)
    //-------------------------------------------

    uint32_t    cnt;
    uint32_t    par;
    uint32_t    err;

    //-------------------------------------------
    // host => pwc
    //-------------------------------------------

    // broadcasted data (atomic exchange)
    uint32_t            hostTimeMs;

    // broadcasted data (crc protection)
    EzSeqRegs           ccSeq;
    EzCrcRegs           ccCrc;
    PwcCmnConfig        cmnConfig;

    // individual data (crc protection)
    EzSeqRegs           ucSeq;
    EzCrcRegs           ucCrc;
    PwcUniqConfig       uniqConfig;

    // broadcasted data (crc protection)
    EzSeqRegs           mdSeq;
    EzCrcRegs           mdCrc;
    PwcMiningData       miningData;

    //-------------------------------------------
    // pwc => host
    //-------------------------------------------

    // sequence protected data
    EzSeqRegs           pwcSeq;
    EzCrcRegs           pwcCrc;
    PwcBlock            pwcData;

    // per item protected data
    PwcNonceArrayBlock  nonces;

    //-------------------------------------------
    // methods
    //-------------------------------------------

    static bool validateBlock(const char *name, uint32_t size)
    {
        const uint32_t tail = (size & 0x03);
        if (tail != 0) {
            log("%s is not alligned: size = %d, tail = %d\n", name, size, tail);
            return false;
        }
        else {
            return true;
        }
    }

    static bool validate()
    {
        bool result = true;

        if (!validateBlock("ExchangeZone", sizeof(ExchangeZone)))
            result = false;
        if (!validateBlock("PwcCmnConfig", sizeof(PwcCmnConfig)))
            result = false;
        if (!validateBlock("PwcUniqConfig", sizeof(PwcUniqConfig)))
            result = false;
        if (!validateBlock("PwcMiningDataBlock", sizeof(PwcMiningData)))
            result = false;
        if (!validateBlock("PwcBlock", sizeof(PwcBlock)))
            result = false;
        if (!validateBlock("PwcTestBlock", sizeof(PwcTestBlock)))
            result = false;
        if (!validateBlock("PwcNonceArray", sizeof(PwcNonceArrayBlock)))
            result = false;

        return result;
    }

    static uint32_t calcCrc32(void *ptr, uint32_t size)
    {
        uint32_t words = size / 4;
        uint32_t *ptr32 = (uint32_t*)ptr;

        uint32_t result = 0x98A79B43;
        for (uint32_t i = 0; i < words; i++, ptr32++)
        {
            result ^= *ptr32;
            result = (result << 1) | (result & 0x80000000 ? 1 : 0);
        }

        return result;
    }

    void dump()
    {
        log("ExchangeZone (size %d bytes):\n", sizeof(ExchangeZone));
        log("cnt:           %d\n", cnt);
    }
}
__attribute__ ((packed));


#endif // EXCHANGE_ZONE_H
