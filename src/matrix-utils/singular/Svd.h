#ifndef _SINGULAR_SVD_H
#define _SINGULAR_SVD_H

#include "matrix-utils/singular/DiagonalMatrix.h"
#include "matrix-utils/singular/Matrix.h"
#include "matrix-utils/singular/Reflector.h"
#include "matrix-utils/singular/Rotator.h"
#include "matrix-utils/singular/singular.h"

#include <algorithm>
#include <cassert>
#include <tuple>

namespace singular {

	/**
	 * Namespace for singular value decomposition.
	 *
	 * @tparam M
	 *     Number of rows in an input matrix.
	 * @tparam N
	 *     Number of columns in an input matrix.
	 */
	template < int M, int N >
	struct Svd {
		/**
		 * Tuple of left singular vectors, singular values and right singular
		 * vectors.
		 *
		 * Use `getU`, `getS` and `getV` instead of `std::get` to access items.
		 */
		typedef std::tuple< Matrix< M, M >,
							DiagonalMatrix< M, N >,
							Matrix< N, N > > USV;

		/** Returns the left-singular-vectors from a given `USV` tuple. */
		static inline const Matrix< M, M >& getU(const USV& usv) {
			return std::get< 0 >(usv);
		}

		/** Returns the singular values from a given `USV` tuple. */
		static inline const DiagonalMatrix< M, N >& getS(const USV& usv) {
			return std::get< 1 >(usv);
		}

        /** Checks if the matrix is full-rank */
        static inline bool isFullRank(const DiagonalMatrix< M, N >& singularValues, const int size) {
            const double round_off = 1.000009e-12;
            for (int i = 0; i < size; ++i) {
                if (std::abs( singularValues(i, i) ) < round_off)
                    return false;
            }
            return true;
        }

		/** Returns the right-singular-vectors from a given `USV` tuple. */
		static inline const Matrix< N, N >& getV(const USV& usv) {
			return std::get< 2 >(usv);
		}

