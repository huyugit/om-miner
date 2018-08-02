/*******************************************************************************
*  file    : format.cpp
*  created : 20.02.2013
*  author  : Slyshyk Oleksiy (alexSlyshyk@gmail.com)
*******************************************************************************/

#include <errno.h>
#include <sys/unistd.h>

#include <cstdarg>
#include <cstdlib>
#include <cstdio>
#include "format.hpp"
#include "uart/uart.hpp"
#include "mytime.h"


void log(const char* fmt, ... )
{
    uint8_t buffer[1024];
    va_list vArgs;

    va_start(vArgs, fmt);
    vsnprintf((char*)buffer, sizeof(buffer), fmt, vArgs);
    va_end(vArgs);

	sprintf((char*)buffer, "%s\r", (char*)buffer);
    uint32_t len = strlen((const char*)buffer);

    if (1)
    {
        // Using DMA (async)
        uartDebug.write(buffer, len);
        uartDebug2.write(buffer, len);
    }
    else
    {
        // Using polling (sync) - ONLY FOR DEBUG
        uartDebug.send_poll_data(buffer, len);
        uartDebug2.send_poll_data(buffer, len);
    }
}

void memdump(const void *ptr, uint32_t size)
{
    const uint8_t* p = (const uint8_t*)ptr;
    log("memory dump, addr = 0x%08x:\n", p);

    char s[32+1]; s[0]=0;
    for (uint32_t i = 0; i < size; i++)
    {
        char ch = *(p+i);
        int col = i % 32;

        if (col == 0) {
            if (i > 0) {
                log("[%s]\n", s);
            }
            log("0x%08x: ", p+i);
        }
        log("%02x ", *(p+i));
        s[col+0] = (ch >= 0x20 && ch < 0x80) ? ch : '.';
        s[col+1] = 0;
    }
    log("[%s]\n", s);
}

void memdump2(const void *ptr, uint32_t size)
{
    static const uint32_t space = 8;
    log("8 byte before: "); memdump(ptr-space, space);
    memdump(ptr, size);
    log("8 byte after: "); memdump(ptr+size, space);
}


void hexdump(const void *ptr, uint32_t size)
{
    hexdump8((const uint8_t*)ptr, size);
}

void hexdump8(const uint8_t *ptr, uint32_t words)
{
    log("[%d]: ", words);
    for (uint32_t i = 0; i < words; ++i, ++ptr)
    {
        log("%02x ", *(ptr));
    }
    log("\n");
}

void hexdump32(const uint32_t *ptr, uint32_t words)
{
    log("[%d]: ", words);
    for (uint32_t i = 0; i < words; ++i, ++ptr)
    {
        log("%08x ", *(ptr));
    }
    log("\n");
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

    log("[%d]: ", words);
    for (uint32_t i = 0; i < len; ++i, ++ptr)
    {
        log("%02x ", *(ptr));
    }
    log("... ");
    for (uint32_t i = 0; i < len; ++i, ++ptrEnd)
    {
        log("%02x ", *(ptrEnd));
    }
    log("\n");
}


extern "C" void assert_failed(const char* file, uint32_t line)
{
    log("Assert in file %s:%i\n", file, line);
    //STOP();
}


