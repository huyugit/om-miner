#ifndef EXCHANGE_ZONE_BT_H
#define EXCHANGE_ZONE_BT_H

#include "cmn_defines.h"
#include "cmn_block.h"
#include "pwc_block.h"
#include "exchange_zone.h"


struct ExchangeZoneBt
{
    //-------------------------------------------
    // pwc => host
    //-------------------------------------------

    uint32_t    cnt;
    uint32_t    par;
    uint32_t    err;

    PwcTestBlock pwcTestData;

    //-------------------------------------------
    // methods
    //-------------------------------------------

    static bool validate()
    {
        bool result = true;

        if (!ExchangeZone::validateBlock("ExchangeZoneBt", sizeof(ExchangeZoneBt)))
            result = false;
        if (!ExchangeZone::validateBlock("PwcTestBlock", sizeof(PwcTestBlock)))
            result = false;

        return result;
    }

    void dump()
    {
        log("ExchangeZoneBt (size %d bytes):\n", sizeof(ExchangeZoneBt));
        log("cnt:           %d\n", cnt);
    }
}
__attribute__ ((packed));


#endif // EXCHANGE_ZONE_BT_H
