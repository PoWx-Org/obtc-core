#ifndef OPOW_CRYPTO_HEAVYHASH_H
#define OPOW_CRYPTO_HEAVYHASH_H

#include <stdint.h>
#include <stdlib.h>
#include <crypto/tiny_sha3/sha3.h>
#include <uint256.h>
#include <memory>

/** A hasher class for SHA3-256. */
class CSHA3_256
{
private:
    sha3_ctx_t context;

public:
    static const size_t OUTPUT_SIZE = 32;

    CSHA3_256();
    CSHA3_256& Write(const unsigned char* data, size_t len);
    void Finalize(unsigned char hash[OUTPUT_SIZE]);
    CSHA3_256& Reset();
};

class CHeavyHash
{
private:
    uint64_t matrix[64*64];
    CSHA3_256 hasher;

public:
    static const size_t OUTPUT_SIZE = 32;
    explicit CHeavyHash(uint64_t matrix_[64*64]);
    CHeavyHash& Reset(uint64_t matrix_[64*64]);
    CHeavyHash& Write(const unsigned char* data, size_t len);
    void Finalize(unsigned char hash[OUTPUT_SIZE]);
};

uint256 MultiplyUsing4bitPrecision(uint64_t matrix[64*64], const uint256& hash);

void ConvertTo4BitPrecisionVector(uint256 bit_sequence, uint64_t vector[64]);

uint256 Convert4bitVectorToUint(const uint64_t x[64]);

#endif  // OPOW_CRYPTO_HEAVYHASH_H