		/**
		 * Decomposes a given matrix into left singular vectors,
		 * singular values and right singular vectors.
		 *
		 * \f[
		 * \mathbf{A} = \mathbf{U} \mathbf{\Sigma} \mathbf{V}^T
		 * \f]
		 * \f[
		 * \begin{array}{ccl}
		 *   \mathbf{A} & : & \text{matrix to be decomposed; i.e., m} \\
		 *   \mathbf{U} & : & \text{left-singular-vectors given as an orthonormal matrix} \\
		 *   \mathbf{S} & : & \text{singular values given as a diagonal matrix} \\
		 *   \mathbf{V} & : & \text{right-singular-vectors as an orthonormal matrix}
		 * \end{array}
		 * \f]
		 *
		 * @param m
		 *     `M` x `N` matrix to be decomposed.
		 * @return
		 *     Decomposition of `m`.
		 * @see getU
		 * @see getS
		 * @see getV
		 */
		static USV decomposeUSV(const Matrix< M, N >& m) {
			// makes sure that M >= N
			// otherwise decomposes the transposed matrix
			if (M < N) {
				// A^T = V * S^T * U^T
				typename Svd< N, M >::USV usvT =
					Svd< N, M >::decomposeUSV(m.transpose());
				return std::make_tuple(
					std::move(std::get< 2 >(usvT)),
					std::get< 1 >(usvT).transpose(),
					std::move(std::get< 0 >(usvT)));
			}
			const int MAX_ITERATIONS = N * 10;
			// allocates matrices
			Matrix< M, M > u = Matrix< M, M >::identity();
			// Matrix< M, N > s = m.clone();
			Matrix< N, N > v = Matrix< N, N >::identity();
			// bidiagonalizes a given matrix
			BidiagonalMatrix m2 = bidiagonalize(u, m.clone(), v);
			// repeats Francis iteration
			int iteration = 0;
			int n = N;
			while (n >= 2) {
				// processes the n-1 x n-1 submatrix
				// if the current n x n submatrix has converged
				double bn = m2(n - 1, n - 1);
				if (bn == 0.0 || std::abs(m2(n - 2, n - 1) / bn) < 1.0e-15) {
					--n;
				} else {
					// aborts if too many iterations
					++iteration;
					if (iteration > MAX_ITERATIONS) {
						break;
					}
					doFrancis(u, m2, v, n);
				}
			}
			// copies the diagonal elements
			// and makes all singular values positive
			double ss[N];
			for (int i = 0; i < N; ++i) {
				if (m2(i, i) < 0) {
					ss[i] = -m2(i, i);
					// inverts the sign of the right singular vector
					Vector< double > vi = v.column(i);
					std::transform(
						vi.begin(), vi.end(), vi.begin(),
						[](double x) {
							return -x;
						});
				} else {
					ss[i] = m2(i, i);
				}
			}
			// sorts singular values in descending order if necessary
			int shuffle[M];  // M >= N
			bool sortNeeded = false;
			for (int i = 0; i < M; ++i) {
				shuffle[i] = i;
				sortNeeded = sortNeeded || (i < N - 1 && ss[i] < ss[i + 1]);
			}
			if (sortNeeded) {
				// shuffles the N (<= M) singular values
				std::sort(shuffle, shuffle + N, [&ss](int i, int j) {
					return ss[i] > ss[j];  // descending order
				});
				double ss2[M];
				std::transform(shuffle, shuffle + N, ss2, [&ss](int i) {
					return ss[i];
				});
				return std::make_tuple(u.shuffleColumns(shuffle),
									   DiagonalMatrix< M, N >(ss2),
									   v.shuffleColumns(shuffle));
			} else {
				return std::make_tuple(std::move(u),
									   DiagonalMatrix< M, N >(ss),
									   std::move(v));
			}
		}
	private:
		/**
		 * M x N bidiagonal matrix.
		 *
		 * If `M >= N`, a bidiagonal matrix looks like,
		 * \f[
		 * \mathbf{A} = \begin{bmatrix}
		 *   \beta_1 & \gamma_1 &          &             &              \\
		 *           & \beta_2  & \gamma_2 &             &              \\
		 *           &          & \ddots   & \ddots      &              \\
		 *           &          &          & \beta_{N-1} & \gamma_{N-1} \\
		 *           &          &          &             & \beta_N
		 * \end{bmatrix}
		 * \f]
		 *
		 * The behavior is undefined if `M < N`.
		 */
		class BidiagonalMatrix {
		private:
			/**
			 * Memory block for the diagonal elements.
			 * The number of elements is `2 * N - 1`
			 *
			 * The ith diagonal element is given by `pBlock[i * 2]`.
			 * The ith upper-diagonal element is given by `pBlock[i * 2 + 1]`.
			 */
			double* pBlock;
		public:
			/**
			 * Initializes from bidiagonal elements of a given matrix.
			 *
			 * **Only bidiagonal elements are taken from `m` whether it is
			 * bidiagonal or not.**
			 *
			 * The behavior is undefined if `M < N`.
			 *
			 * @param m
			 *     Matrix from which bidiagonal elements are to be taken.
			 */
			BidiagonalMatrix(const Matrix< M, N >& m) {
				assert(M >= N);
				this->pBlock = new double[2 * N - 1];
				for (int i = 0; i < N; ++i) {
					this->pBlock[i * 2] = m(i, i);
					if (i < N - 1) {
						this->pBlock[i * 2 + 1] = m(i, i + 1);
					}
				}
			}

