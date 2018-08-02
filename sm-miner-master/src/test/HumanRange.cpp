#include "HumanRange.h"

#include <cstdio>
#include "base/StringBuffer.h"

void HumanRange::push(int x)
{
    if (ranges.empty() || ranges.back().x2 + 1 != x)
    {
        ranges.push_back(Range(x, x));
    }
    else
    {
        ranges.back().x2 = x;
    }
}

