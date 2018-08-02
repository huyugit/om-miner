#ifndef STRATUM_JOB_H
#define STRATUM_JOB_H
/*
 * Contains StratumJob class declaration.
 */

#include "pool/Stratum.h"

#include <stdint.h>
#include <assert.h>


// Class holding Stratum job data.
class StratumJob
{
friend class StratumPool;
friend class ShareValidator;

// Construction/destruction.
public:
    // Default constructor.
    StratumJob();

    // Copy constructor.
    StratumJob(const StratumJob& src)
    {
        assign(src);
    }

// Operators.
public:
    // Assignment operator.
    StratumJob& operator=(const StratumJob& right)
    {
        assign(right);
        return *this;
    }

// Public getters.
public:
    uint32_t getJobId() const  { return m_jobId; }
    
    const char* getJobName() const  { return m_jobName; }

    uint32_t getTime() const  { return m_time; }

    uint32_t getDifficulty() const  { return m_difficulty; }

    const uint32_t getExtraNonce1Size() const  { return m_extraNonce1Size; }
    const uint8_t* getExtraNonce1() const  { return m_extraNonce1; }

    uint32_t getExtraNonce2Size() const  { return m_extraNonce2Size; }
    
    const uint8_t* getPrevHashData() const  { return m_prevHash; }
    
    size_t getCoinbase1Size() const  { return m_coinbase1Size; }
    
    const uint8_t* getCoinbase1Data() const  { return m_coinbase1; }
    
    size_t getCoinbase2Size() const  { return m_coinbase2Size; }
    
    const uint8_t* getCoinbase2Data() const  { return m_coinbase2; }

    size_t getMerkleBranchLen() const  { return m_merkleBranchLen; }
    
    const uint8_t* getMerkleHashData(size_t hashNum) const
    {
        assert(hashNum < m_merkleBranchLen);
        return m_merkleBranch[hashNum];
    }

    uint32_t getVersion() const  { return m_version; }

    uint32_t getNBits() const  { return m_nBits; }

    uint32_t getNTime() const  { return m_nTime; }

    bool getCleanJobs() const  { return m_cleanJobs; }

    // Util methodds

    void toString(char *buffer, size_t size) const;
    void toStringShort(char *buffer, size_t size) const;

// Private setters.
private:
    void setJobId(uint32_t jobId)  { m_jobId = jobId; }
    
    void setJobName(const char* jobName);
    
    void setTime(uint32_t time)  { m_time = time; }
    
    void setDifficulty(uint32_t difficulty)  { m_difficulty = difficulty; }
    
    void setExtraNonce1(const uint8_t* extraNonce1, uint32_t extraNonce1Size);
    
    void setExtraNonce2Size(uint32_t extraNonce2Size)  { m_extraNonce2Size = extraNonce2Size; }
    
    void setPrevHash(const uint8_t* prevHashData);
    uint8_t* getPrevHash()  { return m_prevHash; }
    
    void setCoinbase1(const uint8_t* coinbase1Data, size_t coinbase1Size);
    uint8_t* getCoinbase1()  { return m_coinbase1; }
    void setCoinbase1Size(size_t coinbase1Size)  { m_coinbase1Size = coinbase1Size; }
    
    void setCoinbase2(const uint8_t* coinbase2Data, size_t coinbase2Size);
    uint8_t* getCoinbase2()  { return m_coinbase2; }
    void setCoinbase2Size(size_t coinbase2Size)  { m_coinbase2Size = coinbase2Size; }
    
    void addMerkleHash(const uint8_t* merkleHashData);
    
    void setVersion(uint32_t version)  { m_version = version; }
    
    void setNBits(uint32_t nBits)  { m_nBits = nBits; }

    void setNTime(uint32_t nTime)  { m_nTime = nTime; }
    
    void setCleanJobs(uint32_t cleanJobs)  { m_cleanJobs = cleanJobs; }

// Implementation methods.
private:
    // Copies data from the specified job.
    void assign(const StratumJob& src);

// Member variables.
private:
    // The job ID assigned by Master.
    uint32_t m_jobId;

    // The job ID from Pool.
    char m_jobName[c_jobNameMaxLen + 1];

    // The job timestamp recorded by Master.
    uint32_t m_time;
    
    // Current difficulty.
    uint32_t m_difficulty;

    // Extra-nonce 1 (unique per-connection) 
    uint32_t m_extraNonce1Size;
    uint8_t m_extraNonce1[c_extraNoncesMaxSize];
    
    // Extra-nonce 2 size in bytes.
    uint32_t m_extraNonce2Size;

    // Hash of previous block.
    uint8_t m_prevHash[c_sha256Size];
    
    // Coinbase1 size in bytes (see m_coinbase1).
    size_t m_coinbase1Size;
    
    // Initial part of coinbase transaction data.
    uint8_t m_coinbase1[c_coinbase1MaxSize];
    
    // Coinbase2 size in bytes (see m_coinbase2).
    size_t m_coinbase2Size;
    
    // Final part of coinbase transaction data.
    uint8_t m_coinbase2[c_coinbase2MaxSize];

    // A number of hashes in Merkle branch (0..c_merkleBranchMaxLen).
    size_t m_merkleBranchLen;

    // List of hashes used for calculation of Merkle root.
    uint8_t m_merkleBranch[c_merkleBranchMaxLen][c_sha256Size];

    // Bitcoin block version.
    uint32_t m_version;
    
    // Encoded current network difficulty.
    uint32_t m_nBits;
    
    // Block timestamp recorder by pool.
    uint32_t m_nTime;

    // Whether to discard previous jobs' shares.
    bool m_cleanJobs;
};

#endif  // STRATUM_JOB_H
