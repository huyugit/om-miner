#include "MiscUtil.h"

#include "base/BaseUtil.h"
#include "base/MinMax.h"

#include "old/cByteBufferType.h"

#include <assert.h>
#include <string.h>


namespace util
{

uint32_t hexStrToInt32(const char* hexStr)
{
    assert(strlen(hexStr) == 8);
    return swapEndian( *(uint32_t*)(cByteBufferType(hexStr).GetBuffer()) );
}

void hexdump(const void *ptr, uint32_t size)
{
    hexdump8((const uint8_t*)ptr, size);
}

void hexdump8(const uint8_t *ptr, uint32_t words)
{
    printf("[%d]: ", words);
    for (uint32_t i = 0; i < words; ++i, ++ptr)
    {
        printf("%02x ", *(ptr));
    }
    printf("\n");
}

void hexdump32(const uint32_t *ptr, uint32_t words)
{
    printf("[%d]: ", words);
    for (uint32_t i = 0; i < words; ++i, ++ptr)
    {
        printf("%08x ", *(ptr));
    }
    printf("\n");
}

void hexdumpBeginEnd(const uint8_t *ptr, uint32_t words)
{
    static const uint32_t len = 8;

    if (words <= 2 * len)
    {
        hexdump8(ptr, words);
        return;
    }

    const uint8_t *ptrEnd = ptr + words - len;

    printf("[%d]: ", words);
    for (uint32_t i = 0; i < len; ++i, ++ptr)
    {
        printf("%02x ", *(ptr));
    }
    printf("... ");
    for (uint32_t i = 0; i < len; ++i, ++ptrEnd)
    {
        printf("%02x ", *(ptrEnd));
    }
    printf("\n");
}

//  todo: cByteBufferType deprecated
size_t hexToData(const char* hexStr, void* buf, size_t bufSize)
{
    assert(hexStr != nullptr);
    assert(buf != nullptr);
    
    return cByteBufferType::HexToBuff(hexStr, reinterpret_cast<uint8_t*>(buf), bufSize);
}

size_t dataToHex(char* hexStr, const void* data, size_t len,
    bool hexCapital /*= false*/, bool spaceSeparated /*= false*/,
    size_t bytesPerLine /*= 0*/, const char* lineSeparator /*= "\n"*/)
{
    assert(hexStr != nullptr);
    assert(data != nullptr);
    
    const char* const syms = hexCapital ? "0123456789ABCDEF" : "0123456789abcdef";
    const size_t lineSeparatorLen = (lineSeparator != nullptr) ?
        ::strlen(lineSeparator) : 0u;

    size_t strLen = 0;
    for (size_t i = 0; i < len; ++i)
    {
        if (i > 0)
        {
            if (bytesPerLine > 0 && i % bytesPerLine == 0)
            {
                for (size_t j = 0; j < lineSeparatorLen; ++j)
                    hexStr[strLen++] = lineSeparator[j];
            }
            else if (spaceSeparated)
            {
                hexStr[strLen++] = ' ';
            }
        }

        hexStr[strLen++] = syms[(reinterpret_cast<const uint8_t*>(data)[i] >> 4) & 0xf];
        hexStr[strLen++] = syms[reinterpret_cast<const uint8_t*>(data)[i] & 0xf];
    }
    
    hexStr[strLen] = '\0';
    return strLen;
}

}  // end of namespace util
