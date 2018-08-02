/*
 * Contains StratumShare class definition.
 */

#include "StratumShare.h"

#include "base/BaseUtil.h"
#include "base/MiscUtil.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>


StratumShare::StratumShare()
    : m_jobId(0)
    , m_nTime(0)
    , m_nonce(0)
    , m_difficulty(0)
{
    ::memset(m_extraNonce2, 0, sizeof(m_extraNonce2));
}

StratumShare::StratumShare(
     uint32_t jobId,
     uint32_t extraNonce2,
     uint32_t nTime,
     uint32_t nonce,
     uint32_t difficulty)
     : m_jobId(jobId)
     , m_nTime(nTime)
     , m_nonce(nonce)
     , m_difficulty(difficulty)
{
    setExtraNonce2(extraNonce2);
}

void StratumShare::copy(const StratumShare& src)
{
    m_jobId = src.m_jobId;
    ::memcpy(m_extraNonce2, src.m_extraNonce2, c_extraNoncesMaxSize);
    m_nTime = src.m_nTime;
    m_nonce = src.m_nonce;
    m_difficulty = src.m_difficulty;
}

bool StratumShare::operator==(const StratumShare& right) const
{
    if (m_jobId != right.m_jobId)
        return false;
    else if (m_nonce != right.m_nonce)
        return false;
    else if (m_nTime != right.m_nTime)
        return false;
    else if (::memcmp(m_extraNonce2, right.m_extraNonce2, c_extraNoncesMaxSize) != 0)
        return false;

    return true;
}

void StratumShare::setExtraNonce2(const uint8_t* extraNonce2Data, size_t extraNonce2Size)
{
    assert(extraNonce2Size <= c_extraNoncesMaxSize);
    ::memcpy(m_extraNonce2, extraNonce2Data, extraNonce2Size);
    ::memset(m_extraNonce2 + extraNonce2Size, 0, c_extraNoncesMaxSize - extraNonce2Size);
}

void StratumShare::setExtraNonce2(uint32_t extraNonce2)
{
    //extraNonce2 = util::swapEndian(extraNonce2); //chenbo delete for 8 Bytes extraNonce update from Bitfury 1205 code
    setExtraNonce2(reinterpret_cast<const uint8_t*>(&extraNonce2), 4);
}

void StratumShare::shiftExtraNonce2(uint32_t targetSize)
{
    if (targetSize <= 4)
    {
        int delta = 4 - targetSize;

        for (int i = 0; i < c_extraNoncesMaxSize - delta; i++)
        {
            m_extraNonce2[i] = m_extraNonce2[i + delta];
        }
    }
    else {
        printf("ERROR: shiftExtraNonce2: wrong targetSzie = %u\n", targetSize);
    }
}

void StratumShare::toString(char *buffer, size_t size) const
{
    char extraNonce2Str[c_extraNoncesMaxSize * 2 + 1];
    util::dataToHex(extraNonce2Str, m_extraNonce2, /*jobPtr->getExtraNonce2Size()*/ 4);

    char nTimeStr[9];
    const uint32_t tmpNTime = util::swapEndian(m_nTime);
    util::dataToHex(nTimeStr, &tmpNTime, 4);

    char nonceStr[9];
    const uint32_t tmpNonce = m_nonce;
    util::dataToHex(nonceStr, &tmpNonce, 4);

    ::snprintf(buffer, size, "jobId: %u, extraNonce2: %s, nTime: %s, nonce: %s, diff: %u (0x%08x)",
               m_jobId, extraNonce2Str, nTimeStr, nonceStr, m_difficulty, m_difficulty);
}
