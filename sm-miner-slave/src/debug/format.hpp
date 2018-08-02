/*******************************************************************************
*  file    : format.hpp
*  created : 20.02.2013
*  author  : Slyshyk Oleksiy (alexSlyshyk@gmail.com)
*******************************************************************************/

#ifndef FORMAT_HPP
#define FORMAT_HPP

#include <stdint.h>
#include <string.h>

#define tp() log("%s: %d\n", __FILE__, __LINE__)
#define STOP() { log("STOP-DEBUG at %s: %d\n", __FILE__, __LINE__); while(1); }

#define ASSERT(f, msg) { if (!(f)) { log("ASSERT: %s\n", msg); STOP(); } }

#define LOG_ERROR(...) { log("ERROR: "); log(__VA_ARGS__); log("\n"); }
#define LOG_INFO(...) { log("INFO: "); log(__VA_ARGS__); log("\n"); }

void log(const char* fmt, ... );

void memdump(const void *ptr, uint32_t size);
void memdump2(const void *ptr, uint32_t size);

void hexdump(const void *ptr, uint32_t size);

void hexdump8(const uint8_t *ptr, uint32_t words);
void hexdump32(const uint32_t *ptr, uint32_t words);

void hexdumpBeginEnd(const uint8_t *ptr, uint32_t words);

#endif // FORMAT_HPP
