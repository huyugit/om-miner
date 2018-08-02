#ifndef BYTE_BUFFER_H
#define BYTE_BUFFER_H
/*
 * Contains ByteBuffer class declaration.
 */

#include "base/BaseUtil.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>


// Class representing a binary buffer.
// Defines a set of methods to operate the buffer data.
// Serves as a base for those classes defining a specific buffer
// allocation strategy. It's not allowed to create an instance
// of this class directly (derived classes should be used instead).
class ByteBuffer
{
// Destruction.
public:
    // Virtual destructor to allow derived class objects' deletion
    // through the base class pointer.
    virtual ~ByteBuffer() throw()  {}

    // Operators.
public:
    // Returns true if there is no buffer associated with the object.
    inline bool operator!() const throw()  { return (m_ptr == nullptr); }
    
    // Conversion operators.
    inline operator const uint8_t*() const throw()  { return m_ptr; }
    inline operator uint8_t*() throw()  { return m_ptr; }

// Public interface.
public:
    // Returns the physical size of the raw buffer.
    inline size_t getCapacity() const throw()  { return m_capacity; }
    
    // Returns the size of data stored in the buffer.
    inline size_t getSize() const throw()  { return m_size; }

    // Returns a size of available space in the buffer
    // (the difference between "getCapacity()" and "getSize()").
    inline size_t getAvailableCapacity() const throw()
    {
        return (m_capacity - m_size);
    }
    
    // Returns true if the buffer is empty (data size is zero).
    inline bool isEmpty() const throw()  { return (m_size == 0); }

    // Returns the raw pointer to the buffer data.
    inline const uint8_t* data() const throw()  { return m_ptr; }
    inline uint8_t* data() throw()  { return m_ptr; }

    // Returns the raw pointer to the buffer data converted to a char pointer.
    // Note that is does not automatically add the null-terminator character.
    inline const char* cdata() const throw()  { return reinterpret_cast<const char*>(m_ptr); }
    inline char* cdata() throw()  { return reinterpret_cast<char*>(m_ptr); }

    // Updates the size of the buffer data to the given value.
    // The new size should not exceed the buffer capacity.
    inline void resize(size_t newSize) throw()
    {
        assert(newSize <= m_capacity);
        m_size = newSize;
    }

    // Reads "len" bytes of the buffer data to "dest"
    // from the given "offset" position (by default from beginning).
    inline void read(void* dest, size_t len, size_t offset = 0) throw()
    {
        assert(m_ptr != nullptr);
        assert(offset + len <= m_capacity);
        ::memcpy(dest, m_ptr + offset, len);
    }

    // Writes "len" bytes of data from "src" into the buffer
    // from the given "offset" position (by default from beginning).
    // The buffer should have enough capacity to accept the data.
    // If data are written beyond the current size, the size is increased.
    inline void write(const void* src, size_t len, size_t offset = 0) throw()
    {
        assert(m_ptr != nullptr);
        
        const size_t size = offset + len;
        assert(size <= m_capacity);
        
        ::memcpy(m_ptr + offset, src, len);
        if (size > m_size)
            m_size = size;
    }

    // Populates all the buffer bytes with the given value.
    // This will not change the buffer size.
    inline void fill(int value, bool fillToCapacity = false) throw()
    {
        assert(m_ptr != nullptr);
        ::memset(m_ptr, value, fillToCapacity ? m_capacity : m_size);
    }

    // Populates "len" buffer bytes starting from "offset" with the given value.
    // This will not change the buffer size.
    inline void fill(int value, size_t len, size_t offset) throw()
    {
        assert(m_ptr != nullptr);
        assert(offset + len <= m_capacity);
        ::memset(m_ptr + offset, value, len);
    }

    // Shifts buffer data from the given offset to size to the left
    // so that a byte at offset position becomes the very first byte.
    // This will decrease the buffer size by offset.
    inline void shiftLeft(size_t offset) throw()
    {
        assert(m_ptr != nullptr);
        assert(offset <= m_size);
        
        m_size -= offset;
        ::memmove(m_ptr, m_ptr + offset, m_size);
    }

    // Sets the new buffer size to 0. Does not change the buffer capacity.
    inline void clear() throw()
    {
        resize(0);
    }

// Protected methods.
protected:
    // Default constructor.
    ByteBuffer() throw()
        : m_ptr(nullptr)
        , m_capacity(0)
        , m_size(0)
    {}

    // Constructs the object based on the given raw buffer.
    ByteBuffer(uint8_t* ptr, size_t capacity, size_t size = 0) throw()
        : m_ptr(ptr)
        , m_capacity(capacity)
        , m_size(0)
    {
        assert(ptr != nullptr || capacity == 0);
        assert(size <= capacity);
    }

    // Changes the underlying buffer pointer and it's size/capacity.
    void setPtr(void* ptr, size_t capacity, size_t size = 0) throw()
    {
        assert(ptr != nullptr || capacity == 0);
        assert(size <= capacity);

        m_ptr = static_cast<uint8_t*>(ptr);
        m_capacity = capacity;
        m_size = size;
    }

    // Exchanges the content of this object with "other".
    void swap(ByteBuffer& other) throw()
    {
        util::swap(m_ptr, other.m_ptr);
        util::swap(m_capacity, other.m_capacity);
        util::swap(m_size, other.m_size);
    }

// Member variables (protected).
protected:
    // A pointer to the underlying buffer memory.
    uint8_t* m_ptr;

    // Physical buffer size.
    size_t m_capacity;

    // Size of the data currently stored in the buffer.
    size_t m_size;
};

#endif  // BYTE_BUFFER_H
