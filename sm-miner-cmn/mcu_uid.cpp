#include "mcu_uid.h"

#include <cstdio>
#include <cstring>
#include "format.hpp"

int charToInt(char c)
{
    if (c >= '0' && c <= '9') return (c - '0');
    if (c >= 'a' && c <= 'f') return (c - 'a' + 10);
    if (c >= 'A' && c <= 'F') return (c - 'A' + 10);
    return 0;
}

McuUID::McuUID()
{
}

McuUID::McuUID(const char *str)
{
    memset(uid, 0, sizeof(uid));

    for (uint32_t i = 0; i < MCU_UID_SIZE; i++, str++, str++)
    {
        if (str[0] == 0 || str[1] == 0) break;
        uid[i] = (charToInt(str[0]) << 4) | charToInt(str[1]);
    }
}

void McuUID::setAll(uint8_t x)
{
    for (uint32_t i = 0; i < sizeof(uid); i++)
    {
        uid[i] = x;
    }
}

bool McuUID::equalTo(const McuUID &other) const
{
    return (memcmp(uid, other.uid, sizeof(uid)) == 0);
}

void McuUID::dump() const
{
    McuUIDToStr uidStr(*this);
    log("%s", uidStr.str);
}


McuUIDToStr::McuUIDToStr(const McuUID &uid)
{
    for (uint32_t i = 0; i < MCU_UID_SIZE; i++)
    {
        sprintf(str+i*2, "%02X", uid.uid[i]);
    }
}
