/*
 * Contains StratumJob class definition.
 */

#include "StratumJob.h"

#include "base/StringBuffer.h"
#include "base/MiscUtil.h"

#include <stdlib.h>
#include <string.h>


StratumJob::StratumJob()
    : m_jobId(0)
    , m_time(0)
    , m_difficulty(0)
    , m_extraNonce1Size(0)
    , m_extraNonce2Size(0)
    , m_coinbase1Size(0)
    , m_coinbase2Size(0)
    , m_merkleBranchLen(0)
    , m_version(0)
    , m_nBits(0)
    , m_nTime(0)
    , m_cleanJobs(false)
{
    ::memset(m_jobName, 0, sizeof(m_jobName));
    ::memset(m_extraNonce1, 0, sizeof(m_extraNonce1));
    ::memset(m_prevHash, 0, sizeof(m_prevHash));
    ::memset(m_coinbase1, 0, sizeof(m_coinbase1));
    ::memset(m_coinbase2, 0, sizeof(m_coinbase2));
    ::memset(m_merkleBranch, 0, sizeof(m_merkleBranch));
}

void StratumJob::assign(const StratumJob& src)
{
    ::strncpy(m_jobName, src.m_jobName, c_jobNameMaxLen);
    m_jobId = src.m_jobId;
    m_time = src.m_time;
    m_difficulty = src.m_difficulty;
    m_extraNonce1Size = src.m_extraNonce1Size;
    ::memcpy(m_extraNonce1, src.m_extraNonce1, src.m_extraNonce1Size);
    m_extraNonce2Size = src.m_extraNonce2Size;
    ::memcpy(m_prevHash, src.m_prevHash, c_sha256Size);
    m_coinbase1Size = src.m_coinbase1Size;
    ::memcpy(m_coinbase1, src.m_coinbase1, src.m_coinbase1Size);
    m_coinbase2Size = src.m_coinbase2Size;
    ::memcpy(m_coinbase2, src.m_coinbase2, src.m_coinbase2Size);
    m_merkleBranchLen = src.m_merkleBranchLen;
    ::memcpy(m_merkleBranch, src.m_merkleBranch, src.m_merkleBranchLen * c_sha256Size);
    m_version = src.m_version;
    m_nBits = src.m_nBits;
    m_nTime = src.m_nTime;
    m_cleanJobs = src.m_cleanJobs;
}

void StratumJob::setJobName(const char* jobName)
{
    assert(jobName != nullptr);
    ::strncpy(m_jobName, jobName, c_jobNameMaxLen);
    if (m_jobName[c_jobNameMaxLen] != '\0')
        m_jobName[c_jobNameMaxLen] = '\0';
}

void StratumJob::setExtraNonce1(const uint8_t *extraNonce1, uint32_t extraNonce1Size)
{
    assert(extraNonce1 != nullptr);
    assert(extraNonce1Size <= c_extraNoncesMaxSize);
    ::memcpy(m_extraNonce1, extraNonce1, extraNonce1Size);
    m_extraNonce1Size = extraNonce1Size;
}

void StratumJob::setPrevHash(const uint8_t* prevHashData)
{
    assert(prevHashData != nullptr);
    ::memcpy(m_prevHash, prevHashData, c_sha256Size);
}

void StratumJob::setCoinbase1(const uint8_t* coinbase1Data, size_t coinbase1Size)
{
    assert(coinbase1Data != nullptr);
    assert(coinbase1Size <= c_coinbase1MaxSize);
    ::memcpy(m_coinbase1, coinbase1Data, coinbase1Size);
    m_coinbase1Size = coinbase1Size;
}

void StratumJob::setCoinbase2(const uint8_t* coinbase2Data, size_t coinbase2Size)
{
    assert(coinbase2Data != nullptr);
    assert(coinbase2Size <= c_coinbase2MaxSize);
    ::memcpy(m_coinbase2, coinbase2Data, coinbase2Size);
    m_coinbase2Size = coinbase2Size;
}

void StratumJob::addMerkleHash(const uint8_t* merkleHashData)
{
    assert(merkleHashData != nullptr);
    if (m_merkleBranchLen >= c_merkleBranchMaxLen)
        return;  // throw ex
    
    ::memcpy(m_merkleBranch[m_merkleBranchLen++], merkleHashData, c_sha256Size);
}


void StratumJob::toString(char *buffer, size_t size) const
{
    StringBuffer<8*1024> sb;
    char s[4*1024];

    sb.printf("jobId: %u, ", m_jobId);
    sb.printf("jobName: %s, ", m_jobName);
    sb.printf("time: 0x%08x, ", m_time);
    sb.printf("difficulty: %u, ", m_difficulty);

    util::dataToHex(s, m_extraNonce1, m_extraNonce1Size);
    sb.printf("extraNonce1: %s, ", s);

    sb.printf("extraNonce2Size: %u, ", m_extraNonce2Size);

    util::dataToHex(s, m_prevHash, sizeof(m_prevHash));
    sb.printf("prevHash: %s, ", s);

    util::dataToHex(s, m_coinbase1, m_coinbase1Size);
    sb.printf("coinbase1: %s, ", s);

    util::dataToHex(s, m_coinbase2, m_coinbase2Size);
    sb.printf("coinbase2: %s, ", s);

    for (size_t i = 0; i < m_merkleBranchLen; i++)
    {
        util::dataToHex(s, m_merkleBranch[i], sizeof(m_merkleBranch[i]));
        sb.printf("merkleRoot[%d]: %s, ", i, s);
    }

    sb.printf("version: 0x%08x, ", m_version);
    sb.printf("nBits: 0x%08x, ", m_nBits);
    sb.printf("nTime: 0x%08x, ", m_nTime);
    sb.printf("cleanJobs: %u", m_cleanJobs);

    ::snprintf(buffer, size, "%s", sb.cdata());
}

void StratumJob::toStringShort(char *buffer, size_t size) const
{
    StringBuffer<1024> sb;
    char s[1024];

    sb.printf("jobName: %s, ", m_jobName);

    util::dataToHex(s, m_prevHash, sizeof(m_prevHash));
    sb.printf("prevHash: %s, ", s);

    sb.printf("nTime: 0x%08x, ", m_nTime);
    sb.printf("cleanJobs: %u", m_cleanJobs);

    ::snprintf(buffer, size, "%s", sb.cdata());
}
