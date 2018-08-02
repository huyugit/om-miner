#ifndef SAG6400_RECTIFIER_H
#define SAG6400_RECTIFIER_H

#include <cstdint>
#include "ms_data.h"


struct SagRectifier
{
    SagRectifier();

    PsuInfo info;
    PsuSpec spec;
};

#endif
