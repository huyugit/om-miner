#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#define ABS(x) ((x) < 0 ? -(x) : +(x))

inline uint8_t numSetBits(uint32_t x)
{
    uint8_t n = 0;
    for (int i = 0; i < 32; i++)
    {
        if (x & 1) n++;
        x >>= 1;
    }
    return n;
}

inline uint32_t SwapEndian(uint32_t k)
{
    uint8_t val;
    uint8_t *b=(uint8_t*)&k;
    val=b[0];b[0]=b[3];b[3]=val;
    val=b[1];b[1]=b[2];b[2]=val;
    return k;
}

inline uint16_t SwapEndian16(uint16_t k)
{
    uint8_t val;
    uint8_t *b=(uint8_t*)&k;
    val=b[0];b[0]=b[1];b[1]=val;
    return k;
}

inline void SwapDWORDEndians(uint32_t *k,uint32_t len)
{
    for(uint32_t i=0;i<len;i++)
        k[i]=SwapEndian(k[i]);
}


#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

inline void myswap(void *x, void *y, uint32_t size)
{
    uint8_t* px = (uint8_t*)x;
    uint8_t* py = (uint8_t*)y;
    for (uint32_t i = 0; i < size; ++i, ++px, ++py)
    {
        uint8_t z = *px;
        *px = *py;
        *py = z;
    }
}

template<typename T>
inline void myswap(T &x, T &y)
{
    T tmp = x; x = y; y = tmp;
    //myswap(&x, &y, sizeof(T));
}

int calcPercent(int x, int total);

uint32_t comressBitData(uint32_t d, uint32_t mask, uint32_t bitsPetItem);

#endif // UTILS_H
