#ifndef OPOW_MATRIX_CHECKS
#define OPOW_MATRIX_CHECKS

#include <stdint.h>
#include <stdlib.h>

namespace heavyhash {
namespace checks {
    /** Checks whether matrix contains 4 bit values and is suitable for HeavyHash*/
    bool Is4BitPrecision(const uint64_t matrix[64*64]);
    bool IsFullRank(const uint64_t matrix[64*64]);
}
}

#endif
