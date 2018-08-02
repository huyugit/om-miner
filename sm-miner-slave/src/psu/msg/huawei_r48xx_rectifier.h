#ifndef HUAWEI_R48XX_RECTIFIER_H
#define HUAWEI_R48XX_RECTIFIER_H

#include <cstdint>

#include "huawei_r48xx_msg.h"
#include "ms_data.h"

struct huaWeiR48xxRectifier
{
    huaWeiR48xxRectifier();

    PsuInfo info;
    PsuSpec spec;
};
#endif