			/**
			 * Steals the memory block from a given bidiagonal matrix.
			 *
			 * @param[in,out] copyee
			 *     Bidiagonal matrix from which the memory block is to be
			 *     stolen.
			 *     No longer valid after this call.
			 */
#if SINGULAR_RVALUE_REFERENCE_SUPPORTED
			inline BidiagonalMatrix(BidiagonalMatrix&& copyee)
				: pBlock(copyee.pBlock)
			{
				copyee.pBlock = nullptr;
			}
#else
			inline BidiagonalMatrix(const BidiagonalMatrix& copyee)
				: pBlock(copyee.pBlock)
			{
				const_cast< BidiagonalMatrix& >(copyee).pBlock = nullptr;
			}
#endif

			/** Releases the memory block for bidiagonal elements. */
			inline ~BidiagonalMatrix() {
				this->releaseBlock();
			}

			/**
			 * Returns the element at given row and column.
			 *
			 * Values are 0 unless `i == j` or `i == j + 1`.
			 *
			 * The behavior is undefined,
			 *  - if `i < 0` or `i >= M`,
			 *  - or if `j < 0` or `j >= N`
			 *
			 * @param i
			 *     Index of the row to be obtained.
			 * @param j
			 *     Index of the column to be obtained.
			 * @return
			 *     Element at the given row and column.
			 */
			double operator ()(int i, int j) const {
				assert(i >= 0 && i < M);
				assert(j >= 0 && j < N);
				if (i == j) {
					return this->pBlock[2 * i];
				} else if (i + 1 == j) {
					return this->pBlock[2 * i + 1];
				} else {
					return 0.0;
				}
			}

			/**
			 * Applies a given rotator from right-hand-side of this bidiagonal
			 * matrix at the first time.
			 *
			 * Works like the following,
			 * \f[
			 * \begin{bmatrix}
			 *   * & * &   &   &        \\
			 *     & * & * &   &        \\
			 *     &   & * & * &        \\
			 *     &   &   & * & \ddots \\
			 *     &   &   &   & \ddots
			 * \end{bmatrix}
			 * \begin{bmatrix}
			 *   \mathbf{Q} &            \\
			 *              & \mathbf{I}
			 * \end{bmatrix}
			 * \to
			 * \begin{bmatrix}
			 *   * & * &   &   &        \\
			 *   + & * & * &   &        \\
			 *     &   & * & * &        \\
			 *     &   &   & * & \ddots \\
			 *     &   &   &   & \ddots
			 * \end{bmatrix}
			 * \f]
			 * where
			 * \f[
			 * \begin{array}{ccl}
			 * \mathbf{Q} & : & 2 \times 2 \text{ rotator} \\
			 * \mathbf{I} & : & (N-2) \times (N-2) \text{ identity matrix} \\
			 * +          & : & \text{bulge}
			 * \end{array}
			 * \f]
			 *
			 * The behavior is undefined if `N < 2`.
			 *
			 * @param r
			 *     Rotator to be applied from right-hand-side of this bidiagonal
			 *     matrix.
			 * @return
			 *     Bulge made at (1, 0).
			 */
			double applyFirstRotatorFromRight(const Rotator& r) {
				double b1 = this->pBlock[0];
				double g1 = this->pBlock[1];
				double b2 = this->pBlock[2];
				double r11 = r(0, 0);
				double r12 = r(0, 1);
				double r21 = r(1, 0);
				double r22 = r(1, 1);
				this->pBlock[0] = b1 * r11 + g1 * r21;
				this->pBlock[1] = b1 * r12 + g1 * r22;
				this->pBlock[2] = b2 * r22;
				return b2 * r21;
			}

