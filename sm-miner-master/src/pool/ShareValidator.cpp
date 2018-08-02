#include "ShareValidator.h"

#include "old/cByteBufferType.h"

#include "base/Sha256.h"
#include "base/BaseUtil.h"
#include "base/MiscUtil.h"


void doubleSha(cByteBufferType obj, cByteBufferType &result)
{
    result.SetLength(32);
    SHA256_EncodeMessage(obj.GetBuffer(), obj.GetLength(), result.GetBuffer());
    SHA256_EncodeMessage(result.GetBuffer(), result.GetLength(), result.GetBuffer());
}


ShareValidator::ShareValidator()
    : debug(false)
{
}

bool ShareValidator::calculate(const StratumJob &job, const StratumShare &share)
{
    cByteBufferType coinbase;
    coinbase.Concat(job.getCoinbase1Data(), job.getCoinbase1Size());

    coinbase.Concat(job.getExtraNonce1(), job.getExtraNonce1Size());

    coinbase.Concat(share.getExtraNonce2Data(), job.getExtraNonce2Size());

    coinbase.Concat(job.getCoinbase2Data(), job.getCoinbase2Size());

    if (debug) {
        printf("coinbase: "); coinbase.dump();
    }


    cByteBufferType merkleRoot;
    doubleSha(coinbase, merkleRoot);

    for (size_t i = 0; i < job.getMerkleBranchLen(); i++)
    {
        merkleRoot.Concat( job.getMerkleHashData(i), c_sha256Size );
        doubleSha(merkleRoot, merkleRoot);
    }

    if (debug) {
        printf("merkleRoot: "); merkleRoot.dump();
    }


    cByteBufferType header;
    header = cByteBufferType::fromUInt32( job.getVersion() );

    cByteBufferType prevHashSwapped( job.getPrevHashData(), c_sha256Size );
    prevHashSwapped.SwapDWORDEndians();
    header += prevHashSwapped;

    header += merkleRoot;

    header += cByteBufferType::fromUInt32( share.getNTime() );

    header += cByteBufferType::fromUInt32( job.getNBits() );

    header += cByteBufferType::fromUInt32( util::swapEndian(share.getNonce()) );

    if (header.GetLength() != 80)
    {
        printf("ERROR: wrong header: "); header.dump();
        return false;
    }


    cByteBufferType hashBuffer;
    doubleSha(header, hashBuffer);
    hashBuffer.swap_array();

    if (debug) {
        printf("header: "); header.dump();
        printf("hash: "); hashBuffer.dump();
    }


    calcBlockTarget(job.getNBits());
    memcpy(hash, hashBuffer, c_sha256Size);

    util::dataToHex(hashStr, hash, sizeof(hash));
    util::dataToHex(targetStr, target, sizeof(target));

    if (debug) {
        printf("HASH:   %s\n", hashStr);
        printf("TARGET: %s\n", targetStr);
    }

    return true;
}

void ShareValidator::calcBlockTarget(uint32_t nBits)
{
    memset(target, 0, sizeof(target));

    int pow = (nBits >> 24) - 3;
    uint8_t* mantissaPtr = (uint8_t*)&nBits;

    for (size_t i = 0; i < 3; i++, mantissaPtr++, pow++)
    {
        size_t index = c_sha256Size-1 - pow;

        if (index < c_sha256Size)
        {
            target[index] = *mantissaPtr;
        }
    }
}

uint32_t ShareValidator::getShareDiff()
{
    uint32_t* hash32 = (uint32_t*)hash;

    if (hash32[0] != 0)
    {
        return 0;
    }

    if (hash32[1] == 0)
    {
        return 0xFFFFFFFF;
    }

    return 0xFFFFFFFF / util::swapEndian(hash32[1]);
}

