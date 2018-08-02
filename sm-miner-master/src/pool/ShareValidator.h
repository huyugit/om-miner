#ifndef SHAREVALIDATOR_H
#define SHAREVALIDATOR_H

#include "pool/Stratum.h"
#include "pool/StratumJob.h"
#include "pool/StratumShare.h"

class ShareValidator
{
public:
    ShareValidator();

    bool calculate(const StratumJob& job, const StratumShare& share);

    uint32_t getShareDiff();

    uint8_t hash[c_sha256Size];
    uint8_t target[c_sha256Size];

    char hashStr[128];
    char targetStr[128];

    // tests
    bool debug;
    static void test();

private:
    void calcBlockTarget(uint32_t nBits);
};

#endif // SHAREVALIDATOR_H