			/**
			 * Applies a given rotator from right-hand-side of this bidiagonal
			 * matrix.
			 *
			 * Works like the following,
			 * \f[
			 * \begin{bmatrix}
			 *   \ddots & \ddots &   &   &   &        \\
			 *          & *      & * & + &   &        \\
			 *          &        & * & * &   &        \\
			 *          &        &   & * & * &        \\
			 *          &        &   &   & * & \ddots \\
			 *          &        &   &   &   & \ddots
			 * \end{bmatrix}
			 * \begin{bmatrix}
			 *   \mathbf{I}_1 &            &              \\
			 *                & \mathbf{Q} &              \\
			 *                &            & \mathbf{I}_2
			 * \end{bmatrix}
			 * \to
			 * \begin{bmatrix}
			 *   \ddots & \ddots &   &   &   &        \\
			 *          & *      & * &   &   &        \\
			 *          &        & * & * &   &        \\
			 *          &        & + & * & * &        \\
			 *          &        &   &   & * & \ddots \\
			 *          &        &   &   &   & \ddots
			 * \end{bmatrix}
			 * \f]
			 * where
			 * \f[
			 * \begin{array}{ccl}
			 *   \mathbf{Q}   & : & 2 \times 2 \text{ rotator} \\
			 *   \mathbf{I}_1 & : & n \times n \text{ identity matrix} \\
			 *   \mathbf{I}_2 & : & (N-n-2) \times (N-n-2) \text{ identity matrix} \\
			 *   +            & : & \text{bulge}
			 * \end{array}
			 * \f]
			 *
			 * The behavior is undefined if `n <= 0` or `n + 1 >= N`.
			 *
			 * @param r
			 *     Rotator to be applied from right-hand-side of this bidiagonal
			 *     matrix.
			 * @param n
			 *     Index of the column where a new bulge is to be made.
			 * @param bulge
			 *     Bulge at (n - 1, n + 1).
			 * @return
			 *     Bulge made at (n + 1, n).
			 */
			double applyRotatorFromRight(
				const Rotator& r, int n, double bulge)
			{
				double* p = this->pBlock + n * 2;
				double g0 = p[-1];
				double b1 = p[0];
				double g1 = p[1];
				double b2 = p[2];
				double r11 = r(0, 0);
				double r12 = r(0, 1);
				double r21 = r(1, 0);
				double r22 = r(1, 1);
				p[-1] = g0 * r11 + bulge * r21;
				p[0] = b1 * r11 + g1 * r21;
				p[1] = b1 * r12 + g1 * r22;
				p[2] = b2 * r22;
				return b2 * r21;
			}

			/**
			 * Applies a given rotator from left-hand-side of this bidiagonal
			 * matrix.
			 *
			 * Works like the following,
			 * \f[
			 * \begin{bmatrix}
			 *   \mathbf{I}_1 &              &              \\
			 *                & \mathbf{Q}^T &              \\
			 *                &              & \mathbf{I}_2
			 * \end{bmatrix}
			 * \begin{bmatrix}
			 *   \ddots & \ddots &   &   &   &        \\
			 *          & *      & * &   &   &        \\
			 *          &        & * & * &   &        \\
			 *          &        & + & * & * &        \\
			 *          &        &   &   & * & \ddots \\
			 *          &        &   &   &   & \ddots
			 * \end{bmatrix}
			 * \to
			 * \begin{bmatrix}
			 *   \ddots & \ddots &   &   &   &        \\
			 *          & *      & * &   &   &        \\
			 *          &        & * & * & + &        \\
			 *          &        &   & * & * &        \\
			 *          &        &   &   & * & \ddots \\
			 *          &        &   &   &   & \ddots
			 * \end{bmatrix}
			 * \f]
			 * where
			 * \f[
			 * \begin{array}{ccl}
			 *   \mathbf{Q}   & : & 2 \times 2 \text{ rotator} \\
			 *   \mathbf{I}_1 & : & n \times n \text{ identity matrix} \\
			 *   \mathbf{I}_2 & : & (N-n-2) \times (N-n-2) \text{ identity matrix} \\
			 *   +            & : & \text{bulge}
			 * \end{array}
			 * \f]
			 *
			 * The behavior is undefined if `n + 1 >= N`.
			 *
			 * @param r
			 *     Rotator to be applied from left-hand-side of this bidiagonal
			 *     matrix.
			 * @param n
			 *     Index of the row where a new bulge is to be made.
			 * @param bulge
			 *     Bulge at (n + 1, n).
			 * @return
			 *     Bulge made at (n, n + 2).
			 *     0.0 if `n + 2 >= N`.
			 */
			double applyRotatorFromLeft(const Rotator& r, int n, double bulge) {
				double* p = this->pBlock + n * 2;
				double b1 = p[0];
				double g1 = p[1];
				double b2 = p[2];
				double r11 = r(0, 0);
				double r12 = r(0, 1);
				double r21 = r(1, 0);
				double r22 = r(1, 1);
				p[0] = r11 * b1 + r21 * bulge;
				p[1] = r11 * g1 + r21 * b2;
				p[2] = r12 * g1 + r22 * b2;
				double newBulge;
				if (n < N - 2) {
					double g2 = p[3];
					newBulge = r21 * g2;
					p[3] = r22 * g2;
				} else {
					newBulge = 0.0;
				}
				return newBulge;
			}
		private:
#if SINGULAR_FUNCTION_DELETION_SUPPORTED
			/** Simple copy is forbidden. */
			BidiagonalMatrix(const BidiagonalMatrix& copyee) = delete;

