#ifndef STRING_H
#define STRING_H
/*
 * Contains String class declaration.
 */

#include "base/BaseUtil.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>


// The wrapper class for an arbitrary C-string pointer.
// The underlying string content is not modified (immutability).
// No memory allocation/deallocation is performed.
//
class String
{
// Construction/destruction.
public:
    // Constructs the object by wrapping the given C-string pointer.
    // The "str" argument defaults to an empty string.
    explicit String(const char* str = c_emptyStr) throw()
        : m_str(str != nullptr ? str : c_emptyStr)
    {}

    // Copy constructor.
    String(const String& src) throw()
        : m_str(src.m_str)
    {}
    
// Operators.
public:
    // Assignment operators.
    String& operator=(const String& right) throw()
    {
        m_str = right.m_str;
        return *this;
    }
    String& operator=(const char* str) throw()
    {
        m_str = (str != nullptr) ? str : c_emptyStr;
        return *this;
    }

    // Comparison operators.
    template<class T>
    inline bool operator==(const T& right) const throw()
    {
        return (compareTo(static_cast<const char*>(right)) == 0);
    }
    template<class T>
    inline bool operator!=(const T& right) const throw()
    {
        return !(*this == right);
    }
    template<class T>
    inline bool operator<(const T& right) const throw()
    {
        return (compareTo(static_cast<const char*>(right)) < 0);
    }
    template<class T>
    inline bool operator<=(const T& right) const throw()
    {
        return (*this < right || *this == right);
    }
    template<class T>
    inline bool operator>(const T& right) const throw()
    {
        return !(*this <= right);
    }
    template<class T>
    inline bool operator>=(const T& right) const throw()
    {
        return !(*this < right);
    }
    
    // Conversion operators.
    inline operator const char*() const throw()
    {
        return m_str;
    }

// Public interface.
public:
    // Returns the wrapped C-string data pointer.
    inline const char* cdata() const throw()
    {
        return m_str;
    }
    
    // Returns the string length.
    size_t length() const throw()
    {
        return ::strlen(m_str);
    }
    
    // Returns true if the string is empty (string length is zero).
    bool isEmpty() const throw()
    {
        return (*m_str == '\0');
    }
    
    // Returns true if the string is empty or all spaces.
    bool isWhiteSpace() const throw();
    
    // Returns 0 if this string's content equals to the given C-string.
    // Similar to the standard strcmp() function, but treats null
    // as an empty string.
    int compareTo(const char* str) const throw();

    // Exchanges the content of this object with "other".
    void swap(String& other) throw()
    {
        util::swap(m_str, other.m_str);
    }

// Conversion methods.
public:
    long toLong(int radix = 0, long nullValue = 0) const;
    unsigned long toULong(int radix = 0, unsigned long nullValue = 0) const;
    int toInt(int radix = 0, int nullValue = 0) const;
    unsigned int toUInt(int radix = 0, unsigned int nullValue = 0) const;
    uint8_t toUInt8(int radix = 0, uint8_t nullValue = 0) const;
    uint16_t toUInt16(int radix = 0, uint16_t nullValue = 0) const;
    uint32_t toUInt32(int radix = 0, uint32_t nullValue = 0) const;
    double toDouble(double nullValue = 0.0) const;
    bool toBool(const char* trueStr = "true", const char* falseStr = "false") const;

// Member variables.
private:
    // Shared empty string buffer.
    static const char* const c_emptyStr;
    
    // C-string pointer.
    const char* m_str;
};

#endif  // STRING_H
