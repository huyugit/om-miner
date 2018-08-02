#include "pwr_chip_packet.h"

#include "format.hpp"
#include "utils.h"


uint8_t PwrChipPacket::crc8(void *ptr, uint32_t len)
{
    const uint8_t *ptrByte = (uint8_t*)ptr;
    uint8_t crc = 0xff;

    while (len--)
    {
        crc ^= *ptrByte++;

        for (uint8_t i = 0; i < 8; i++)
            crc = crc & 0x80 ? (crc << 1) ^ 0x31 : crc << 1;
    }

    return crc;
}

uint8_t PwrChipPacket::condIf(uint8_t cond)
{
    return (cond + 1) << COND_SHIFT;
}

uint8_t PwrChipPacket::condIfNot(uint8_t cond)
{
    return condIf(cond) | COND_I;
}


PwrChipPacket::PwrChipPacket()
    : len(0)
{
}

void PwrChipPacket::clear()
{
    len = 0;
}

void PwrChipPacket::pushUInt8(uint8_t data)
{
    if (len + sizeof(data) > MAX_SIZE)
    {
        log("ERROR: can not push %d bytes into buffer, current buffer size is %d bytes!\n",
            sizeof(data), len);
        STOP();
    }

    *(uint8_t*)(buffer + len) = data;
    len += sizeof(data);
}

void PwrChipPacket::pushUInt16(uint16_t data)
{
    if (len + sizeof(data) > MAX_SIZE)
    {
        log("ERROR: can not push %d bytes into buffer, current buffer size is %d bytes!\n",
            sizeof(data), len);
        STOP();
    }

    *(uint16_t*)(buffer + len) = SwapEndian16( data );
    len += sizeof(data);
}

void PwrChipPacket::pushUInt32(uint32_t data)
{
    if (len + sizeof(data) > MAX_SIZE)
    {
        log("ERROR: can not push %d bytes into buffer, current buffer size is %d bytes!\n",
            sizeof(data), len);
        STOP();
    }

    *(uint32_t*)(buffer + len) = SwapEndian( data );
    len += sizeof(data);
}

void PwrChipPacket::initCrc()
{
    crcStartPosition = len;
}

void PwrChipPacket::pushCrc()
{
    pushUInt8( crc8(buffer+crcStartPosition, len-crcStartPosition) );
}

void PwrChipPacket::pushCmd(uint8_t cmd)
{
    initCrc();
    pushUInt8(cmd);
    pushCrc();
}

void PwrChipPacket::pushHeader(uint16_t length, uint32_t addr)
{
    if (addr & 0x3)
    {
        log("ERROR: can not create header: addr 0x%08x is not alligned!\n", addr);
        STOP();
    }

    initCrc();
    pushUInt16(length);
    pushUInt32(addr);
    pushCrc();
}

void PwrChipPacket::pushWData(uint32_t data)
{
    initCrc();
    pushUInt32(data);
    pushCrc();
}
