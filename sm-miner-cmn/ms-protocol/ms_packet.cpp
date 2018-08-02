#include "ms_packet.h"
#include "format.hpp"


struct MsFooter
{
    uint32_t crc;
}
__attribute__ ((packed));

struct MsHeader
{
    uint16_t magic;
    uint16_t version;
    uint16_t msgId;
    uint16_t msgSize;

    uint32_t getFullSize() {
        return sizeof(MsHeader) + msgSize + sizeof(MsFooter);
    }

    inline MsFooter& footer() {
        return *(MsFooter*)((uint8_t *)this + sizeof(MsHeader) + msgSize);
    }

    void dump() {
        log("magic=0x%04x, ver=0x%04x, msgId=0x%02x, msgSize=%u\n", magic, version, msgId, msgSize);
    }
}
__attribute__ ((packed));


uint32_t MsPacket::calcCrc(uint8_t *begin, uint8_t *end)
{
    uint32_t crc = 0;
    while (begin < end)
    {
        uint8_t d = *begin;
        d = ((d >> 3) | (d << 5)) + 0x11;

        crc = ((crc >> 24) | (crc << 8));
        crc ^= d;

        begin++;
    }
    return crc;
}

uint32_t MsPacket::calcCrc(const MsHeader &h)
{
    uint8_t *p = (uint8_t*)&h;
    return calcCrc(p, p + sizeof(MsHeader) + h.msgSize);
}


MsPacket::MsPacket(void *_ptr, uint32_t _size)
    : ptrSrc(_ptr), sizeSrc(_size),
      ptr(_ptr), size(_size)
{}

bool MsPacket::pushMsg(uint16_t msgId, const void *msgPtr, uint32_t msgSize)
{
    uint32_t sz = sizeof(MsHeader) + msgSize + sizeof(MsFooter);
    if (sz > size)
    {
        log("MsPacket::pushMsg: not enough space, msgId=%u, msgSize=%u, req=%u, free=%u\n",
            msgId, msgSize, sz, size);
        return false;
    }

    MsHeader &h = *(MsHeader*)ptr;
    h.magic     = MAGIC;
    h.version   = VERSION;
    h.msgId     = msgId;
    h.msgSize   = msgSize;

    moveForward(sizeof(MsHeader));

    memcpy(ptr, msgPtr, msgSize);
    moveForward(msgSize);

    h.footer().crc = calcCrc(h);
    moveForward(sizeof(MsFooter));

    return true;
}

bool MsPacket::pushEnd()
{
    return pushMsg(EOP, nullptr, 0);
}


bool MsPacket::hasMsg()
{
    // search magic
    while (1)
    {
        if (sizeof(MsHeader) > size) {
            //log("MsPacket: magic not found\n");
            return false;
        }

        if (header().magic == MAGIC) {
            break;
        }

        moveForward(1);
    }

    if (header().version != VERSION) {
        log("MsPacket: version mismatch: act 0x%04x != exp 0x%04x\n",
            header().version, VERSION);
        return false;
    }

    if (header().getFullSize() > size)
        return false;

    MsFooter &f = *(MsFooter*)((uint8_t *)ptr + sizeof(MsHeader) + header().msgSize);
    uint32_t actCrc = calcCrc(header());
    if (f.crc != actCrc)
    {
        log("MsPacket: crc mismatch: exp 0x%08x != act 0x%08x\n", f.crc, actCrc);
        header().dump();
        return false;
    }

    return true;
}

int MsPacket::getMsgId()
{
    return header().msgId;
}

void MsPacket::nextMsg()
{
    moveForward(sizeof(MsHeader) + header().msgSize + sizeof(MsFooter));
}

void MsPacket::unexpectedMsg()
{
//    log("MsPacket: unexpected msgId=%u\n", header().msgId);
//    log("MsPacket: "); header().dump();
}

void MsPacket::skipLeadingZero()
{
    uint8_t *pb = (uint8_t*)ptr;
    if ((size > 0) && pb && (*pb == 0)) {
        ptr += 1;
        size--;
    }
}

void MsPacket::moveForward(uint32_t sz)
{
    if (sz <= size)
    {
        ptr += sz;
        size -= sz;
    }
    else {
        log("MsPacket::moveForward: ERROR: sz (%u) > size (%u)!\n",
            sz, size);
    }
}

void* MsPacket::getMsgPtr(uint16_t msgId, uint32_t msgSize)
{
    if (header().msgId != msgId) {
        log("MsPacket::getMsgPtr: ERROR: msgId mismatch!\n");
        return nullptr;
    }
    if (header().msgSize != msgSize) {
        log("MsPacket::getMsgPtr: ERROR: msgSize mismatch, msgId=0x%02x, msgSize=%u, exp=%u!\n", msgId,
            header().msgSize, msgSize);

        return nullptr;
    }
    return (uint8_t *)ptr + sizeof(MsHeader);
}

