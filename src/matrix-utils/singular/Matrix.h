#ifndef _SINGULAR_MATRIX_H
#define _SINGULAR_MATRIX_H

#include "matrix-utils/singular/singular.h"
#include "matrix-utils/singular/Vector.h"
#include "matrix-utils/singular/Vector.h"

#include <algorithm>
#include <cstring>
#include <iostream>

namespace singular {

	/**
	 * `M x N` matrix.
	 *
	 * @tparam M
	 *     Number of rows.
	 * @tparam N
	 *     Number of columns.
	 */
	template <int M, int N >
	class Matrix {
	private:
		/**
		 * Memory block for this matrix.
		 *
		 * Element at the row `i` and column `j` is given by
		 * `pBlock[i * N + j]`.
		 */
		double* pBlock;

#if SINGULAR_TEMPLATE_FRIEND_OPERATOR_OVERLOADING_SUPPORTED
		// transposed Matrix is a friend
		friend class Matrix< N, M >;
#else
		// every Matrix should be a friend
		// because some workarounds need private members
		template < int, int >
		friend class Matrix;
#endif
	public:
		/** Initializes a matrix filled with zeros. */
		Matrix() {
			this->pBlock = new double[M * N];
			std::fill(this->pBlock, this->pBlock + (M * N), 0.0);
		}

		/**
		 * Steals the memory block from a given matrix.
		 *
		 * @param[in,out] copyee
		 *     Matrix from which the memory block is to be stolen.
		 *     No longer valid after this call.
		 */
#if SINGULAR_RVALUE_REFERENCE_SUPPORTED
		Matrix(Matrix&& copyee) : pBlock(copyee.pBlock) {
			copyee.pBlock = 0;
		}
#else
		Matrix(const Matrix& copyee) : pBlock(copyee.pBlock) {
			const_cast< Matrix& >(copyee).pBlock = 0;
		}
#endif

		/** Releases the allocated block. */
		~Matrix() {
			this->release();
		}

