#ifndef HEAP_BUFFER_H
#define HEAP_BUFFER_H
/*
 * Contains HeapBuffer class declaration.
 */

#include "base/ByteBuffer.h"
#include "except/OutOfMemoryException.h"

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>


// This is wrapper for dynamically allocated buffer to ensure
// the memory is disposed when the class instance is destroyed.
// The storage is allocated with the standard malloc() function
// on construction and deallocated with free() on destruction.
class HeapBuffer
    : public ByteBuffer
{
// Construction/destruction.
public:
    // Constructs the object allocating the buffer of a given size.
    explicit HeapBuffer(size_t capacity = 0)
        : ByteBuffer()
    {
        if (capacity > 0)
            alloc(capacity);
    }

    // Copy constructor.
    HeapBuffer(const HeapBuffer& src)
        : ByteBuffer()
    {
        if (src.isEmpty())
            return;
        
        alloc(src.m_capacity);
        write(src.m_ptr, src.m_size);
    }

    // Constructs the object and initializes it with data
    // from the specified source ("src") of a given size ("len").
    // It will allocate either "len" or "capacity" bytes for the buffer
    // whatever is greater.
    HeapBuffer(const void* src, size_t len, size_t capacity = 0)
        : ByteBuffer()
    {
        if (capacity < len)
            capacity = len;
        
        if (capacity == 0)
            return;
        
        assert(src != nullptr);
        alloc(capacity);
        write(src, len);
    }

    // Disposes the associated buffer memory.
    ~HeapBuffer() throw()
    {
        free();
    }

// Operators.
public:
    // Assignment operator.
    HeapBuffer& operator=(const HeapBuffer& right)
    {
        HeapBuffer(right).swap(*this);
        return *this;
    }

// Public interface.
public:
    // Allocates a block of memory of the given size and associate it
    // with the buffer object. Previously allocated memory (if any)
    // is disposed. Returns a pointer to the allocated buffer.
    // If not enough memory, throws OutOfMemoryException.
    uint8_t* alloc(size_t capacity)
    {
        assert(capacity > 0);
        void* const ptr = ::malloc(capacity);
        if (ptr == nullptr)
            throw OutOfMemoryException();
        
        return attach(ptr, capacity);
    }

    // Disposes memory associated with this buffer object.
    void free() throw()
    {
        if (m_ptr != nullptr)
        {
            ::free(m_ptr);
            setPtr(nullptr, 0);
        }
    }

    // Attaches a block of memory of a given size ("capacity"),
    // allocated by malloc(), to this buffer object.
    // Disposes the existing memory allocation, if any.
    // Returns a pointer to the attached buffer.
    uint8_t* attach(void* ptr, size_t capacity) throw()
    {
        free();
        setPtr(ptr, capacity);
        return m_ptr;
    }

    // Exchanges the content of this object with "other".
    void swap(HeapBuffer& other) throw()
    {
        ByteBuffer::swap(other);
    }
};

#endif  // BYTE_BUFFER_H
