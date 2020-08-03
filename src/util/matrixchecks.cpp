#include <stdint.h>
#include <stdlib.h>

#include <util/matrixchecks.h>

#include <Eigen/QR>

bool heavyhash::checks::Is4BitPrecision(const Eigen::Matrix<int, 64, 64>& matrix) {
    for (Eigen::Index i = 0; i < 64; ++i) {
        for (Eigen::Index j = 0; j < 64; ++j) {
            if (matrix(i, j) < 0 || matrix (i, j) > 0xF)
                return false;
        }
    }
    return true;
}

bool heavyhash::checks::IsFullRank(const Eigen::Matrix<int, 64, 64>& matrix) {
    Eigen::MatrixXd copy = matrix.cast<double>();
    Eigen::ColPivHouseholderQR<Eigen::Ref<Eigen::MatrixXd>> decomposition(copy);

    return decomposition.rank() == 64;
}
