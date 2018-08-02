#ifndef MISC_UTIL_H
#define MISC_UTIL_H
/*
 * Declares a number of various utility functions for different purpose.
 */

#include <stdlib.h>
#include <stdint.h>


namespace util
{
    // Convert hex string "aabbccdd" to int 0xaabbccdd
    uint32_t hexStrToInt32(const char* hexStr);

    void hexdump(const void *ptr, uint32_t size);

    void hexdump8(const uint8_t *ptr, uint32_t words);
    void hexdump32(const uint32_t *ptr, uint32_t words);

    void hexdumpBeginEnd(const uint8_t *ptr, uint32_t words);

    // Converts hex-encoded data (str) into binary and writes
    // into the given buffer (buf). Returns the number of bytes
    // written that will not exceed the given limit (bufSize).
    size_t hexToData(const char* hexStr, void* buf, size_t bufSize);

    // Converts binary data (data) into the hex string (buf).
    // 
    // data - the binary data to convert.
    // len - the length of the binary data.
    // hexStr - the output string buffer. Should be enough size.
    // to receive the converted string.
    //
    // hexCapital - whether to use hexadecimal characters in the upper case.
    // spaceSeparated - whether to separate bytes in the hex string
    // representation with spaces (default false).
    // bytesPerLine - a number of bytes per line
    // (0 - do not insert line separators, default).
    // lineSeparator - line separator (used if bytesPerLine > 0).
    size_t dataToHex(char* hexStr, const void* data, size_t len,
        bool hexCapital = false, bool spaceSeparated = false,
        size_t bytesPerLine = 0, const char* lineSeparator = "\n");

}  // End of namespace util

#endif  // MISC_UTIL_H
