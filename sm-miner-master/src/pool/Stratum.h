#ifndef STRATUM_H
#define STRATUM_H
/*
 * Contains Stratum pool related constants and type declarations.
 */

#include <stdlib.h>
#include <stdint.h>


// Maximum length of job ID assigned by Pool.
const size_t c_jobNameMaxLen = 64;

// SHA-256 digest size (bytes).
const size_t c_sha256Size = 32;

// Maximum size of the initial part of coinbase transaction.
const size_t c_coinbase1MaxSize = 4096;

// Maximum size of the final part of coinbase transaction.
const size_t c_coinbase2MaxSize = 4096;

// Maximum number of hashes in Merkle branch.
const size_t c_merkleBranchMaxLen = 16;

// Maximum number of extra-nonce 1/2 (bytes).
const size_t c_extraNoncesMaxSize = 8;


#endif  // STRATUM_H
