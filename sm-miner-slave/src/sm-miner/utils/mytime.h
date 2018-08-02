#ifndef TIME_H
#define TIME_H

#include <stdint.h>

// get current time in mili-seconds since start
uint32_t getMiliSeconds();

// get current time in seconds since start
uint32_t getSeconds();

// sleep for specified number of mili-seconds
void mysleep(uint32_t msec);

#endif // TIME_H
