#include <stdint.h>
#include <stdlib.h>

#include <util/matrixchecks.h>
#include <matrix-utils/singular/Svd.h>

bool heavyhash::checks::Is4BitPrecision(const uint64_t matrix[64*64]){
    for (int i = 0; i < 64; ++i) {
        for (int j = 0; j < 64; ++j) {
            if (matrix[ i*64 + j] > 0xF)
                return false;
        }
    }
    return true;
}

bool heavyhash::checks::IsFullRank(const uint64_t matrix_[64*64]){
    double matrix__ [64*64];
    singular::Matrix<64, 64> matrix;
    for (int i = 0; i < 64; ++i) {
        for (int j = 0; j < 64; ++j) {
            matrix__[64*i + j] = (double) matrix_[64*i + j];
        }
    }
    matrix.fill(matrix__);
    singular::Svd< 64, 64 >::USV usv = singular::Svd< 64, 64 >::decomposeUSV(matrix);               // computes SVD
    const singular::DiagonalMatrix< 64, 64 >& singularValues = singular::Svd< 64, 64 >::getS(usv);  // gets singular values only
    return singular::Svd< 64, 64 >::isFullRank(singularValues, 64);                                 // checks if full rank
}
