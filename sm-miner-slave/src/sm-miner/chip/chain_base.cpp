#include "chain_base.h"

#include "stm_gpio.h"
#include "pwr_chip_rx_packet.h"


//FastSpiRelayBus g_spi;
PwrChipPacket g_packet;
PwrChipRxPacket g_rxPacket;


ChainBase::ChainBase()
    : spiId(0), spiLen(0),
      currentSeq(0),
      parritySchema(0)
{
}

void ChainBase::gotoFirstChip()
{
    resetLine();
    //log("RX STATUS = %d\n", g_spiExchange.rxStatus(0));
    sendBreak();
    //log("RX STATUS = %d\n", g_spiExchange.rxStatus(0));

    currentSeq = 0;
}

void ChainBase::gotoNextChip()
{
    sendNext( seqToAddr(currentSeq) );
    currentSeq++;
}

void ChainBase::gotoChip(uint8_t seq)
{
    gotoFirstChip();

    while (currentSeq < seq)
    {
        gotoNextChip();
    }
}

uint8_t ChainBase::seqToParity(uint32_t seq)
{
    switch (parritySchema)
    {
    case 0/*CHAIN_PARRITY_SCHEMA_123*/:  return (seq % 3) + 1; // 1, 2, 3, 1, 2, 3, ....
    case 1/*CHAIN_PARRITY_SCHEMA_0123*/: return (seq % 4) + 0; // 0, 1, 2, 3, 0, 1, 2, 3, ....
    }

    ASSERT(false, "wrong parritySchema");
    return 0;
}

uint8_t ChainBase::seqToAddr(uint32_t seq)
{
    return PwrChipPacket::A_0 + seqToParity(seq);
}

void ChainBase::sendBreak(uint8_t flags)
{
    g_packet.clear();
    g_packet.pushCmd(PwrChipPacket::CMD_BREAK | PwrChipPacket::A_ALL | flags);

    send(g_packet);
}

void ChainBase::sendNextLR(uint8_t flags)
{
    sendNext( PwrChipPacket::A_ALL | flags );
}

void ChainBase::sendNext(uint8_t flags)
{
    g_packet.clear();
    g_packet.pushCmd(PwrChipPacket::CMD_NEXT | flags);

    send(g_packet);
}

//-----------------------------------------------
// Read / Write functions
//-----------------------------------------------

PwcReadResult ChainBase::readReg(uint8_t flags, uint32_t regAddr, uint32_t &regValue)
{
    return read(flags, (void*)regAddr, sizeof(regValue), &regValue);
}

PwcReadResult ChainBase::read(uint8_t flags, void *virtualPtr, uint32_t size, void *dest)
{
    if (size % 4 != 0)
    {
        log("ERROR: can not read NOT alligned data, size = %d!\n", size);
        STOP();
    }

    uint32_t words = size / 4;

    g_packet.clear();
    g_packet.pushCmd(PwrChipPacket::CMD_READ | flags);
    g_packet.pushHeader(words, (uint32_t)virtualPtr);
    send(g_packet);

    SpiRxResult rxResult = receive(words*5);

    if (rxResult == SPI_RX_TIMEOUT) {
        return PWC_READ_TIMEOUT;
    }

    if (rxResult != SPI_RX_OK)
    {
//        if (rxResult == SPI_RX_FRAME_ERROR) {
//            log("SPI_RX_FRAME_ERROR: "); g_rxPacket.dump();
//        }
//        if (rxResult == SPI_RX_TIMEOUT) {
//            log("SPI_RX_TIMEOUT: "); g_rxPacket.dump();
//        }

        //log("read: fail: "); g_rxPacket.dump();
        return PWC_READ_ERROR;
    }


    if (!g_rxPacket.decode()) {
        log("read: can not decode!\n");
        return PWC_READ_ERROR;
    }
    if (g_rxPacket.getLength() != words*4) {
        log("read: wrong length %d, words %d!\n", g_rxPacket.getLength(), words);
        return PWC_READ_ERROR;
    }

    memcpy(dest, g_rxPacket.getBuffer(), g_rxPacket.getLength());
    return PWC_READ_OK;
}


bool ChainBase::writeReg(uint8_t flags, uint32_t regAddr, uint32_t regValue)
{
    // redirect to general static method
    return writeToSpi(spiId, flags, (void*)regAddr, sizeof(regValue), &regValue);
}

bool ChainBase::writeRegToSpi(uint8_t spiId, uint8_t flags, uint32_t regAddr, uint32_t regValue)
{
    // redirect to general static method
    return writeToSpi(spiId, flags, (void*)regAddr, sizeof(regValue), &regValue);
}

bool ChainBase::write(uint8_t flags, void *virtualPtr, uint32_t size, const void *src)
{
    // redirect to general static method
    return writeToSpi(spiId, flags, virtualPtr, size, src);
}

bool ChainBase::writeToSpi(uint8_t spiId, uint8_t flags, void *virtualPtr, uint32_t size, const void *src)
{
    //log("WRITE: size = %d\n", size);

    uint32_t addr = (uint32_t)virtualPtr;

    if (size % 4 != 0)
    {
        log("ERROR: can not write NOT alligned data, size = %d!\n", size);
        STOP();
    }

    uint32_t words = size / 4;

    g_packet.clear();
    g_packet.pushCmd(PwrChipPacket::CMD_WRITE | flags);
    g_packet.pushHeader(words, addr);

    uint32_t *src32 = (uint32_t*)src;
    for (uint32_t i = 0; i < words; i++, src32++)
    {
        g_packet.pushWData(*src32);
    }

    sendToSpi(spiId, g_packet);

    return true;
}

//-----------------------------------------------
// Low Level functions
//-----------------------------------------------

void ChainBase::resetLine()
{
    // redirect to general static method
    g_spiExchange.resetLine(spiId);
}

void ChainBase::resetLineToSpi(uint8_t spiId)
{
    //log("RESET LINE\n");
    g_spiExchange.resetLine(spiId);
}

void ChainBase::send(PwrChipPacket &packet)
{
    // redirect to general static method
    sendToSpi(spiId, packet);
}

void ChainBase::sendToSpi(uint8_t spiId, PwrChipPacket &packet)
{
    g_spiExchange.tx(spiId, packet.getBuffer(), packet.getLen());
}

SpiRxResult ChainBase::receive(uint32_t size)
{
    uint32_t received = 0;

    SpiRxResult res = g_spiExchange.rx(spiId, g_rxPacket.getBuffer(), size, received);
    g_rxPacket.setLength(received);

    return res;
}
