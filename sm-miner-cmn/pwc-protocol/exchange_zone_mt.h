#ifndef EXCHANGE_ZONE_MT_H
#define EXCHANGE_ZONE_MT_H

#include <stdint.h>


struct ExchangeZoneMt
{
    static const uint32_t ADDR          = 0x00000020;

    static const uint32_t MARKER_DONE   = 0x55555555;

    static const uint32_t RESULT_OK     = 0x00112233;
    static const uint32_t RESULT_ERR    = 0x0BAD0BAD;


    //-------------------------------------------
    // pwc => host
    //-------------------------------------------

    uint32_t result;
    uint32_t counter;
    uint32_t errWords;
    uint32_t errBits;
    uint32_t marker;
    uint32_t crc32;

    //-------------------------------------------
    // methods
    //-------------------------------------------

    void setDone(int code) {
        result  = code;
        marker  = MARKER_DONE;
        crc32   = calcCrc32();
    }

    void done(int code)
    {
        setDone(code);
        while (1) {}
    }

    uint32_t calcCrc32()
    {
        return calcCrc32(this, sizeof(ExchangeZoneMt) - sizeof(crc32));
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
}
__attribute__ ((packed));


#endif // EXCHANGE_ZONE_MT_H
