#ifndef _SINGULAR_REFLECTOR_H
#define _SINGULAR_REFLECTOR_H

#include "matrix-utils/singular/Matrix.h"
#include "matrix-utils/singular/singular.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <iterator>
#include <limits>
#include <numeric>
#include <vector>

namespace singular {

	/**
	 * Reflector.
	 *
	 * A reflector transforms a vector (\f$\mathbf{x} =
	 * \begin{bmatrix} x_1 & x_2 & \dots & x_N \end{bmatrix}\f$)
	 * into a vector (\f$\mathbf{y} =
	 * \begin{bmatrix}-\tau & 0 & \dots & 0 \end{bmatrix}\f$).
	 *
	 * A reflector is equivalent to the following matrix.
	 * \f[
	 * \mathbf{H} = \mathbf{I} - \gamma \mathbf{u} \mathbf{u}^T
	 * \f]
	 *
	 * @tparam L
	 *     Size of the transform matrix.
	 */
	template < int L >
	class Reflector {
	private:
		/** U vector. */
		std::vector< double > u;

		/** Gamma. */
		double gamma;
	public:
		/**
		 * Constructs a reflector from a given vector.
		 *
		 * Forms an \f$L \times L\f$ matrix like the following,
		 * \f[
		 * \begin{bmatrix}
		 *   \mathbf{I} & \mathbf{0} \\
		 *   \mathbf{0} & \mathbf{R}
		 * \end{bmatrix}
		 * \f]
		 *
		 * \f$\mathbf{R}\f$ is an \f$N \times N\f$ reflector created from
		 * `v` where `N = v.size()`.
		 * \f$\mathbf{I}\f$ is an \f$(L-N) \times (L-N)\f$ identity matrix.
		 *
		 * The behavior is undefined if `v.size() == 0` or `v.size() > L`.
		 *
		 * @param v
		 *     Vector from which the reflector is formed.
		 * @throws Exception
		 *     If `v.size() == 0`, or if `v.size() > L`.
		 */
		Reflector(const Vector< const double >& v) {
			assert(v.size() > 0 && v.size() <= L);
			const size_t N = v.size();
			// copies the vector
			this->u.reserve(N);
			std::copy(v.begin(), v.end(), std::back_inserter(this->u));
			// normalizes elements by the maximum amplitude
			// to avoid harmful underflow and overflow
			double mx = 0.0;
			for (size_t i = 0; i < N; ++i) {
				mx = std::max(std::abs(this->u[i]), mx);
			}
			if (mx > 0.0) {
				// calculates the normalized norm
				double tau = 0.0;
				for (size_t i = 0; i < N; ++i) {
					double x = this->u[i] / mx;
					this->u[i] = x;
					tau += x * x;
				}
				tau = sqrt(tau);
				// tau's sign should be the same as the first element in `u`
				if (this->u[0] < 0.0) {
					tau = -tau;
				}
				double u0 = this->u[0] + tau;
				this->u[0] = u0;
				std::transform(this->u.begin(), this->u.end(), this->u.begin(),
					[u0] (double& e) { return e / u0; });
				this->gamma = u0 / tau;
			} else {
				// v is a zero vector
				this->gamma = 0.0;
				std::fill(this->u.begin(), this->u.end(), 0.0);
			}
		}

		/**
		 * Applies this reflector to a given matrix from left.
		 *
		 * @tparam N
		 *     Number of columns in the given matrix.
		 * @param m
		 *     Matrix to be transformed.
		 * @return
		 *     Transformed matrix.
		 */
		template < int N >
		Matrix< L, N > applyFromLeftTo(const Matrix< L, N >& m) const {
			// H * m = m - gamma * u * u^T * m
			Matrix< L, N > m2 = m.clone();
			int offset = L - u.size();
			for (int i = 0; i < N; ++i) {
				// caches gamma * u^T * m
				Vector< const double > srcColumn = m.column(i).slice(offset);
				double gUM = std::inner_product(
					this->u.begin(), this->u.end(), srcColumn.begin(), 0.0);
				gUM *= this->gamma;
				// H * m = m - u * gUM
				Vector< double > dstColumn = m2.column(i).slice(offset);
				std::transform(
					this->u.begin(), this->u.end(), srcColumn.begin(),
					dstColumn.begin(),
					[gUM](double a, double b) {
						return b - a * gUM;
					});
			}
			return m2;
		}

		/**
		 * Applies this reflector to a given matrix from right.
		 *
		 * @tparam M
		 *     Number of rows in the given matrix.
		 * @param m
		 *     Matrix to be transformed.
		 * @return
		 *     Transformed matrix.
		 */
		template < int M >
		Matrix< M, L > applyFromRightTo(const Matrix< M, L >& m) const {
			// m * H = m - m * gamma * u * u^T
			Matrix< M, L > m2 = m.clone();
			int offset = L - u.size();
			for (int i = 0; i < M; ++i) {
				// caches gamma * m * u
				Vector< const double > srcRow = m.row(i).slice(offset);
				double gMU = std::inner_product(
					this->u.begin(), this->u.end(), srcRow.begin(), 0.0);
				gMU *= this->gamma;
				// m * H = m - gMU * u^T
				Vector< double > dstRow = m2.row(i).slice(offset);
				std::transform(
					this->u.begin(), this->u.end(), srcRow.begin(),
					dstRow.begin(),
					[gMU](double a, double b) {
						return b - gMU * a;
					});
			}
			return m2;
		}
	};

}

#endif
