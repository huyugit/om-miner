/*
 * Contains String class definition.
 */

#include "String.h"

#include "except/BaseException.h"

#include <ctype.h>
#include <errno.h>


// Defines String::c_emptyStr static member.
const char* const String::c_emptyStr = "";

bool String::isWhiteSpace() const throw()
{
    for (const char* str = m_str; *str != '\0'; ++str)
    {
        if (!::isspace(*str))
            return false;
    }
    
    return true;
}

int String::compareTo(const char* str) const throw()
{
    return ::strcmp(m_str, str != nullptr ? str : c_emptyStr);
}

long String::toLong(int radix, long nullValue) const
{
    if (isWhiteSpace())
        return nullValue;

    char* endptr = nullptr;
    
    errno = 0;
    const long value = ::strtol(m_str, &endptr, radix);
    if (errno != 0)
        throw BaseException("Error converting '%s' to 'long' with radix %d", m_str, radix);
    else if (endptr != nullptr && *endptr != '\0')
        throw BaseException("Error converting '%s' to 'long' with radix %d: Illegal trailing characters: %s",
            m_str, radix, endptr);

    return value;
}

unsigned long String::toULong(int radix, unsigned long nullValue) const
{
    if (isWhiteSpace())
        return nullValue;

    char* endptr = nullptr;

    errno = 0;
    const unsigned long value = ::strtoul(m_str, &endptr, radix);
    if (errno != 0)
        throw BaseException("Error converting '%s' to 'unsigned long' with radix %d", m_str, radix);
    else if (endptr != nullptr && *endptr != '\0')
        throw BaseException("Error converting '%s' to 'unsigned long' with radix %d: Illegal trailing characters: %s",
            m_str, radix, endptr);

    return value;
}

int String::toInt(int radix, int nullValue) const
{
    return static_cast<int>(toLong(radix, nullValue));
}

unsigned int String::toUInt(int radix, unsigned int nullValue) const
{
    return static_cast<unsigned int>(toULong(radix, nullValue));
}

uint8_t String::toUInt8(int radix, uint8_t nullValue) const
{
    unsigned long tmpValue = toULong(radix, nullValue);
    if (tmpValue > static_cast<uint8_t>(~0))
        throw BaseException("Error converting '%s' to 'uint8_t': Out of range", m_str);

    return static_cast<uint8_t>(tmpValue);
}

uint16_t String::toUInt16(int radix, uint16_t nullValue) const
{
    unsigned long tmpValue = toULong(radix, nullValue);
    if (tmpValue > static_cast<uint16_t>(~0))
        throw BaseException("Error converting '%s' to 'uint16_t': Out of range", m_str);

    return static_cast<uint16_t>(tmpValue);
}

uint32_t String::toUInt32(int radix, uint32_t nullValue) const
{
    unsigned long tmpValue = toULong(radix, nullValue);
    if (tmpValue > static_cast<uint32_t>(~0))
        throw BaseException("Error converting '%s' to 'uint32_t': Out of range", m_str);

    return static_cast<uint32_t>(tmpValue);
}

double String::toDouble(double nullValue) const
{
    if (isWhiteSpace())
        return nullValue;

    char* endptr = nullptr;

    errno = 0;
    const double value = ::strtod(m_str, &endptr);
    if (errno != 0)
        throw BaseException("Error converting '%s' to 'double'", m_str);
    else if (endptr != nullptr && *endptr != '\0')
        throw BaseException("Error converting '%s' to 'double': Illegal trailing characters: %s", m_str, endptr);

    return value;
}

bool String::toBool(const char* trueStr, const char* falseStr) const
{
    assert(trueStr != nullptr);
    assert(falseStr != nullptr);
    
    if (compareTo(trueStr) == 0)
        return true;
    if (compareTo(falseStr) == 0)
        return false;
    
    throw BaseException("Error converting '%s' to 'bool': expected '%s' or '%s'", m_str, trueStr, falseStr);
}
