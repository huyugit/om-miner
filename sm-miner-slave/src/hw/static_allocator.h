#ifndef STATIC_ALLOCATOR_H
#define STATIC_ALLOCATOR_H

#include <stdint.h>

class StaticAllocator
{
public:
    StaticAllocator(uint8_t* _base, uint32_t _bufferSize);

    uint8_t* alloc(uint32_t size);
    void dump();

private:
    const uint8_t* bufferPtr;
    const uint32_t bufferSize;

    uint32_t total;
};

extern StaticAllocator g_staticAllocator;
extern StaticAllocator g_staticAllocatorCCM;

#endif // STATIC_ALLOCATOR_H
