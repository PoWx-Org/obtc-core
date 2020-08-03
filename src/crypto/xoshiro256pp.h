#ifndef OPOW_CRYPTO_XOSHIRO256PP_H
#define OPOW_CRYPTO_XOSHIRO256PP_H

#include <stdint.h>
#include <uint256.h>

/**
 * Pseudorandom number generator
 * */
class XoShiRo256PlusPlus {
private:
    uint64_t s[4];

public:
    explicit XoShiRo256PlusPlus(uint256 seed) {
        for (int i = 0; i < 4; ++i) {
            s[i] = seed.GetUint64(i);
        }
    }

    void Reset(uint256 seed) {
        for (int i = 0; i < 4; ++i) {
            s[i] = seed.GetUint64(i);
        }
    }

    uint64_t operator()() {
        const uint64_t result = RotateLeft64(s[0] + s[3], 23) + s[0];

        const uint64_t t = s[1] << 17;

        s[2] ^= s[0];
        s[3] ^= s[1];
        s[1] ^= s[2];
        s[0] ^= s[3];

        s[2] ^= t;

        s[3] = RotateLeft64(s[3], 45);

        return result;
    }

private:
    // to avoid unexpected bugs when generator is passed by value and yields the same results
    XoShiRo256PlusPlus(const XoShiRo256PlusPlus&) = default;
    XoShiRo256PlusPlus& operator=(const XoShiRo256PlusPlus&) = default;

    static inline uint64_t RotateLeft64(const uint64_t x, int k) {
        return (x << k) | (x >> (64 - k));
    }
};

#endif //OPOW_CRYPTO_XOSHIRO256PP_H