		/**
		 * Steals the memory block from a given matrix.
		 *
		 * @param[in,out] copyee
		 *     Matrix from which the memory block is to be stolen.
		 *     No longer valid after this call.
		 * @return
		 *     Reference to this matrix.
		 */
#if SINGULAR_RVALUE_REFERENCE_SUPPORTED
		Matrix& operator =(Matrix&& copyee) {
#else
		Matrix& operator =(const Matrix& copyee) {
#endif
			this->release();
			this->pBlock = copyee.pBlock;
#if SINGULAR_RVALUE_REFERENCE_SUPPORTED
			copyee.pBlock = 0;
#else
			const_cast< Matrix& >(copyee).pBlock = 0;
#endif
			return *this;
		}

		/**
		 * Creates a clone of this matrix.
		 *
		 * A clone has the same contents of this matrix but an independent
		 * memory block from this matrix.
		 *
		 * @return
		 *     Clone of this matrix.
		 */
		Matrix clone() const {
			double* pBlock = new double[M * N];
			std::copy(this->pBlock, this->pBlock + M * N, pBlock);
			return Matrix(pBlock);
		}

		/**
		 * Creates an identity matrix.
		 *
		 * @return
		 *     Identity matrix.
		 */
		static Matrix identity() {
			const int L = M < N ? M : N;
			Matrix eye;
			double* pDst = eye.pBlock;
			for (int i = 0; i < L; ++i) {
				*pDst = 1;
				pDst += N + 1;
			}
			return eye;
		}

		/**
		 * Creates a matrix filled with given values.
		 *
		 * The value at the ith row and jth column is taken from
		 * `values[M * i + j]`.
		 *
		 * The behavior is undefined if `values` has less than `M * N` elements.
		 *
		 * @param values
		 *     Values to fill the matrix.
		 * @return
		 *     Matrix filled with `values`.
		 */
		static Matrix filledWith(const double values[]) {
			double* pBlock = new double[M * N];
			memcpy(pBlock, values, sizeof(double) * M * N);
			return Matrix(pBlock);
		}

		/**
		 * Returns the value at a given row and column.
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
		 *     Changes on a returned element is reflected to this matrix.
		 */
		inline double& operator ()(int i, int j) {
			assert(i >= 0 && i < M);
			assert(j >= 0 && j < N);
			return this->pBlock[i * N + j];
		}

		/**
		 * Returns the value at a given row and column.
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
		 */
		inline double operator ()(int i, int j) const {
			assert(i >= 0 && i < M);
			assert(j >= 0 && j < N);
			return this->pBlock[i * N + j];
		}

		/**
		 * Returns a given row in this matrix as a modifiable vector.
		 *
		 * @param i
		 *     Index of the row to be obtained.
		 * @return
		 *     ith row as a vector.
		 *     Changes on this vector are reflected to this matrix.
		 */
		Vector< double > row(int i) {
			return Vector< double >(this->pBlock + i * N, N, 1);
		}

		/**
		 * Returns a given row in this matrix as an unmodifiable vector.
		 *
		 * @param i
		 *     Index of the row to be obtained.
		 * @return
		 *     ith row as a vector.
		 */
		Vector< const double > row(int i) const {
			return Vector< const double >(this->pBlock + i * N, N, 1);
		}

		/**
		 * Returns a given column in this matrix as a modifiable vector.
		 *
		 * @param j
		 *     Index of the column to be obtained.
		 * @return
		 *     jth column as a vector.
		 *     Changes on this vector are reflected to this matrix.
		 */
		Vector< double > column(int j) {
			return Vector< double >(this->pBlock + j, M, N);
		}

		/**
		 * Returns a given column in this matrix as an unmodifiable vector.
		 *
		 * @param j
		 *     Index of the column to be obtained.
		 * @return
		 *     jth column as a vector.
		 */
		Vector< const double > column(int j) const {
			return Vector< const double >(this->pBlock + j, M, N);
		}

		/**
		 * Fills this matrix with given values.
		 *
		 * The value at the ith row and jth column is taken from
		 * `values[i * N + j]`.
		 *
		 * The behavior is undefined if `values` has less than `M * N` elements.
		 *
		 * @param values
		 *     Values to fill this matrix.
		 */
		Matrix& fill(const double values[]) {
			std::copy(values, values + M * N, this->pBlock);
			return *this;
		}

		// Defined outside
		template < int M2, int N2, int L >
		friend Matrix< M2, L > operator *(const Matrix< M2, N2 >& lhs,
										  const Matrix< N2, L >& rhs);

#if SINGULAR_TEMPLATE_FRIEND_OPERATOR_OVERLOADING_SUPPORTED
		// Defined outside
		template <
			int M2, int N2, int L, template < int, int > class MatrixLike >
		friend Matrix< M2, L > operator *(const Matrix< M2, N2 >& lhs,
										  const MatrixLike< N2, L >& rhs);

		// Defined outside
		template <
			int M2, int N2, int L, template < int, int > class MatrixLike >
		friend Matrix< M2, L > operator *(const MatrixLike< M2, N2 >& lhs,
										  const Matrix< N2, L >& rhs);

		// Defined outside
		template <
			int M2, int N2, int L,
			template < int, int > class MatrixLike1,
			template < int, int > class MatrixLike2 >
		friend Matrix< M2, L > operator *(const MatrixLike1< M2, N2 >& lhs,
										  const MatrixLike2< N2, L >& rhs);
#else
		// Visual Studio 2012 does not like a friend function has templates
		// in its type parameters

		/**
		 * Multiplies given two matrices.
		 *
		 * `MatrixLike` must overload the function-call operator `()` which
		 * takes a row index `i` and column index `j` and returns the element at
		 * the ith row and jth column.
		 * The function prototype should look like the following,
		 *  - `double operator ()(int i, int j) const`
		 *
		 * @tparam M2
		 *     Number of rows in the left-hand-side matrix.
		 * @tparam N2
		 *     Number of columns in the left-hand-side matrix.
		 *     Number of rows in the right-hand-side matrix.
		 * @tparam L
		 *     Number of columns in the right-hand-side matrix.
		 * @tparam MatrixLike
		 *     Type of the right-hand-side matrix.
		 * @param lhs
		 *     Left-hand-side of the multiplication.
		 * @param rhs
		 *     Right-hand-side of the multiplication.
		 * @return
		 *     Product of `lhs` and `rhs`.
		 */
		template <
			int M2, int N2, int L, template < int, int > class MatrixLike >
		static Matrix< M2, L > multiply(const Matrix< M2, N2 >& lhs,
										const MatrixLike< N2, L >& rhs)
		{
			double* pBlock = new double[M2 * L];
			double* pDst = pBlock;
			for (int i = 0; i < M2; ++i) {
				for (int j = 0; j < L; ++j) {
					const double* pL = lhs.pBlock + i * N2;
					double x = 0.0;
					for (int k = 0; k < N2; ++k) {
						x += *pL * rhs(k, j);
						++pL;
					}
					*pDst = x;
					++pDst;
				}
			}
			return Matrix< M2, L >(pBlock);
		}

		/**
		 * Multiplies given two matrices.
		 *
		 * `MatrixLike` must overload the function-call operator `()` which
		 * takes a row index `i` and column index `j` and returns the element at
		 * the ith row and jth column.
		 * The function prototype should look like the following,
		 *  - `double operator ()(int i, int j) const`
		 *
		 * @tparam M2
		 *     Number of rows in the left-hand-side matrix.
		 * @tparam N2
		 *     Number of columns in the left-hand-side matrix.
		 *     Number of rows in the right-hand-side matrix.
		 * @tparam L
		 *     Number of columns in the right-hand-side matrix.
		 * @tparam MatrixLike
		 *     Type of the left-hand-side matrix.
		 * @param lhs
		 *     Left-hand-side of the multiplication.
		 * @param rhs
		 *     Right-hand-side of the multiplication.
		 * @return
		 *     Product of `lhs` and `rhs`.
		 */
		template <
			int M2, int N2, int L, template < int, int > class MatrixLike >
		static Matrix< M2, L > multiply(const MatrixLike< M2, N2 >& lhs,
										const Matrix< N2, L >& rhs)
		{
			double* pBlock = new double[M2 * L];
			double* pDst = pBlock;
			for (int i = 0; i < M2; ++i) {
				for (int j = 0; j < L; ++j) {
					const double* pR = rhs.pBlock + j;
					double x = 0.0;
					for (int k = 0; k < N2; ++k) {
						x += lhs(i, k) * *pR;
						pR += L;
					}
					*pDst = x;
					++pDst;
				}
			}
			return Matrix< M2, L >(pBlock);
		}

		/**
		 * Multiplies given two matrices.
		 *
		 * Both of `MatrixLike1` and `MatrixLike2` must overload the
		 * function-call operator `()` which takes a row index `i` and column
		 * index `j` and returns the element at ith row and jth column.
		 * The function prototype should look like the following,
		 *  - `double operator ()(int i, int j) const`
		 *
		 * @tparam M2
		 *     Number of rows in the left-hand-side matrix.
		 * @tparam N2
		 *     Number of columns in the left-hand-side matrix.
		 *     Number of rows in the right-hand-side matrix.
		 * @tparam L
		 *     Number of columns in the right-hand-side matrix.
		 * @tparam MatrixLike1
		 *     Type of the left-hand-side matrix.
		 * @tparam MatrixLike2
		 *     Type of the right-hand-side matrix.
		 * @param lhs
		 *     Left-hand-side of the multiplication.
		 * @param rhs
		 *     Right-hand-side of the multiplication.
		 * @return
		 *     Product of `lhs` and `rhs`.
		 */
		template <
			int M2, int N2, int L,
			template < int, int > class MatrixLike1,
			template < int, int > class MatrixLike2 >
		static Matrix< M2, L > multiply(const MatrixLike1< M2, N2 >& lhs,
										const MatrixLike2< N2, L >& rhs)
		{
			double* pBlock = new double[M2 * L];
			double* pDst = pBlock;
			for (int i = 0; i < M2; ++i) {
				for (int j = 0; j < L; ++j) {
					double x = 0.0;
					for (int k = 0; k < N2; ++k) {
						x += lhs(i, k) * rhs(k, j);
					}
					*pDst = x;
					++pDst;
				}
			}
			return Matrix< M2, L >(pBlock);
		}
#endif

		/**
		 * Returns the transposition of this matrix.
		 *
		 * @return
		 *     Transposition of this matrix.
		 */
		Matrix< N, M > transpose() const {
			double* pBlock = new double[M * N];
			const double* pSrc = this->pBlock;
			for (int i = 0; i < M; ++i){
				double* pDst = pBlock + i;
				for (int j = 0; j < N; ++j) {
					*pDst = *pSrc;
					++pSrc;
					pDst += M;
				}
			}
			return Matrix< N, M >(pBlock);
		}

		/**
		 * Shuffles rows in this matrix.
		 *
		 * Equivalent to multiplying the following permutation matrix
		 * \f$\mathbf{P}\f$ from the left of this matrix.
		 *
		 * \f[
		 * \mathbf{P}_{ij} =
		 *   \left\{
		 *     \begin{array}{ll}
		 *       1 & (\text{order}[i] = j) \\
		 *       0 & (\text{order}[i] \neq j)
		 *     \end{array}
		 *   \right.
		 * \f]
		 *
		 * @param order
		 *     New order of rows.
		 *     Must have at least M elements.
		 * @return
		 *     Matrix shuffled in the given order.
		 */
		Matrix shuffleRows(const int order[]) const {
			double* pBlock = new double[M * N];
			double* pDst = pBlock;
			for (int i = 0; i < M; ++i) {
				double* pSrc = this->pBlock + order[i] * N;
				std::copy(pSrc, pSrc + N, pDst);
				pDst += N;
			}
			return Matrix(pBlock);
		}

		/**
		 * Shuffles columns in this matrix.
		 *
		 * Equivalent to multiplying the following permutation matrix
		 * \f$\mathbf{P}\f$ from the right of this matrix.
		 *
		 * \f[
		 * \mathbf{P}_{ij} =
		 *   \left\{
		 *     \begin{array}{ll}
		 *       1 & (i = \text{order}[j]) \\
		 *       0 & (i \neq \text{order}[j])
		 *     \end{array}
		 *   \right.
		 * \f]
		 *
		 * @param order
		 *     New order of columns.
		 *     Must have at least N elements.
		 * @return
		 *     Matrix shuffled in the given order.
		 */
		Matrix shuffleColumns(const int order[]) const {
			double* pBlock = new double[M * N];
			for (int j = 0; j < N; ++j) {
				double* pDst = pBlock + j;
				double* pSrc = this->pBlock + order[j];
				for (int i = 0; i < M; ++i) {
					*pDst = *pSrc;
					pSrc += N;
					pDst += N;
				}
			}
			return Matrix(pBlock);
		}
	private:
		/**
		 * Initializes with a given memory block.
		 *
		 * @param pBlock
		 *     Memory block of the new matrix.
		 *     Must have at least M * N elements.
		 */
		Matrix(double* pBlock) : pBlock(pBlock) {}

#if SINGULAR_FUNCTION_DELETION_SUPPORTED
		/** Simple copy is not allowed. */
		Matrix(const Matrix& copyee) = delete;

		/** Simple copy is not allowed. */
		Matrix& operator =(const Matrix& copyee) = delete;
#elif SINGULAR_RVALUE_REFERENCE_SUPPORTED
		/** Simple copy is not allowed. */
		Matrix(const Matrix& copyee) {}

		/** Simple copy is not allowed. */
		Matrix& operator =(const Matrix& copyee) {
			return *this;
		}
#endif

		/**
		 * Releases the memory block of this matrix.
		 *
		 * Has no effect if the memory block has already been released.
		 */
		inline void release() {
			delete[] this->pBlock;
			this->pBlock = 0;
		}
	};

