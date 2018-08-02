#ifndef PWR_CHIP_PACKET_H
#define PWR_CHIP_PACKET_H

#include <stdint.h>


class PwrChipPacket
{
public:
    // Command
    // | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
    // | i |  op   | cond  |   addr    |

    static const uint8_t CMD_BREAK        = 0 << 5;
    static const uint8_t CMD_NEXT         = 1 << 5;
    static const uint8_t CMD_WRITE        = 2 << 5;
    static const uint8_t CMD_READ         = 3 << 5;

    static const uint8_t COND_I           = 1 << 7;
    static const uint8_t COND_SHIFT       = 3;

    static const uint8_t A_NONE           = 0;
    static const uint8_t A_0              = 1;
    static const uint8_t A_1              = 2;
    static const uint8_t A_2              = 3;
    static const uint8_t A_3              = 4;
    static const uint8_t A_ALL            = 7;


    static const uint32_t ADDR_SPI_COND_FLAGS       = 0xD0000000;
    static const uint32_t ADDR_SPI_COND_FLAGS_SET   = 0xD000000c;
    static const uint32_t ADDR_SPI_COND_FLAGS_CLEAR = 0xD0000010;

    static const uint32_t ADDR_SYS_RESET        = 0xE0000000;
    static const uint32_t ADDR_CPU_ENABLE       = 0xE0000004;
    static const uint32_t ADDR_SYS_CLK_CFG      = 0xE0000008;
    static const uint32_t ADDR_SPI_CFG          = 0xE0000010;
    static const uint32_t ADDR_SHUTDOWN_PIN     = 0xE0000014;
    static const uint32_t ADDR_CHIP_VENDOR_ID   = 0xE0000080;

    static const uint32_t CHIP_RESET_MAGIC      = 0xAABBCCDD;

    static const uint32_t SPI_CFG_FWD_LATCHED_UP        = 0x1;
    static const uint32_t SPI_CFG_FWD_LATCHED_DOWN      = 0x2;
    static const uint32_t SPI_CFG_FWD_LATCHED           = 0x3;

    static const uint32_t SPI_CFG_FWD_DISABLE_UP        = 0x4;
    static const uint32_t SPI_CFG_FWD_DISABLE_DOWN      = 0x8;
    static const uint32_t SPI_CFG_FWD_DISABLE           = 0xC;

    static uint8_t crc8(void *ptr, uint32_t len);
    static uint8_t condIf(uint8_t cond);
    static uint8_t condIfNot(uint8_t cond);

public:
    PwrChipPacket();

    void clear();

    uint32_t getLen() { return len; }
    uint8_t* getBuffer() { return buffer; }

    void pushUInt8(uint8_t data);
    void pushUInt16(uint16_t data);
    void pushUInt32(uint32_t data);

    void initCrc();
    void pushCrc();

    void pushCmd(uint8_t cmd);
    void pushHeader(uint16_t length, uint32_t addr);
    void pushWData(uint32_t data);

private:
    static const uint32_t MAX_SIZE = 4*1024;

    uint32_t len;
    uint8_t buffer[MAX_SIZE];

    uint32_t crcStartPosition;
};

#endif // PWR_CHIP_PACKET_H
