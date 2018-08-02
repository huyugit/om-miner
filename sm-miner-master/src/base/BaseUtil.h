#ifndef BASE_UTIL_H
#define BASE_UTIL_H
/*
 * Defines a number of basic utility functions for different purpose.
 */

#include <stdlib.h>
#include <stdint.h>


namespace util
{
    // Returns the length of a given array.
    template<typename T, size_t LENGTH>
    inline size_t arrayLength(T(&)[LENGTH]) throw()
    {
        return LENGTH;
    }

    // Swaps two values (a & b) of arbitrary type.
    // This is the simple classic generic implementation. It will work on
    // any type which has a copy constructor and an assignment operator.
    template<typename T>
    inline void swap(T& a, T& b) throw()
    {
        const T tmp = a;
        a = b;
        b = tmp;
    }

    // Returns the given uint16_t value with the byte order changed
    // from big to little endian or vise versa.
    inline uint16_t swapEndian(uint16_t x) throw()
    {
        return static_cast<uint16_t>
            ( (x & 0x00ffu) << 8
            | (x & 0xff00u) >> 8);
    }

    // Returns the given uint32_t value with the byte order changed
    // from big to little endian or vise versa.
    inline uint32_t swapEndian(uint32_t x) throw()
    {
        return static_cast<uint32_t>
            ( (x & 0x000000fful) << 24
            | (x & 0x0000ff00ul) <<  8
            | (x & 0x00ff0000ul) >>  8
            | (x & 0xff000000ul) >> 24);
    }

    inline double safeDiv(double x, double y)
    {
        return (y > 1e-6 ? x / y : 0);
    }

}  // End of namespace util

#endif  // BASE_UTIL_H
