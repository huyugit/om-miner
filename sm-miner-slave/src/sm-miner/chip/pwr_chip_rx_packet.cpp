#include "pwr_chip_rx_packet.h"

#include <cstring>
#include "format.hpp"
#include "pwr_chip_packet.h"
#include "utils.h"


PwrChipRxPacket::PwrChipRxPacket()
    : len(0)
{}

bool PwrChipRxPacket::decode()
{
    // check and convert (data32 + crc8) into (data32)

    if (len % 5 != 0)
    {
        log("PwrChipRxPacket: ERROR: packet is not aligned, len = %d, len mod 5 = %d\n", len, len % 5);
        return false;
    }

    uint32_t pos = 0;
    uint32_t pos2 = 0;

    while (pos + 5 <= len)
    {
        uint32_t data = *(uint32_t*)(buffer+pos);

        uint8_t calcedCrc = PwrChipPacket::crc8(buffer+pos, 4);
        uint8_t packetCrc = *(uint8_t*)(buffer+pos+4);

        if (calcedCrc != packetCrc)
        {
            log("PwrChipRxPacket: WRONG CRC: pos %d, data 0x%08x, calcedCrc 0x%02x != packetCrc 0x%02x\n", pos, data, calcedCrc, packetCrc);
            return false;
        }

        *(uint32_t*)(buffer+pos2) = SwapEndian( data );

        pos += 5;
        pos2 += 4;
    }

    len = pos2;
    return true;
}

void PwrChipRxPacket::dump() const
{
    log("PwrChipRxPacket: "); hexdumpBeginEnd(buffer, len);
}

void PwrChipRxPacket::dumpFull() const
{
    log("PwrChipRxPacket: "); hexdump32((uint32_t*)buffer, len/4);
}