void ShareValidator::test()
{
    //--- test job
    StratumJob job;

    job.setJobId(372);
    job.setJobName("1325");
    job.setTime(0x0014d336);
    job.setDifficulty(4096);

    //job.setExtraNonce1(0x60000823);
    cByteBufferType en1("60000823");
    job.setExtraNonce1( en1.GetBuffer(), en1.GetLength() );

    job.setExtraNonce2Size(4);

    job.setPrevHash( cByteBufferType("6d0e486862796b4a0ccfa10df321e39e6ef73b1711e653ea0000000000000000") );

    cByteBufferType cb1("01000000010000000000000000000000000000000000000000000000000000000000000000ffffffff23030e8105062f503253482f0458e07b5508");
    job.setCoinbase1( cb1.GetBuffer(), cb1.GetLength() );

    cByteBufferType cb2("092f426974467572792f000000000190efdb96000000001976a91463afd7ba46472834b3a91a31ecf16c8bcb19616188ac00000000");
    job.setCoinbase2( cb2.GetBuffer(), cb2.GetLength() );

    { cByteBufferType x("d65ba0d0c4498e159cbfaeaa673b8e0add971c8ec0f1fe98bd44f867a1894d86"); job.addMerkleHash(x); }
    { cByteBufferType x("a1f832c88ad129b846aafa64dd13c88e8a48d55a44514d61e51a97b81f4dc2c4"); job.addMerkleHash(x); }
    { cByteBufferType x("fdf44838cebfd25a00995cee89d7292fcb9dba9366aa53e9125605243cb65500"); job.addMerkleHash(x); }
    { cByteBufferType x("94c70de5bca79edc3778eb435d28da5f4b2d2f56b4159321ad10e356f072619b"); job.addMerkleHash(x); }
    { cByteBufferType x("2445785d3e63ef74a6f877eb4c8a9ca3ab4258d5dc198633fc123e052efc7cfb"); job.addMerkleHash(x); }
    { cByteBufferType x("d3d4abd1d23916696b16edd79e491c0937d44fc9a01f9f28d7d98d58c7ef6d89"); job.addMerkleHash(x); }
    { cByteBufferType x("1eabf1340da6f063375c4e78b921e744485da4584e4c0d1b7cd33f895607a11b"); job.addMerkleHash(x); }
    { cByteBufferType x("ee6de993d323e096d5ac52ff454fb5c7c7bf35367189c9f2228fc19ecba475d3"); job.addMerkleHash(x); }
    { cByteBufferType x("4d0fe2042a8dfef84e03cf9e6551a07f884b84f47eb0713a13ad0bac4b91eb6c"); job.addMerkleHash(x); }
    { cByteBufferType x("6804697366af8327a047c156af14f1a57426d7151f8f0a2445a7f0d0ea401d08"); job.addMerkleHash(x); }
    { cByteBufferType x("dbdf08d877da8cc8e14de5cee4780e7665b9e0849a7b0dfdd6bcde1fb5f7ba3a"); job.addMerkleHash(x); }

    job.setVersion(0x00000003);
    job.setNBits(0x18171a8b);
    job.setNTime(0x557be058);

    char jobStr[8*1024];
    job.toString(jobStr, sizeof(jobStr));

    printf("JOB: %s\n\n", jobStr);


    //--- test share
    StratumShare share;

    share.setJobId(372);
    share.setExtraNonce2(0x10087ef4);
    share.setNTime(0x557be072);
    share.setNonce(0x1471e841);
    share.setDifficulty(0x856815c9);

    char shareStr[512];
    share.toString(shareStr, sizeof(shareStr));

    printf("SHARE: %s\n\n", shareStr);


    //--- run test
    {
        printf("RUNNING TEST\n\n");

        ShareValidator sv;
        sv.debug = true;

        assert( sv.calculate(job, share) == true);
    }
    {
        printf("\n\nRUNNING TEST: corrupted share nTime\n\n");

        share.setNTime(0x12345678);

        ShareValidator sv;
        sv.debug = true;

        assert( sv.calculate(job, share) == true);
    }

    printf("\n\nTESTS PASSED!!!\n\n");
    assert(false);
}
