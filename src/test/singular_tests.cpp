// Copyright (c) 2020-2021 The PoWx Core developers

#include <crypto/heavyhash_dummyArray.h>

#include <matrix-utils/singular/Svd.h>
#include <test/util/setup_common.h>
#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(singular_tests, BasicTestingSetup)

using namespace std;
const int N = 4;
const int M = 64;
const double DATA[] = {
        1, 7, 7, 5,
        2, 1, 8, 10,
        1, 2, 9, 17,
        1, 2, 10, 2
};

const double WRONG_DATA1[] = {
        1, 7, 7, 5,
        1, 7, 7, 5,
        1, 2, 9, 17,
        1, 2, 10, 2
};


const double WRONG_DATA2[] = {
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0
};

static bool CheckCompatibility(const double *data) {
    singular::Matrix< N, N > matrix;
    matrix.fill(data);
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            if (data[i * N + j] != matrix(i, j))
                return false;
        }
    }
    return true;
}

static bool CheckMatrixRank4x4(const double *data) {
    singular::Matrix< N, N > matrix;
    matrix.fill(data);
    singular::Svd< N, N >::USV usv = singular::Svd< N, N >::decomposeUSV(matrix);
    const singular::DiagonalMatrix< N, N >& sing = singular::Svd< N, N >::getS(usv);
    cout << sing << endl;
    cout << endl;
    return singular::Svd< N, N >::isFullRank(sing, N);
}


static bool CheckMatrixRank64x64(const double *data) {
    singular::Matrix< M, M > matrix;
    matrix.fill(data);
    singular::Svd< M, M >::USV usv = singular::Svd< M, M >::decomposeUSV(matrix);
    const singular::DiagonalMatrix< M, M >& sing = singular::Svd< M, M >::getS(usv);
    cout << sing << endl;
    cout << endl;
    return singular::Svd< M, M >::isFullRank(sing, M);
}


static void ConvertReferenceArrayToInline(double* matrix) {
    for (int i = 0; i < 64; ++i) {
        for (int j = 0; j < 64; ++j) {
            matrix[64*i + j] = (double) reference_matrix[i][j];
        }
    }
}

BOOST_AUTO_TEST_CASE(raw_data_to_singular_matrix_conversion) {
        assert(CheckCompatibility(DATA));
}

BOOST_AUTO_TEST_CASE(compute_svd_of_a_matrix) {
        assert(CheckMatrixRank4x4(DATA));
        assert(!CheckMatrixRank4x4(WRONG_DATA1));
        assert(!CheckMatrixRank4x4(WRONG_DATA2));
}

BOOST_AUTO_TEST_CASE(check_full_rank_for_64x64) {
        double matrix[64*64];
        ConvertReferenceArrayToInline(matrix);
        assert(CheckMatrixRank64x64(matrix));
}

BOOST_AUTO_TEST_SUITE_END()
