#ifndef OPOW_CRYPTO_HEAVYHASH_H
#define OPOW_CRYPTO_HEAVYHASH_H

#include <stdint.h>
#include <stdlib.h>
#include <crypto/tiny_sha3/sha3.h>
#include <Eigen/Dense>


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
    Eigen::Matrix<int, 64, 64> matrix;
    CSHA3_256 hasher;

public:
    static const size_t OUTPUT_SIZE = 32;

    explicit CHeavyHash(const Eigen::Matrix<int, 64, 64>& matrix);
    CHeavyHash& Write(const unsigned char* data, size_t len);
    void Finalize(unsigned char hash[OUTPUT_SIZE]);
    CHeavyHash& Reset(const Eigen::Matrix<int, 64, 64>& matrix);
};

#endif  // OPOW_CRYPTO_HEAVYHASH_H
