#include <crypto/heavyhash.h>
#include <crypto/tiny_sha3/sha3.h>
#include <uint256.h>

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

Eigen::Matrix<int, 64, 1> ConvertTo4BitPrecisionVector(uint256 bit_sequence);

uint256 Convert4bitVectorToUint(const Eigen::Matrix<int, 64, 1>& x);

uint256 MultiplyUsing4bitPrecision(const Eigen::Matrix<int, 64, 64>& matrix, const uint256& hash);


CHeavyHash::CHeavyHash(const Eigen::Matrix<int, 64, 64>& matrix_) : matrix(matrix_) {}

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

CHeavyHash& CHeavyHash::Reset(const Eigen::Matrix<int, 64, 64>& matrix_) {
    *this = CHeavyHash(matrix_);
    return *this;
}

uint256 MultiplyUsing4bitPrecision(const Eigen::Matrix<int, 64, 64>& matrix, const uint256& hash) {
    Eigen::Matrix<int, 64, 1> vector = ConvertTo4BitPrecisionVector(hash);
    Eigen::Matrix<int, 64, 1> product = matrix * vector;

    for (Eigen::Index i = 0; i < 64; ++i) {
        product[i] >>= 10;
    }

    return Convert4bitVectorToUint(product);
}

Eigen::Matrix<int, 64, 1> ConvertTo4BitPrecisionVector(uint256 bit_sequence) {
    Eigen::Matrix<int, 64, 1> result;
    Eigen::Index index = 0;
    for (auto byte : bit_sequence) {
        result[index] = byte >> 4;
        result[index+1] = byte & 0xF;
        index += 2;
    }
    return result;
}

uint256 Convert4bitVectorToUint(const Eigen::Matrix<int, 64, 1>& x) {
    uint256 bit_sequence;
    Eigen::Index index = 0;
    for (auto& byte : bit_sequence) {
        byte = (x[index] << 4) | x[index+1];
        index += 2;
    }
    return bit_sequence;
}
