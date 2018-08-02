#include "utils.h"


int calcPercent(int x, int total)
{
    return total > 0 ? int( (double)(x) / total * 100 ) : 0;
}


uint32_t comressBitData(uint32_t d, uint32_t mask, uint32_t bitsPetItem)
{
    const uint32_t itemMask = (1 << bitsPetItem) - 1;

    uint32_t d2 = 0;

    for (int i = 0, j = 0; mask > 0; i++, mask >>= 1, d >>= bitsPetItem)
    {
        if (mask & 1)
        {
            d2 |= (d & itemMask) << (j * bitsPetItem);
            j++;
        }
    }

    return d2;
}
