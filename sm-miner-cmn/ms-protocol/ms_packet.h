#ifndef MSPACKET_H
#define MSPACKET_H

#include<stdint.h>


class MsHeader;
class MsFooter;

class MsPacket
{
public:
    static const uint16_t MAGIC     = 0xABCD;
    static const uint16_t VERSION   = 0x8161;
    static const uint16_t EOP       = 0xFFFF;

public:
    MsPacket(void *ptr, uint32_t size);

    template<class MsgT>
    bool pushMsg(const MsgT &msg)
    {
        return pushMsg(MsgT::MS_ID, &msg, sizeof(MsgT));
    }

    bool pushEnd();

    bool hasMsg();
    int getMsgId();
    void nextMsg();

    template<class MsgT>
    MsgT* getMsgPtrT()
    {
        return (MsgT*)getMsgPtr(MsgT::MS_ID, sizeof(MsgT));
    }

    void unexpectedMsg();

    void skipLeadingZero();

    void *ptrSrc;
    uint32_t sizeSrc;

private:
    void *ptr;
    uint32_t size;

    uint32_t calcCrc(uint8_t *begin, uint8_t *end);
    uint32_t calcCrc(const MsHeader &h);

    inline MsHeader& header() { return *(MsHeader*)ptr; }
    void moveForward(uint32_t sz);

    bool pushMsg(uint16_t msgId, const void *msgPtr, uint32_t msgSize);
    void* getMsgPtr(uint16_t msgId, uint32_t msgSize);

public:
    // master-slave spi test/debug
    static const uint32_t TEST_MARKER_SIZE = 8;

    void pushByte(uint8_t x);
    uint8_t popByte();

    bool isTest();

    void fillTestMaster();
    void fillTestSlave(MsPacket &packet);

    bool analyzeTest();
};


#define MS_DISPATCH_BEGIN(packet) \
    while (packet.hasMsg()) \
    { \
        if (packet.hasMsg() == false) return false; \
        if (packet.getMsgId() == MsPacket::EOP) return true; \
        \
        switch (packet.getMsgId()) \
        {

#define MS_DISPATCH_MSG(packet, MsgT) \
        case MsgT::MS_ID: { \
            MsgT *ptr = packet.getMsgPtrT<MsgT>(); \
            if (ptr) processMsg(*ptr); \
            break; \
        }

#define MS_DISPATCH_END(packet) \
        default: \
            packet.unexpectedMsg(); \
            break; \
        } \
        packet.nextMsg(); \
    } \
    return false;


class MsPacketUT
{
public:
    struct TestMsg1
    {
        static const uint8_t MS_ID = 0x01;
        uint32_t data;
        uint8_t dummyData[128];
    };

    struct TestMsg2
    {
        static const uint8_t MS_ID = 0x02;
        uint32_t data;
        uint8_t dummyData[128];
    };

    static void unitTest();

    void test1();
    void test2();

    void writePacket();
    bool readPacket();

    void processMsg(TestMsg1 &msg);
    void processMsg(TestMsg2 &msg);

private:
    uint8_t buffer[1024];
};

#endif // MSPACKET_H
