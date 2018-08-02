#ifndef STRING_BUFFER_H
#define STRING_BUFFER_H
/*
 * Contains StringBuffer class declaration.
 */

#include "base/BaseUtil.h"
#include "base/String.h"
#include "base/MinMax.h"
#include "base/VarArgs.h"

#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>


// Class managing a string buffer based on a fixed-size array.
// The buffer size is defined by CAPACITY template argument
// (MUST be greater then 0 to contain at least null-terminator).
//
template<size_t CAPACITY>
class StringBuffer
{
// Construction/destruction.
public:
    // Default constructor.
    explicit StringBuffer(bool zeroFill = false)
        : m_len(0)
        , m_overflow(false)
    {
        assert(CAPACITY > 0);
        if (zeroFill)
            purge();
        else
            m_str[0] = '\0';
    }
    
    // Constructs the object from the given C-string pointer.
    // Copies no more then "maxChars" characters from "str".
    StringBuffer(const char* str, size_t maxChars = CAPACITY)
        : m_len(0)
        , m_overflow(false)
    {
        assert(CAPACITY > 0);
        print(str, maxChars);
    }
    
    // Constructor assigning to the buffer a string consisting of
    // "n" number of "c" characters.
    explicit StringBuffer(char c, size_t n = 1)
        : m_len(0)
        , m_overflow(false)
    {
        assert(CAPACITY > 0);
        printc(c, n);
    }
    
    // Copy constructor.
    StringBuffer(const StringBuffer& src)
        : m_len(0)
        , m_overflow(false)
    {
        assert(CAPACITY > 0);
        write(src.m_str, src.m_len);
    }

// Operators.
public:
    // Assignment operators.
    StringBuffer& operator=(const StringBuffer& right) throw()
    {
        clear();
        write(right.m_str, right.m_len);
        return *this;
    }
    StringBuffer& operator=(const char* str) throw()
    {
        clear();
        print(str);
        return *this;
    }
    
    // Array subscript operators.
    char& operator[](size_t index) throw()
    {
        assert(index < CAPACITY);
        return m_str[index];
    }
    const char& operator[](size_t index) const throw()
    {
        assert(index < CAPACITY);
        return m_str[index];
    }
    
    // Comparison operators.
    inline bool operator==(const StringBuffer& right) const throw()
    {
        return (right.str() == m_str);
    }
    inline bool operator!=(const StringBuffer& right) const throw()
    {
        return !(*this == right);
    }

// Public interface.
public:
    // Returns the allocated string buffer size.
    inline size_t capacity() const throw()  { return CAPACITY; }
    
    // Returns the string length (not counting the null terminator).
    inline size_t length() const throw()  { return m_len; }
    
    // Returns true if the string is empty (string length is zero).
    inline bool isEmpty() const throw()  { return (m_len == 0); }
    
    // Returns a pointer to the internal string buffer.
    inline const char* cdata() const throw()  { return m_str; }
    inline char* cdata() throw()  { return m_str; }
    
    // Creates String object wrapping the underlying string buffer.
    inline const String str() const throw()  { return String(m_str); }
    
    // Overflow flag. Set when the previous print/append
    // operation was truncated due to insufficient buffer space.
    inline bool isOverflow() const throw()  { return m_overflow; }

    // Empties the string buffer.
    // Resets overflow flag.
    void clear() throw()
    {
        if (m_len > 0)
        {
            m_str[0] = '\0';
            m_len = 0;
            m_overflow = false;
        }
    }

    // Same as empty(), but fills the entire string buffer with zeros.
    // Resets overflow flag.
    void purge() throw()
    {
        ::memset(m_str, 0, CAPACITY);
        m_len = 0;
        m_overflow = false;
    }
    
