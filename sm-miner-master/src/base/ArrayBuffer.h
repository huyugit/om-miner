#ifndef ARRAY_BUFFER_H
#define ARRAY_BUFFER_H
/*
 * Contains ArrayBuffer class declaration.
 */

#include "base/ByteBuffer.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>


// ByteBuffer derivative based on a fixed-size byte array.
template<size_t CAPACITY>
class ArrayBuffer
    : public ByteBuffer
{
// Construction/destruction.
public:
    // Constructs an empty ArrayBuffer object.
    // If instructed by setting zeroFill flag, initialize the array
    // buffer with zeros.
    explicit ArrayBuffer(bool zeroFill = true)
        : ByteBuffer(m_array, CAPACITY)
    {
        if (zeroFill)
            fill(0, true);
    }

    // Copy constructor.
    ArrayBuffer(const ArrayBuffer& src)
        : ByteBuffer(m_array, CAPACITY)
    {
        if (src.isEmpty())
            return;

        write(src.m_ptr, src.m_size);
    }

    // Constructs the object and initializes it with data
    // from the specified source ("src") of a given size ("len").
    explicit ArrayBuffer(const void* src, size_t len = CAPACITY)
        : ByteBuffer(m_array, CAPACITY)
    {
        assert(len <= CAPACITY);
        write(src, len);
    }

    // Operators.
public:
    // Assignment operator.
    ArrayBuffer& operator=(const ArrayBuffer& right)
    {
        if (&right != this)
        {
            m_size = 0;
            write(right.m_ptr, right.m_size);
        }
        
        return *this;
    }

// Member variables.
private:
    // Pointer to an internal storage array.
    uint8_t m_array[CAPACITY] __attribute__((aligned));
};

#endif  // ARRAY_BUFFER_H
