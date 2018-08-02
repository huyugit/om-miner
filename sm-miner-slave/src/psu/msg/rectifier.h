#ifndef RECTIFIER_H
#define RECTIFIER_H

#include <cstdint>
#include "ms_data.h"

struct Rectifier
{
    Rectifier();

    PsuInfo info;
    PsuSpec spec;
};
#endif

