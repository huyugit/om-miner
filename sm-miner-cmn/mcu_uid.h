#ifndef MCU_UID_H
#define MCU_UID_H

#include <stdint.h>

#define MCU_UID_SIZE 12


struct McuUID
{
    uint8_t uid[MCU_UID_SIZE];

    McuUID();
    McuUID(const char *str);

    void setAll(uint8_t x);
    bool equalTo(const McuUID &other) const;

    void dump() const;
}
__attribute__ ((packed));


struct McuUIDToStr
{
    char str[MCU_UID_SIZE*2 + 1];
    McuUIDToStr(const McuUID &uid);
};

#endif // MCU_UID_H