	/**
	 * Multiplies given two matrices.
	 *
	 * @tparam M
	 *     Number of rows in the left-hand-side matrix.
	 * @tparam N
	 *     Number of columns in the left-hand-side matrix.
	 *     Number of rows in the right-hand-side matrix as well.
	 * @tparam L
	 *     Number of columns in the right-hand-side matrix.
	 * @param lhs
	 *     Left-hand side of the multiplication.
	 * @param rhs
	 *     Right-hand side of the multiplication.
	 * @return
	 *     Product of `lhs` and `rhs`.
	 */
	template < int M, int N, int L >
	Matrix< M, L > operator *(const Matrix< M, N >& lhs,
							  const Matrix< N, L >& rhs)
	{
		double* pBlock = new double[M * L];
		double* pDst = pBlock;
		for (int i = 0; i < M; ++i) {
			for (int l = 0; l < L; ++l) {
				double* pL = lhs.pBlock + i * N;
				double* pR = rhs.pBlock + l;
				double x = 0.0;
				for (int j = 0; j < N; ++j) {
					x += *pL * *pR;
					++pL;
					pR += L;
				}
				*pDst = x;
				++pDst;
			}
		}
		return Matrix< M, L >(pBlock);
	}

	/**
	 * Multiplies given two matrices.
	 *
	 * `MatrixLike` must overload the function-call operator `()` which
	 * takes a row index `i` and column index `j` and returns the element at
	 * the ith row and jth column.
	 * The function prototype should look like the following,
	 *  - `double operator ()(int i, int j) const`
	 *
	 * @tparam M
	 *     Number of rows in the left-hand-side matrix.
	 * @tparam N
	 *     Number of columns in the left-hand-side matrix.
	 *     Number of rows in the right-hand-side matrix.
	 * @tparam L
	 *     Number of columns in the right-hand-side matrix.
	 * @tparam MatrixLike
	 *     Type of the right-hand-side matrix.
	 * @param lhs
	 *     Left-hand-side of the multiplication.
	 * @param rhs
	 *     Right-hand-side of the multiplication.
	 * @return
	 *     Product of `lhs` and `rhs`.
	 */
	template < int M, int N, int L, template < int, int > class MatrixLike >
	Matrix< M, L > operator *(const Matrix< M, N >& lhs,
							  const MatrixLike< N, L >& rhs)
	{
#if SINGULAR_TEMPLATE_FRIEND_OPERATOR_OVERLOADING_SUPPORTED
		double* pBlock = new double[M * L];
		double* pDst = pBlock;
		for (int i = 0; i < M; ++i) {
			for (int j = 0; j < L; ++j) {
				const double* pL = lhs.pBlock + i * N;
				double x = 0.0;
				for (int k = 0; k < N; ++k) {
					x += *pL * rhs(k, j);
					++pL;
				}
				*pDst = x;
				++pDst;
			}
		}
		return Matrix< M, L >(pBlock);
#else
		return Matrix< M, N >::multiply(lhs, rhs);
#endif
	}

