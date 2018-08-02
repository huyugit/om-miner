#ifndef BOARD_REVISIONS_H
#define BOARD_REVISIONS_H

#include <stdint.h>
#include "ms-protocol/ms_data.h"


struct RevisionData
{
    uint16_t revisionAdcMin;
    uint16_t revisionAdcMax;

    BoardSpec spec;
};


class BoardRevisions
{
public:
    // Reserverd Revisions
    static const uint8_t REV_ID_NONE                = 0;
    static const uint8_t REV_ID_MANUAL_CONFIG       = 1;    // custom board
    static const uint8_t REV_ID_AUTO                = 3;    // auto detect
    static const uint8_t REV_ID_DEFAULT             = 20;   // default rev for custom override

    static uint32_t getPwcMask(const BoardSpec &spec, int seq);

    static RevisionData* getRevisionByIndex(uint32_t index);

    static RevisionData* findRevision(uint32_t adcValue);
    static RevisionData* getRevByRevId(uint8_t revisionId);
};

#endif // BOARD_REVISIONS_H