			/** Simple assignment is forbidden. */
			void operator =(const BidiagonalMatrix& copyee) = delete;
#elif SINGULAR_RVALUE_REFERENCE_SUPPORTED
			/** Simple copy is forbidden. */
			BidiagonalMatrix(const BidiagonalMatrix& copyee) {}

			/** Simple assignment is forbidden. */
			void operator =(const BidiagonalMatrix& copyee) {}
#endif

			/** Releases the memory block for bidiagonal elements. */
			inline void releaseBlock() {
				delete[] this->pBlock;
				this->pBlock = nullptr;
			}
		};
	private:
		/**
		 * Bindiagonalizes a given matrix.
		 *
		 * `M` must be greater than or equal to `N`.
		 * The behavior is undefined if `M < N`.
		 *
		 * @param[in,out] u
		 *     Left-singular-vectors to be upated.
		 * @param[in,out] m
		 *     Matrix to be bidiagonalized.
		 * @param[in,out] v
		 *     Right-singular-vectors to be updated.
		 * @return
		 *     Bidiagonal matrix built from `m`.
		 */
		static BidiagonalMatrix bidiagonalize(Matrix< M, M >& u,
											  Matrix< M, N > m,
											  Matrix< N, N >& v)
		{
			assert(M >= N);
			for (int i = 0; i < N; ++i) {
				// applies a householder transform to the column vector i
				Reflector< M > rU(m.column(i).slice(i));
				m = rU.applyFromLeftTo(m);
				u = rU.applyFromRightTo(u);  // U1^T*U0^T = U0*U1
				if (i < N - 1) {
					// applies a householder transform to the row vector i + 1
					Reflector< N > rV(m.row(i).slice(i + 1));
					m = rV.applyFromRightTo(m);
					v = rV.applyFromRightTo(v);
				}
			}
			return BidiagonalMatrix(m);
		}

