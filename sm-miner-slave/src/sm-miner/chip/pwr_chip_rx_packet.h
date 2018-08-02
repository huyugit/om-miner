#ifndef PWR_CHIP_RX_PACKET_H
#define PWR_CHIP_RX_PACKET_H

#include <stdint.h>
#include <stddef.h>
#include "common.h"


class PwrChipRxPacket
{
public:
    PwrChipRxPacket();

    inline uint8_t *getBuffer() { return buffer; }

    inline uint16_t getLength() const { return len; }
    inline void setLength(uint32_t newLen) { len = newLen; }

    bool decode();

    void dump() const;
    void dumpFull() const;

protected:
    static const uint32_t MAX_SIZE = 4*1024;

    uint32_t len;
    uint8_t buffer[MAX_SIZE];
};

#endif // PWR_CHIP_RX_PACKET_H
