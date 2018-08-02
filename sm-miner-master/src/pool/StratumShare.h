#ifndef STRATUM_SHARE_H
#define STRATUM_SHARE_H
/*
 * Contains StratumShare class declaration.
 */

#include "pool/Stratum.h"

#include <stdint.h>


// Class holding a Stratum share to submit.
class StratumShare
{
// Construction/destruction.
public:
    // Default constructor.
    StratumShare();

    // Constructs the object using the specified property values.
    StratumShare(
        uint32_t jobId,
        uint32_t extraNonce2,
        uint32_t nTime,
        uint32_t nonce,
        uint32_t difficulty);

    // Copy constructor.
    StratumShare(const StratumShare& src)
    {
        copy(src);
    }

// Operators.
public:
    // Assignment operator.
    StratumShare& operator=(const StratumShare& right)
    {
        copy(right);
        return *this;
    }

    // Comparison operator (equality/inequality).
    bool operator==(const StratumShare& right) const;
    bool operator!=(const StratumShare& right) const
    {
        return !(*this == right);
    }

// Getters/setters.
public:
    uint32_t getJobId() const  { return m_jobId; }
    void setJobId(uint32_t jobId)  { m_jobId = jobId; }

    const uint8_t* getExtraNonce2Data() const  { return m_extraNonce2; }
    void setExtraNonce2(const uint8_t* extraNonce2Data, size_t extraNonce2Size);
    void setExtraNonce2(uint32_t extraNonce2);

    void shiftExtraNonce2(uint32_t targetSize);

    uint32_t getNTime() const  { return m_nTime; }
    void setNTime(uint32_t nTime)  { m_nTime = nTime; }

    uint32_t getNonce() const  { return m_nonce; }
    void setNonce(uint32_t nonce)  { m_nonce = nonce; }

    uint32_t getDifficulty() const  { return m_difficulty; }
    void setDifficulty(uint32_t difficulty)  { m_difficulty = difficulty; }

    void toString(char *buffer, size_t size) const;

// Implementation methods.
private:
    // Copies data from the specified share object.
    void copy(const StratumShare& src);

// Member variables.
private:
    // The job ID assigned.
    uint32_t m_jobId;

    // Extra-nonce 2.
    uint8_t m_extraNonce2[c_extraNoncesMaxSize];

    // Rolled timestamp.
    uint32_t m_nTime;

    // Found nonce.
    uint32_t m_nonce;

    // Share difficulty.
    uint32_t m_difficulty;
};

#endif  // STRATUM_SHARE_H