    // Exchanges the content of this object with "other".
    void swap(StringBuffer& other) throw()
    {
        // The below loop is not the optimal way to swap two memory
        // blocks, however, let's stick to this trivial implementation
        // until there is a need to improve.
        for (size_t i = 0; i < CAPACITY; ++i)
            util::swap(m_str[i], other.m_str[i]);
        
        util::swap(m_len, other.m_len);
        util::swap(m_overflow, other.m_overflow);
    }

// Data appending methods.
// 
// A couple of methods to add data to the end of the data currently
// stored in this string buffer.
//
// Depending of the available space in the buffer left, these methods
// may write less data then instructed. Overflow flag is set in this
// case. Null-terminator is appended to the written data anyway.
// All methods return an intrinsic pointer to the appended data.
// 
public:
    // Appends the given data of length "len" to this buffer.
    // The data may contain null characters that will all be copied.
    const char* write(const char* data, size_t len) throw()
    {
        char* const dest = prepareSpace(len);
        if (len > 0)
        {
            assert(data != nullptr);
            ::memcpy(dest, data, len);
            m_len += len;
            m_str[m_len] = '\0';
        }
        
        return dest;
    }

    // Appends the given C-string into this buffer, but no more
    // then "maxChars" characters (not counting the null-terminator).
    const char* print(const char* str, size_t maxChars = CAPACITY) throw()
    {
        const size_t len = (str != nullptr && maxChars > 0) ? ::strnlen(str, maxChars) : 0u;
        return write(str, len);
    }

    // Constructs a string consisting of "n" characters "c"
    // and appends it to this string buffer.
    const char* printc(char c, size_t n = 1) throw()
    {
        size_t len = n;
        char* const dest = prepareSpace(len);
        if (len > 0)
        {
            ::memset(dest, c, len);
            m_len += len;
            m_str[m_len] = '\0';
        }
        
        return dest;
    }
    
    // Formats a string based on the given "format" template and arguments
    // and appends it to this string buffer. The standard printf() format
    // specification applies.
    const char* printf(const char* format, ...)
    {
        VarArgs arglist;
        VA_START(arglist, format);
        return vprintf(format, arglist);
    }
    
    const char* csvprintf(const char* format, ...)
    {
        if (!isEmpty()) printf(", ");

        VarArgs arglist;
        VA_START(arglist, format);
        return vprintf(format, arglist);
    }

    // Same as format(), but uses the variable argument list.
    const char* vprintf(const char* format, va_list arglist)
    {
        const size_t maxLen = availableSpace();
        char* const dest = m_str + m_len;

        size_t len = (format != nullptr) ?
            ::vsnprintf(dest, maxLen + 1, format, arglist) : 0;
        if (len > maxLen)
        {
            len = maxLen;
            dest[maxLen] = '\0';
            m_overflow = true;
        }
        
        m_len += len;
        return dest;
    }

// Helper methods.
public:
    // Returns the maximum number of characters that can be appended
    // to this string buffer including the null-terminator.
    inline size_t availableCapacity() const throw()
    {
        return (CAPACITY - m_len);
    }
    
    // Returns the maximum length of string that can be appended
    // to this string buffer not counting the null-terminator.
    inline size_t availableSpace() const throw()
    {
        return (availableCapacity() - 1);
    }
    
    // Returns a pointer just beyond the end of the data currently stored
    // in this string buffer as a starting position for appending new data.
    // If the given length "len" of the data to append exceeds the available
    // length, updates the length to the maximum value and sets overflow flag.
    char* prepareSpace(size_t& len) throw()
    {
        const size_t maxLen = availableSpace();
        if (len > maxLen)
        {
            len = maxLen;
            m_overflow = true;
        }
        
        return (m_str + m_len);
    }

// Member variables.
private:
    // Pointer to the string storage array.
    char m_str[CAPACITY]  __attribute__((aligned));
    
    // The current string length.
    size_t m_len;
    
    // Overflow flag. Set when the previous assign or append operation
    // was truncated due to insufficient buffer space.
    bool m_overflow;
};

#endif  // STRING_BUFFER_H
