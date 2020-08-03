#ifndef _SINGULAR_ROTATOR_H
#define _SINGULAR_ROTATOR_H

#include "matrix-utils/singular/Matrix.h"
#include "matrix-utils/singular/singular.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>

namespace singular {

	/**
	 * Rotator.
	 */
	class Rotator {
	private:
		/**
		 * 2x2 transformation matrix.
		 * The element at the ith row and jth column is given by
		 * `element[i * 2 + j]`.
		 */
		double elements[4];
	public:
		/**
		 * Builds a rotator from a given two-element vector.
		 *
		 * Builds a \f$2 \times 2\f$ rotator \f$Q\f$ such that,
		 * \f[
		 * Q^T \begin{bmatrix} x_1 \\ x_2 \end{bmatrix}
		 * = \begin{bmatrix} * \\ 0 \end{bmatrix}
		 * \f]
		 *
		 * \f$Q\f$ is given by the following formula,
		 * \f[
		 * Q = \begin{bmatrix}
		 *   \frac{x_1}{\sqrt{x_1^2 + x_2^2}} & -\frac{x_2}{\sqrt{x_1^2 + x_2^2}} \\
		 *   \frac{x_2}{\sqrt{x_1^2 + x_2^2}} & \frac{x_1}{\sqrt{x_1^2 + x_2^2}}
		 * \end{bmatrix}
		 * \f]
		 *
		 * @param x1
		 *     First element in the vector.
		 * @param x2
		 *     Second element in the vector.
		 */
		Rotator(double x1, double x2) {
			// normalizes by the maximum magnitude
			// to avoid harmful underflow and overflow
			double mx = std::max(std::abs(x1), std::abs(x2));
			x1 /= mx;
			x2 /= mx;
			double norm = sqrt(x1 * x1 + x2 * x2);
			double cs = x1 / norm;
			double sn = x2 / norm;
			this->elements[0] = cs;
			this->elements[1] = -sn;
			this->elements[2] = sn;
			this->elements[3] = cs;
		}

		/**
		 * Returns the element at given row and column.
		 *
		 * The behavior is undefined,
		 *  - if `i < 0` or `i >= 2`,
		 *  - or if `j < 0` or `j >= 2`
		 *
		 * @param i
		 *     Index of the row to be obtained.
		 * @param j
		 *     Index of the column to be obtained.
		 * @return
		 *     Element at the given row and column.
		 */
		inline double operator ()(int i, int j) const {
			assert(0 <= i && i < 2);
			assert(0 <= j && j < 2);
			return this->elements[i * 2 + j];
		}

		/**
		 * Applies this rotator from the left hand side of a given matrix.
		 *
		 * This rotator can be viewed as the following \f$M \times M\f$ matrix,
		 * \f[
		 * \begin{bmatrix}
		 *   I_1 &             &            &     \\
		 *       & \cos\theta  & \sin\theta &     \\
		 *       & -\sin\theta & \cos\theta &     \\
		 *       &             &            & I_2
		 * \end{bmatrix}
		 * \f]
		 * where
		 * \f[
		 * \begin{array}{ccl}
		 *   I_1    & : & k \times k \text{ identity matrix} \\
		 *   I_2    & : & (M-k-2) \times (M-k-2) \text{ identity matrix} \\
		 *   \theta & : & \text{angle of this rotator}
		 * \end{array}
		 * \f]
		 *
		 * The behavior is undefined if `M < k + 2`.
		 * 
		 * @tparam M
		 *     Number of the rows in the given matrix.
		 * @tparam N
		 *     Number of the columns in the given matrix.
		 * @param rhs
		 *     Matrix to be rotated.
		 * @param k
		 *     Top-left row and column index where this rotator is applied.
		 * @return
		 *     Result of this rotation.
		 */
		template < int M, int N >
		Matrix< M, N > applyFromLeftTo(const Matrix< M, N >& rhs, int k) {
			assert(M >= k + 2);
			Matrix< M, N > m = rhs.clone();
			for (int i = 0; i < N; ++i) {
				double x1 = rhs(k, i);
				double x2 = rhs(k + 1, i);
				m(k, i) = this->elements[0] * x1 + this->elements[2] * x2;
				m(k + 1, i) = this->elements[1] * x1 + this->elements[3] * x2;
			}
			return m;
		}

		/**
		 * Applies this rotator from the right hand side of a given matrix.
		 *
		 * This rotator can be viewed as the following \f$N \times N\f$ matrix,
		 * \f[
		 * \begin{bmatrix}
		 *   I_1 &            &             &     \\
		 *       & \cos\theta & -\sin\theta &     \\
		 *       & \sin\theta & \cos\theta  &     \\
		 *       &            &             & I_2
		 * \end{bmatrix}
		 * \f]
		 * where
		 * \f[
		 * \begin{array}{ccl}
		 *   I_1    & : & k \times k \text{ identity matrix} \\
		 *   I_2    & : & (N-k-2) \times (N-k-2) \text{ identity matrix} \\
		 *   \theta & : & \text{angle of this rotator}
		 * \end{array}
		 * \f]
		 *
		 * The behavior is undefined if `N < k + 2`.
		 *
		 * @tparam M
		 *     Number of the rows in the given matrix.
		 * @tparam N
		 *     Number of the columns in the given matrix.
		 * @param lhs
		 *     Matrix to be rotated.
		 * @param k
		 *     Top-left row and column index where this rotator is applied.
		 * @return
		 *     Result of this rotation.
		 */
		template < int M, int N >
		Matrix< M, N > applyFromRightTo(const Matrix< M, N >& lhs, int k) {
			assert(N >= k + 2);
			Matrix< M, N > m = lhs.clone();
			for (int i = 0; i < M; ++i) {
				double x1 = lhs(i, k);
				double x2 = lhs(i, k + 1);
				m(i, k) = x1 * this->elements[0] + x2 * this->elements[2];
				m(i, k + 1) = x1 * this->elements[1] + x2 * this->elements[3];
			}
			return m;
		}
	};

}

#endif
