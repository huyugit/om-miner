#include "static_allocator.h"

#include "format.hpp"

uint8_t staticAllocatorBuffer[16*1024];
StaticAllocator g_staticAllocator(staticAllocatorBuffer, sizeof(staticAllocatorBuffer));

StaticAllocator g_staticAllocatorCCM((uint8_t*)0x10000000, 64*1024);


StaticAllocator::StaticAllocator(uint8_t* _bufferPtr, uint32_t _bufferSize)
    : bufferPtr(_bufferPtr), bufferSize(_bufferSize), total(0)
{}

uint8_t* StaticAllocator::alloc(uint32_t size)
{
    uint8_t *result = (uint8_t*)bufferPtr + total;

    total += size;

    return result;
}

void StaticAllocator::dump()
{
    log("StaticAllocator[%p]: allocated %u / %u\n",
        bufferPtr, total, bufferSize);
}
