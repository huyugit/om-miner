#ifndef FORMAT_HPP
#define FORMAT_HPP

#include <cstdio>
#include <cstring>
#include <stdint.h>

#define log printf

inline void hexdump8(const uint8_t *ptr, uint32_t words)
{
    log("[%d]: ", words);
    for (uint32_t i = 0; i < words; ++i, ++ptr)
    {
        log("%02x ", *(ptr));
    }
    log("\n");
}

inline void hexdump(const void *ptr, uint32_t size)
{
    hexdump8((const uint8_t*)ptr, size);
}


#endif // FORMAT_HPP