void MsPacket::pushByte(uint8_t x)
{
    if (size > 0)
    {
        *(uint8_t*)ptr = x;
        moveForward(1);
    }
}

uint8_t MsPacket::popByte()
{
    uint8_t result = 0;
    if (size > 0)
    {
        result = *(uint8_t*)ptr;
        moveForward(1);
    }
    return result;
}

bool MsPacket::isTest()
{
    if (size < TEST_MARKER_SIZE) {
        return false;
    }

    uint32_t n = 0;
    for (uint32_t i = 0; i < TEST_MARKER_SIZE; i++)
    {
        uint8_t b = *((uint8_t*)ptr + i);
        if (b == 0xAA || b == 0x55)
        {
            n++;
        }
    }

    return (n >= TEST_MARKER_SIZE / 2);
}

void MsPacket::fillTestMaster()
{
    for (uint32_t i = 0; i < TEST_MARKER_SIZE; i++)
    {
        pushByte(0x55);
    }

    for (uint8_t b = 0; size > 0; b++)
    {
        pushByte(b);
    }
}

void MsPacket::fillTestSlave(MsPacket &packet)
{
    // copy test marker
    for (uint32_t i = 0; i < TEST_MARKER_SIZE; i++)
    {
        pushByte(packet.popByte());
    }

    for (uint8_t b = 0; size > 0; b++)
    {
        pushByte(b);
        pushByte(packet.popByte());
    }
}

bool MsPacket::analyzeTest()
{
    bool result = true;

    for (uint32_t i = 0; i < TEST_MARKER_SIZE; i++)
    {
        uint8_t b = popByte();
        if (b != 0x55)
        {
            log("MS TEST: marker error: pos=%d, b=0x%02x\n", i, b);
            result = false;
        }
    }

    uint8_t prev1 = 0xff;
    uint8_t prev2 = 0xff;

    int num1 = 0;
    int num2 = 0;

    for (uint32_t i = 0; size > 0; i++)
    {
        uint8_t b1 = popByte();
        uint8_t b2 = popByte();

        if (i > 0){
            bool e1 = (b1 != uint8_t(prev1 + 1));
            bool e2 = (b2 != uint8_t(prev2 + 1));

            if (e1) num1++;
            if (e2) num2++;

            if (e1 || e2)
            {
                log("MS TEST: data error: pos=%d, e1/e2=%u/%u, data ", i, e1, e2);
                hexdump8((uint8_t*)ptr - 6, 4 + 2 + 4);

                result = false;
            }
        }

        prev1 = b1;
        prev2 = b2;
    }

    if (result) {
        log("MS TEST: %u bytes: OK\n", sizeSrc);
    }
    else {
        log("MS TEST: %u bytes: ERRORS: %d/%d\n", sizeSrc, num1, num2);
    }

    return result;
}


void MsPacketUT::unitTest()
{
    log("MsPacketUT::unitTest\n");
    MsPacketUT ut;
    ut.test1();
    ut.test2();
}

void MsPacketUT::test1()
{
    log("----------------------------------------\n");
    log("TEST1: write / read\n");

    writePacket();
    readPacket();
}

void MsPacketUT::test2()
{
    log("----------------------------------------\n");
    log("TEST2: write / broken data / read\n");

    writePacket();

    buffer[500] ^= 0x55;

    readPacket();
}

void MsPacketUT::writePacket()
{
    log("--- writePacket\n");
    MsPacket packet(buffer, sizeof(buffer));

    for (int i = 0; i < 5; i++)
    {
        TestMsg1 msg1;
        msg1.data = i;

        log("push TestMsg1: data = %u\n", msg1.data);
        packet.pushMsg(msg1);

        TestMsg1 msg2;
        msg2.data = i;

        log("push TestMsg2: data = %u\n", msg2.data);
        packet.pushMsg(msg2);

        packet.pushEnd();
    }
}

bool MsPacketUT::readPacket()
{
    log("--- readPacket\n");
    MsPacket packet(buffer, sizeof(buffer));

    MS_DISPATCH_BEGIN(packet)
    MS_DISPATCH_MSG(packet, TestMsg1)
    MS_DISPATCH_MSG(packet, TestMsg2)
    MS_DISPATCH_END(packet)
}

void MsPacketUT::processMsg(MsPacketUT::TestMsg1 &msg)
{
    log("processMsg(TestMsg1): data = %u\n", msg.data);
}

void MsPacketUT::processMsg(MsPacketUT::TestMsg2 &msg)
{
    log("processMsg(TestMsg2): data = %u\n", msg.data);
}
