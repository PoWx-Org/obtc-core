#include <crypto/heavyhash.h>
#include <crypto/tiny_sha3/sha3.h>

#include <random>
#include <stdexcept>

#include <vector>

CSHA3_256::CSHA3_256() {
    sha3_init(&context, OUTPUT_SIZE);
}

CSHA3_256& CSHA3_256::Write(const unsigned char* data, size_t len) {
    sha3_update(&context, data, len);
    return *this;
}

void CSHA3_256::Finalize(unsigned char hash[OUTPUT_SIZE]) {
    sha3_final(hash, &context);
}

CSHA3_256& CSHA3_256::Reset() {
    *this = CSHA3_256();
    return *this;
}

CHeavyHash::CHeavyHash(uint64_t matrix_[64*64]) {
    for (int i = 0; i < 64*64; ++i)
        matrix[i] = matrix_[i];
}

CHeavyHash& CHeavyHash::Write(const unsigned char* data, size_t len) {
    hasher.Write(data, len);
    return *this;
}

void CHeavyHash::Finalize(unsigned char hash[OUTPUT_SIZE]) {
    uint256 hash_first;
    hasher.Finalize(hash_first.begin());
    uint256 product = MultiplyUsing4bitPrecision(matrix, hash_first);

    uint256 hash_xored;
    for (size_t i = 0; i < OUTPUT_SIZE; ++i) {
        hash_xored.begin()[i] = hash_first.begin()[i] ^ product.begin()[i];
    }

    CSHA3_256().Write(hash_xored.begin(), OUTPUT_SIZE).Finalize(hash);
}

CHeavyHash& CHeavyHash::Reset(uint64_t matrix_[64*64]) {
    *this = CHeavyHash(matrix_);
    return *this;
}

void MultiplyMatrices(uint64_t matrix[64*64], uint64_t vector[64], uint64_t product[64]){
    for (int i = 0; i < 64; ++i) {
        for (int j = 0; j < 64; ++j) {
            product[i] += matrix[64*i + j]*vector[j];
        }
    }
}

uint256 MultiplyUsing4bitPrecision(uint64_t matrix[64*64], const uint256& hash) {
    // conversion to matrix with 4 bit values
    uint64_t vector[64] = {0};
    ConvertTo4BitPrecisionVector(hash, vector);

    // perform matrix multiplication
    uint64_t product[64] = {0};
    MultiplyMatrices(matrix, vector, product);
    for (int i = 0; i < 64; ++i) {
        product[i] >>= 10;
    }
    return Convert4bitVectorToUint(product);
}

void ConvertTo4BitPrecisionVector(uint256 bit_sequence, uint64_t vector[64]) {
    int index = 0;
    for (auto byte : bit_sequence) {
        vector[index] = byte >> 4;
        vector[index+1] = byte & 0xF;
        index += 2;
    }
}

uint256 Convert4bitVectorToUint(const uint64_t x[64]) {
    uint256 bit_sequence;
    int index = 0;
    for (auto& byte : bit_sequence) {
        byte =  ( x[index] << 4) |  x[index+1];
        index += 2;
    }
    return bit_sequence;
}
