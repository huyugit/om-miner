#include "board_revisions.h"



#define MASK_001    1
#define MASK_100    4
#define MASK_101    5
#define MASK_111    7

namespace {

#define F_A_2 (BOARD_SPEC_FLAG_TYPE_A)
#define F_A_3 (F_A_2 | BOARD_SPEC_FLAG_HEAT_SINK_ERR)
#define F_A_4 (F_A_3 | BOARD_SPEC_FLAG_TMP_NUM_2 | BOARD_SPEC_FLAG_TMP_ALERT)

RevisionData g_boardRevisionsArray[] =
{
    //  revAdc              SPI                PWR       BTC
    // min   max     rev   N x L    spiMask   N x L    N   MASK      V           ADC U/I
    {  100,  300, {  40,   1, SPI_LEN,  MASK_001,   1,PWR_LEN,   8,  0x0ff,    48,  F_A_4, ADCU_FAC, ADCI_FAC, MACH_DESC    }},
    {  400,  600, {  41,   1, SPI_LEN,  MASK_001,   1,PWR_LEN,   8,  0x0ff,    48,  F_A_4, ADCU_FAC, ADCI_FAC, MACH_DESC    }},
    {  700,  900, {  42,   1, SPI_LEN,  MASK_001,   1,PWR_LEN,   8,  0x0ff,    48,  F_A_4, ADCU_FAC, ADCI_FAC, MACH_DESC    }},
    { 1000, 1200, {  43,   1, SPI_LEN,  MASK_001,   1,PWR_LEN,   8,  0x0ff,    48,  F_A_4, ADCU_FAC, ADCI_FAC, MACH_DESC    }},
    { 1300, 1500, {  44,   1, SPI_LEN,  MASK_001,   1,PWR_LEN,   8,  0x0ff,    48,  F_A_4, ADCU_FAC, ADCI_FAC, MACH_DESC    }},
    { 1600, 1800, {  45,   1, SPI_LEN,  MASK_001,   1,PWR_LEN,   8,  0x0ff,    48,  F_A_4, ADCU_FAC, ADCI_FAC, MACH_DESC    }},
    { 1900, 2100, {  46,   1, SPI_LEN,  MASK_001,   1,PWR_LEN,   8,  0x0ff,    48,  F_A_4, ADCU_FAC, ADCI_FAC, MACH_DESC    }},
    { 2200, 2400, {  47,   1, SPI_LEN,  MASK_001,   1,PWR_LEN,   8,  0x0ff,    48,  F_A_4, ADCU_FAC, ADCI_FAC, MACH_DESC    }},
    { 2500, 2700, {  48,   1, SPI_LEN,  MASK_001,   1,PWR_LEN,   8,  0x0ff,    48,  F_A_4, ADCU_FAC, ADCI_FAC, MACH_DESC    }},
    { 2800, 3000, {  49,   1, SPI_LEN,  MASK_001,   1,PWR_LEN,   8,  0x0ff,    48,  F_A_4, ADCU_FAC, ADCI_FAC, MACH_DESC    }},
    { 3100, 3300, {  50,   1, SPI_LEN,  MASK_001,   1,PWR_LEN,   8,  0x0ff,    48,  F_A_4, ADCU_FAC, ADCI_FAC, MACH_DESC    }},
};

#define MAX_BOARD_REVISONS ((sizeof(g_boardRevisionsArray) / sizeof(g_boardRevisionsArray[0])))

bool setupBoardRevisionsArray()
{
    return true;
}

static bool dummy = setupBoardRevisionsArray();
}

uint32_t BoardRevisions::getPwcMask(const BoardSpec &spec, int seq)
{
    uint32_t result = spec.btcMask;

    if (seq == spec.spiLen - 1)
    {
        // special case for the last chip
        result = 0x7e6;
    }
    else {
        result = 0x3e7; // TODO: move to defaults
    }

    return result;
}


RevisionData *BoardRevisions::getRevisionByIndex(uint32_t index)
{
    return (index < MAX_BOARD_REVISONS ? &g_boardRevisionsArray[index] : nullptr);
}

RevisionData* BoardRevisions::findRevision(uint32_t adcValue)
{
    for (uint32_t i = 0; i < MAX_BOARD_REVISONS; i++)
    {
        if (adcValue >= g_boardRevisionsArray[i].revisionAdcMin &&
            adcValue <= g_boardRevisionsArray[i].revisionAdcMax)
        {
            return &g_boardRevisionsArray[i];
        }
    }

    return nullptr;
}

RevisionData* BoardRevisions::getRevByRevId(uint8_t revisionId)
{
    for (uint32_t i = 0; i < MAX_BOARD_REVISONS; i++)
    {
        if (g_boardRevisionsArray[i].spec.revisionId == revisionId)
        {
            return &g_boardRevisionsArray[i];
        }
    }

    return nullptr;
}
