#include "Sha256.h"

#include <string.h>

//поменять порядок байт
inline uint32_t SwapEndian(uint32_t k)
{
    uint8_t val;
    uint8_t *b=(uint8_t*)&k;
    val=b[0];b[0]=b[3];b[3]=val;
    val=b[1];b[1]=b[2];b[2]=val;
    return k;
}

// TODO: check if func is used somewhere
inline void SwapDWORDEndians(uint32_t *k,uint32_t len)
{
    for(uint32_t i=0;i<len;i++)
        k[i]=SwapEndian(k[i]);
}


#define rotrFixed(x,y) (((x) >> (y)) | ((x) << (32-(y))))
#define s0(x) (rotrFixed(x,7)^rotrFixed(x,18)^(x>>3))
#define s1(x) (rotrFixed(x,17)^rotrFixed(x,19)^(x>>10))
#define Ch(x,y,z) (z^(x&(y^z)))
#define Maj(x,y,z) (y^((x^y)&(y^z)))
#define S0(x) (rotrFixed(x,2)^rotrFixed(x,13)^rotrFixed(x,22))
#define S1(x) (rotrFixed(x,6)^rotrFixed(x,11)^rotrFixed(x,25))



/* SHA256 CONSTANTS */
const uint32_t SHA_K[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

const uint32_t sha_initial_state[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};


#define blk0(i) (W[i] = data[i])
#define blk2(i) (W[i&15]+=s1(W[(i-2)&15])+W[(i-7)&15]+s0(W[(i-15)&15]))

#define a(i) T[(0-i)&7]
#define b(i) T[(1-i)&7]
#define c(i) T[(2-i)&7]
#define d(i) T[(3-i)&7]
#define e(i) T[(4-i)&7]
#define f(i) T[(5-i)&7]
#define g(i) T[(6-i)&7]
#define h(i) T[(7-i)&7]

#define R(i) h(i)+=S1(e(i))+Ch(e(i),f(i),g(i))+SHA_K[i+j]+(j?blk2(i):blk0(i));\
        d(i)+=h(i);h(i)+=S0(a(i))+Maj(a(i),b(i),c(i))


void SHA256_Full(uint32_t *state, uint32_t *data, const uint32_t *st)
{
        uint32_t W[16];
        uint32_t T[8];
        uint32_t j;

        T[0] = state[0] = st[0]; T[1] = state[1] = st[1]; T[2] = state[2] = st[2]; T[3] = state[3] = st[3];
        T[4] = state[4] = st[4]; T[5] = state[5] = st[5]; T[6] = state[6] = st[6]; T[7] = state[7] = st[7];
        j = 0;
        for (j = 0; j < 64; j+= 16) { R(0); R(1);  R(2); R(3); R(4); R(5); R(6); R(7); R(8); R(9); R(10); R(11); R(12); R(13); R(14); R(15); }
        state[0] += T[0]; state[1] += T[1]; state[2] += T[2]; state[3] += T[3];
        state[4] += T[4]; state[5] += T[5]; state[6] += T[6]; state[7] += T[7];
}

void SHA256_EncodeMessage(const uint8_t* message, uint32_t messageLen, uint8_t* hash) // TODO: make hash as uint32_t
{
    uint32_t len = messageLen;
    const uint8_t* ptr = message;

    bool firstIteration = true;
    bool endBitAdded = false;
    bool lenAdded = false;

    uint8_t data[16*4];

    while (len > 0 || !endBitAdded || !lenAdded)
    {
        uint32_t l = (len < 64 ? len : 64);

        memset(data, 0, sizeof(data)); // TODO: performance leak
        memcpy(data, ptr, l);

        len -= l;
        ptr += l;

        if (!endBitAdded && l < 64)
        {
            endBitAdded = true;
            data[l] = 0x80;
            l++;
        }

        SwapDWORDEndians((uint32_t*)data, 16);

        if (!lenAdded && l <= 64-8)
        {
            lenAdded = true;
            *(uint32_t*)(data + (64-4)) = messageLen * 8; // messageLen in bits!
            // l += 4; - does not matter
        }

        uint32_t* state = (firstIteration ? (uint32_t*)sha_initial_state : (uint32_t*)hash);

        //log("SHA256_EncodeMessage: hashing block:\n");
        //log("data:  "); hexdump32((uint32_t*)data, 16);
        //log("state: "); hexdump32(state, 8);

        SHA256_Full((uint32_t*)hash, (uint32_t*)data, state);

        //log("hash:  "); hexdump32((uint32_t*)hash, 8);

        firstIteration = false;
    }

    SwapDWORDEndians((uint32_t*)hash, 8);
}
