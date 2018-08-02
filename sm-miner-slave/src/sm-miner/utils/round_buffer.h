#ifndef ROUND_BUFFER_H
#define ROUND_BUFFER_H

#include <stdint.h>


template<class TItem>
class RoundBuffer
{
public:
    RoundBuffer(TItem *_items, uint32_t _size)
        : items(_items), SIZE(_size), tx(0), totalTx(0)
    {}

    inline uint32_t getTotalTx() const { return totalTx; }

    void add(const TItem &data)
    {
        items[tx] = data;

        tx++; totalTx++;
        if (tx >= SIZE)
        {
            tx = 0;
        }
    }

    bool contains(const TItem &data) const
    {
        for (uint32_t i = 0; i < SIZE; i++)
        {
            if (memcmp(&data, &items[i], sizeof(TItem)) == 0)
                return true;
        }
        return false;
    }

private:
    const uint32_t SIZE;
    TItem *items;

    uint32_t tx, totalTx;
};



#endif // ROUND_BUFFER_H
