#ifndef OPOW_MATRIX_CHECKS
#define OPOW_MATRIX_CHECKS

#include <stdint.h>
#include <stdlib.h>
#include <Eigen/Dense>

namespace heavyhash {
namespace checks {
    /** Checks whether matrix contains 4 bit values and is suitable for HeavyHash*/
    bool Is4BitPrecision(const Eigen::Matrix<int, 64, 64>& matrix);

    bool IsFullRank(const Eigen::Matrix<int, 64, 64>& matrix);
}
}

#endif