	/**
	 * Multiplies given two matrices.
	 *
	 * `MatrixLike` must overload the function-call operator `()` which
	 * takes a row index `i` and column index `j` and returns the element at
	 * the ith row and jth column.
	 * The function prototype should look like the following,
	 *  - `double operator ()(int i, int j) const`
	 *
	 * @tparam M
	 *     Number of rows in the left-hand-side matrix.
	 * @tparam N
	 *     Number of columns in the left-hand-side matrix.
	 *     Number of rows in the right-hand-side matrix.
	 * @tparam L
	 *     Number of columns in the right-hand-side matrix.
	 * @tparam MatrixLike
	 *     Type of the left-hand-side matrix.
	 * @param lhs
	 *     Left-hand-side of the multiplication.
	 * @param rhs
	 *     Right-hand-side of the multiplication.
	 * @return
	 *     Product of `lhs` and `rhs`.
	 */
	template < int M, int N, int L, template < int, int > class MatrixLike >
	Matrix< M, L > operator *(const MatrixLike< M, N >& lhs,
							  const Matrix< N, L >& rhs)
	{
#if SINGULAR_TEMPLATE_FRIEND_OPERATOR_OVERLOADING_SUPPORTED
		double* pBlock = new double[M * L];
		double* pDst = pBlock;
		for (int i = 0; i < M; ++i) {
			for (int j = 0; j < L; ++j) {
				const double* pR = rhs.pBlock + j;
				double x = 0.0;
				for (int k = 0; k < N; ++k) {
					x += lhs(i, k) * *pR;
					pR += L;
				}
				*pDst = x;
				++pDst;
			}
		}
		return Matrix< M, L >(pBlock);
#else
		return Matrix< M, L >::multiply(lhs, rhs);
#endif
	}

