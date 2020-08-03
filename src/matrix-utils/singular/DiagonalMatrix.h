#ifndef _SINGULAR_DIAGONAL_MATRIX_H
#define _SINGULAR_DIAGONAL_MATRIX_H

#include "matrix-utils/singular/singular.h"

#include <algorithm>
#include <cassert>
#include <cstring>

namespace singular {

	/**
	 * Diagonal matrix.
	 */
	template < int M, int N >
	class DiagonalMatrix {
	public:
		enum {
			/** Number of diagonal elements. */
			L = M < N ? M : N
		};
	private:
		/**
		 * Memory block for the diagonal elements.
		 * The ith row and ith column is given by `elements[i]`.
		 */
		double* pBlock;
	public:
		/** Initializes a diagonal matrix filled with 0. */
		DiagonalMatrix() {
			this->pBlock = new double[L];
			std::fill(this->pBlock, this->pBlock + L, 0.0);
		}

		/**
		 * Initializes a diagonal matrix with given diagonal values.
		 *
		 * The diagonal matrix will look like,
		 * \f[
		 * \begin{bmatrix}
		 *   \text{values[0]} &        &                            \\
		 *                    & \ddots &                            \\
		 *                    &        & \text{values[min(M, N)-1]}
		 * \end{bmatrix}
		 * \f]
		 *
		 * The behavior is undefined if `values` has less than `min(M, N)`
		 * elements.
		 *
		 * @param values
		 *     Diagonal values of the matrix.
		 */
		explicit DiagonalMatrix(const double values[]) {
			this->pBlock = new double[L];
			memcpy(this->pBlock, values, sizeof(double) * L);
		}

		/**
		 * Steals the memory block from a given diagonal matrix.
		 *
		 * @param[in,out] copyee
		 *     Diagonal matrix from which the memory block is to be stolen.
		 *     No loger valid after this call.
		 */
#if SINGULAR_RVALUE_REFERENCE_SUPPORTED
		DiagonalMatrix(DiagonalMatrix&& copyee) : pBlock(copyee.pBlock) {
			copyee.pBlock = nullptr;
		}
#else
		DiagonalMatrix(const DiagonalMatrix& copyee) : pBlock(copyee.pBlock) {
			const_cast< DiagonalMatrix& >(copyee).pBlock = nullptr;
		}
#endif

		/** Releases the memory block of this diagonal matrix. */
		~DiagonalMatrix() {
			this->release();
		}

		/**
		 * Steals the memory block from a given diagonal matrix.
		 *
		 * @param[in,out] copyee
		 *     Diagonal matrix from which the memory block is to be stolen.
		 *     No longer valid after this call.
		 * @return
		 *     Reference to this diagonal matrix.
		 */
#if SINGULAR_RVALUE_REFERENCE_SUPPORTED
		DiagonalMatrix& operator =(DiagonalMatrix&& copyee) {
#else
		DiagonalMatrix& operator =(const DiagonalMatrix& copyee) {
#endif
			this->release();
			this->pBlock = copyee.pBlock;
#if SINGULAR_RVALUE_REFERENCE_SUPPORTED
			copyee.pBlock = nullptr;
#else
			const_cast< DiagonalMatrix& >(copyee).pBlock = nullptr;
#endif
			return *this;
		}

		/**
		 * Returns a clone of this matrix.
		 *
		 * @return
		 *     Clone of this matrix.
		 */
		inline DiagonalMatrix clone() const {
			return DiagonalMatrix(this->pBlock);
		}

		/**
		 * Returns the element at a given row and column.
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
		 *     Element at the ith row and jth column.
		 *     0 if `i != j`.
		 */
		double operator ()(int i, int j) const {
			assert(i >= 0 && i < M);
			assert(j >= 0 && j < N);
			if (i == j) {
				return this->pBlock[i];
			} else {
				return 0.0;
			}
		}

		/**
		 * Transposes this matrix.
		 *
		 * @return
		 *     Transposed matrix.
		 */
		DiagonalMatrix< N, M > transpose() const {
			return DiagonalMatrix< N, M >(this->pBlock);
		}
	private:
#if SINGULAR_FUNCTION_DELETION_SUPPORTED
		/** Copy constructor is not allowed. */
		DiagonalMatrix(const DiagonalMatrix& copyee) = delete;

		/** Copy assignment is not allowed. */
		DiagonalMatrix& operator =(const DiagonalMatrix& copyee) = delete;
#elif SINGULAR_RVALUE_REFERENCE_SUPPORTED
		/** Copy constructor is not allowed. */
		DiagonalMatrix(const DiagonalMatrix& copyee) {}

		/** Copy assignment is not allowed. */
		DiagonalMatrix& operator =(const DiagonalMatrix& copyee) {
			return *this;
		}
#endif

		/**
		 * Releases the memory block of this matrix.
		 * Has no effect if the memory block has already been released.
		 */
		inline void release() {
			delete[] this->pBlock;
			this->pBlock = nullptr;
		}
	};

}

#endif