		/**
		 * Performs a single Francis iteration.
		 *
		 * Submatrices other than the top-left `n` x `n` submatrix of `m` are
		 * regarded as already converged.
		 *
		 * The behavior is undefined,
		 *  - if `M < N`,
		 *  - or if `n < 2`
		 *
		 * @param[in,out] u
		 *     Left-singular-vectors to be updated.
		 * @param[in,out] m
		 *     Bidiagonalized input matrix where diagonal elements are to be
		 *     singular values after convergence.
		 * @param[in,out] v
		 *     Right-singular-vectors to be updated.
		 * @param n
		 *     Size of the submatrix over which the Francis iteration is to be
		 *     performed.
		 *     Must be gerater than or equal to 2.
		 */
		static void doFrancis(Matrix< M, M >& u,
							  BidiagonalMatrix& m,
							  Matrix< N, N >& v,
							  int n)
		{
			assert(M >= N);
			assert(n >= 2);
			// calculates the shift
			double rho = calculateShift(m, n);
			// applies the first right rotator
			double b1 = m(0, 0);
			double g1 = m(0, 1);
			double mx =
				std::max(std::abs(rho), std::max(std::abs(b1), std::abs(g1)));
			rho /= mx;
			b1 /= mx;
			g1 /= mx;
			Rotator r0(b1 * b1 - rho * rho, b1 * g1);
			double bulge = m.applyFirstRotatorFromRight(r0);
			v = r0.applyFromRightTo(v, 0);
			// applies the first left rotator
			Rotator r1(m(0, 0), bulge);
			bulge = m.applyRotatorFromLeft(r1, 0, bulge);
			u = r1.applyFromRightTo(u, 0);  // U1^T*U0^T = U0*U1
			for (int i = 1; i + 1 < n; ++i) {
				// calculates (i+1)-th right rotator
				Rotator rV(m(i - 1, i), bulge);
				bulge = m.applyRotatorFromRight(rV, i, bulge);
				v = rV.applyFromRightTo(v, i);
				// calculates (i+1)-th left rotator
				Rotator rU(m(i, i), bulge);
				bulge = m.applyRotatorFromLeft(rU, i, bulge);
				u = rU.applyFromRightTo(u, i);  // U1^T*U0^T = U0*U1
			}
		}

		/**
		 * Calculates the shift for a given bidiagonal matrix.
		 *
		 * Submatrices other than top-left `n` x `n` submatrix of `m` are
		 * regarded as already converged.
		 *
		 * The behavior is undefined,
		 *  - if `M < N`,
		 *  - or if `n < 2`
		 *
		 * @param m
		 *     Bidiagonal matrix from which a shift is to be calculated.
		 * @param n
		 *     Size of the submatrix to be considered. 
		 * @return
		 *     Shift for the top-left `n` x `n` submatrix of `m`.
		 */
		static double calculateShift(const BidiagonalMatrix& m, int n) {
			assert(M >= N);
			assert(n >= 2);
			double b1 = m(n - 2, n - 2);
			double b2 = m(n - 1, n - 1);
			double g1 = m(n - 2, n - 1);
			// solves lambda^4 - d*lambda^2 + e = 0
			// where
			//  d = b1^2 + b2^2 + g1^2
			//  e = b1^2 * b2^2
			// chooses lambda (rho) closest to b2
			double rho;
			double d = b1 * b1 + b2 * b2 + g1 * g1;
			double e = b1 * b1 * b2 * b2;
			// lambda^2 = (d +- sqrt(d^2 - 4e)) / 2
			// so, f = d^2 - 4e must be positive
			double f = d * d - 4 * e;
			if (f >= 0) {
				f = sqrt(f);
				// lambda = +-sqrt(d +- f)  (d >= 0, f >= 0)
				// if d > f, both d+f and d-f have real square roots
				// otherwise considers only d+f
				if (d > f) {
					// lets l1 > l2
					double l1 = sqrt((d + f) * 0.5);
					double l2 = sqrt((d - f) * 0.5);
					// if b2 >= 0, chooses a positive shift
					// otherwise chooses a negative shift
					if (b2 >= 0) {
						if (std::abs(b2 - l1) < std::abs(b2 - l2)) {
							rho = l1;
						} else {
							rho = l2;
						}
					} else {
						if (std::abs(b2 + l1) < std::abs(b2 + l2)) {
							rho = -l1;
						} else {
							rho = -l2;
						}
					}
				} else {
					double l1 = sqrt((d + f) * 0.5);
					if (std::abs(b2 - l1) <= std::abs(b2 + l1)) {
						rho = l1;
					} else {
						rho = -l1;
					}
				}
			} else {
				// no solution. chooses b2 as the shift
				rho = b2;
			}
			return rho;
		}
	};

}

#endif
