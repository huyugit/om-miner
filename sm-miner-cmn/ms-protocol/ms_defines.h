#ifndef MS_DEFINES_H
#define MS_DEFINES_H

#include <stdint.h>
#include "cmn_block.h"


#define MAX_SLAVE_COUNT          4
#define MAX_BOARD_PER_SLAVE      6
#define MAX_BOARD_PER_MASTER     (MAX_BOARD_PER_SLAVE * MAX_SLAVE_COUNT)
#define MAX_SPI_PER_BOARD        1
#define MAX_SPI_PER_SLAVE        (MAX_SPI_PER_BOARD * MAX_BOARD_PER_SLAVE)
#define MAX_PL_PER_BOARD         1
#define MAX_PWC_PER_SLAVE        (MAX_PWC_PER_SPI*MAX_BOARD_PER_SLAVE)
#define MAX_TMP_PER_BOARD        2
#define MS_FRAME_SIZE            (12*1024)

#define TEST_MODE_NONE          0
#define TEST_MODE_SERVER        1
#define TEST_MODE_HASH_BOARD    2
#define TEST_MODE_MOTHER_BOARD  3
#define TEST_MODE_FAN_BOARD     4

//chenbo add begin 20180123
#define HASH_BOARD_SN_LENGHT     18
//chenbo add end

enum LedStateEnum
{
    LED_OFF             = 0,
    LED_ON              = 1,
    LED_BLINK_SLOW      = 2,
    LED_BLINK_QUICK     = 3,
    LED_BLINK_SWITCH    = 4,
    //
    MAX_LED_BLINK_TYPES = 5
};

#define BOARD_LED_COLOR_GREEN   0
#define BOARD_LED_COLOR_RED     1


#define LOG_LEVEL_NONE      0
#define LOG_LEVEL_INFO      1
#define LOG_LEVEL_DEBUG     2
#define LOG_LEVEL_TRACE     3

#define PWC_TEST_NA         0x00
#define PWC_TEST_MEM_ERR    0x01
#define PWC_TEST_OK         0xff

#endif // MS_DEFINES_H