	/**
	 * Multiplies given two matrices.
	 *
	 * Both of `MatrixLike1` and `MatrixLike2` must overload the
	 * function-call operator `()` which takes a row index `i` and column
	 * index `j` and returns the element at ith row and jth column.
	 * The function prototype should look like the following,
	 *  - `double operator ()(int i, int j) const`
	 *
	 * @tparam M
	 *     Number of rows in the left-hand-side matrix.
	 * @tparam N
	 *     Number of columns in the left-hand-side matrix.
	 *     Number of rows in the right-hand-side matrix.
	 * @tparam L
	 *     Number of columns in the right-hand-side matrix.
	 * @tparam MatrixLike1
	 *     Type of the left-hand-side matrix.
	 * @tparam MatrixLike2
	 *     Type of the right-hand-side matrix.
	 * @param lhs
	 *     Left-hand-side of the multiplication.
	 * @param rhs
	 *     Right-hand-side of the multiplication.
	 * @return
	 *     Product of `lhs` and `rhs`.
	 */
	template <
		int M, int N, int L,
		template < int, int > class MatrixLike1,
		template < int, int > class MatrixLike2 >
	Matrix< M, L > operator *(const MatrixLike1< M, N >& lhs,
							  const MatrixLike2< N, L >& rhs)
	{
#if SINGULAR_TEMPLATE_FRIEND_OPERATOR_OVERLOADING_SUPPORTED
		double* pBlock = new double[M * L];
		double* pDst = pBlock;
		for (int i = 0; i < M; ++i) {
			for (int j = 0; j < L; ++j) {
				double x = 0.0;
				for (int k = 0; k < N; ++k) {
					x += lhs(i, k) * rhs(k, j);
				}
				*pDst = x;
				++pDst;
			}
		}
		return Matrix< M, L >(pBlock);
#else
		return Matrix< M, L >::multiply(lhs, rhs);
#endif
	}


	/**
	 * Writes a given matrix to a given stream.
	 *
	 * `MatrixLike` should overload the function-call operator `()` which takes
	 * a row index `i` and column index `j` and returns the element at the ith
	 * row and jth column.
	 * The function prototype should look like the following,
	 *  - `double operator ()(int i, int j) const`
	 *
	 * @tparam M
	 *     Number of rows in the matrix to be written.
	 * @tparam N
	 *     Number of columns in the matrix to be written.
	 * @tparam MatrixLike
	 *     Type of the matrix to be written.
	 * @param out
	 *     Output stream where the matrix is to be written.
	 * @param m
	 *     Matrix to be written.
	 * @return
	 *     `out`.
	 */
	template < int M, int N, template < int, int > class MatrixLike >
	std::ostream& operator <<(std::ostream& out, const MatrixLike< M, N >& m) {
		out << '[' << std::endl;
		for (int i = 0; i < M; ++i) {
			for (int j = 0; j < N; ++j) {
				out << m(i, j);
				if (j + 1 < N) {
					out << ' ';
				}
			}
			out << std::endl;
		}
		out << ']';
		return out;
	}

}

#endif
