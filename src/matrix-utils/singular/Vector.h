#ifndef _SINGULAR_VECTOR_H
#define _SINGULAR_VECTOR_H

#include "matrix-utils/singular/singular.h"

#include <cassert>
#include <cstddef>
#include <iterator>
#include <sstream>

namespace singular {

	/** Vector. */
	template < typename T >
	class Vector {
	private:
		/** Memory block of this vector. */
		T* pBlock;

		/** Size of this vector. */
		size_t len;

		/** Distance to jump from one element to the next element. */
		ptrdiff_t delta;
	public:
		/** General iterator. */
		template < typename U >
		class general_iterator
			: public std::iterator< std::bidirectional_iterator_tag, U >
		{
		private:
			friend class Vector;

			/** Pointer to an element. */
			U* ptr;

			/** Distance to jump to the next element. */
			ptrdiff_t delta;
		private:
			/**
			 * Constructs an iterator with given pointer and delta.
			 *
			 * @param ptr
			 *     Pointer to an element.
			 * @param delta
			 *     Distance to jump to the next element.
			 */
			inline general_iterator(U* ptr, ptrdiff_t delta)
				: ptr(ptr), delta(delta) {}

			/**
			 * Returns an iterator that points to the first element in a given
			 * vector.
			 *
			 * @param v
			 *     Vector in which elements are to be iterated.
			 * @return
			 *     Iterator that points to the first element in `v`.
			 */
			static inline general_iterator begin(const Vector< T >& v) {
				return general_iterator(v.pBlock, v.delta);
			}

			/**
			 * Returns an iterator that points the stop element in a given
			 * vector.
			 *
			 * @param v
			 *     Vector in which elements are to be iterated.
			 * @return
			 *     Stop iterator of `pVector`.
			 */
			static inline general_iterator end(const Vector< T >& v) {
				U* ptr = v.pBlock + v.size() * v.delta;
				return general_iterator(ptr, v.delta);
			}

			/**
			 * Moves the pointer.
			 *
			 * @param delta
			 *     Amount of the move.
			 */
			inline void move(ptrdiff_t delta) {
				this->ptr += delta;
			}
		public:
			/**
			 * Returns the element that this iterator points.
			 *
			 * @return
			 *     Element that this iterator points.
			 */
			inline U& operator *() const {
				return *this->ptr;
			}

			/**
			 * Returns the pointer to the element that this iterator points.
			 *
			 * @return
			 *     Pointer to the element that this iterator points.
			 */
			inline U* operator ->() const {
				return this->ptr;
			}

			/**
			 * Moves forward this iterator (pre-increment).
			 *
			 * @return
			 *     This iterator that points to the next element.
			 */
			inline general_iterator& operator ++() {
				this->move(this->delta);
				return *this;
			}

			/**
			 * Moves forward this iterator (post-increment).
			 *
			 * @return
			 *     Iterator that points to the element before the move.
			 */
			general_iterator operator ++(int) {
				general_iterator prev = *this;
				this->move(this->delta);
				return prev;
			}

			/**
			 * Moves backward this iterator (pre-decrement).
			 *
			 * @return
			 *     This iterator that points to the previous element.
			 */
			inline general_iterator& operator --() {
				this->move(-this->delta);
				return *this;
			}

			/**
			 * Moves backward this iterator (post-decrement).
			 *
			 * @return
			 *     Iterator that points to the element before the move.
			 */
			general_iterator operator --(int) {
				general_iterator next = *this;
				this->move(-this->delta);
				return next;
			}

			/**
			 * Returns whether this iterator and a given iterator point
			 * the same element.
			 *
			 * @param rhs
			 *     Iterator to be tested.
			 * @return
			 *     Whether this iterator and `rhs` point the same element.
			 */
			inline bool operator ==(const general_iterator& rhs) const {
				return this->ptr == rhs.ptr;
			}

			/**
			 * Returns whether this iterator and a given iterator point
			 * different elements.
			 *
			 * @param rhs
			 *     Iterator to be tested.
			 * @return
			 *     Whether this iterator and `rhs` point different elements.
			 */
			inline bool operator !=(const general_iterator& rhs) const {
				return this->ptr != rhs.ptr;
			}
		};

		/** Iterator. */
		typedef general_iterator< T > iterator;

		/** Const iterator. */
		typedef general_iterator< const T > const_iterator;
	public:
		/**
		 * Constructs an instance that wraps a given memory block.
		 *
		 * `pBlock` must be valid during this vector is used.
		 * It is the caller's responsibility to release `pBlock` when it is no
		 * longer needed.
		 *
		 * @param pBlock
		 *     Pointer to the memory block of the vector.
		 * @param size
		 *     Size of the vector.
		 * @param delta
		 *     Distance to jump from one element to the next element.
		 */
		Vector(T* pBlock, size_t size, ptrdiff_t delta)
			: pBlock(pBlock), len(size), delta(delta) {}

		/**
		 * Returns the size of this vector.
		 *
		 * @return
		 *     Size of this vector.
		 */
		inline size_t size() const {
			return this->len;
		}

		/**
		 * Returns the element at a given index in this vector.
		 *
		 * The behavior is undefined if `idx >= this.size()`.
		 *
		 * @param idx
		 *     Index of the element to be obtained.
		 * @return
		 *     Element at `idx`.
		 *     Changes to a returned element is reflected to this vector.
		 */
		inline T& operator [](size_t idx) {
			return this->pBlock[idx * this->delta];
		}

		/**
		 * Returns the element at a given index in this vector.
		 *
		 * The behavior is undefined if `idx >= this.size()`.
		 *
		 * @param idx
		 *     Index of the element to be obtained.
		 * @return
		 *     Element at `idx`.
		 */
		inline const T& operator [](size_t idx) const {
			return this->pBlock[idx * this->delta];
		}

		/**
		 * Starts iteration over elements in this vector.
		 *
		 * @return
		 *    Iterator that iterates elements in this vector.
		 */
		iterator begin() {
			return iterator::begin(*this);
		}

		/**
		 * Starts iteration over elements in this vector (const).
		 *
		 * @return
		 *     Const iterator that iterates elements in this vector.
		 */
		const_iterator begin() const {
			return const_iterator::begin(*this);
		}

		/**
		 * Returns the stop iterator of this vector.
		 *
		 * @return
		 *     Stop iterator of this vector.
		 */
		iterator end() {
			return iterator::end(*this);
		}

		/**
		 * Returns the stop iterator of this vector (const).
		 *
		 * @return
		 *     Stop const iterator of this vector.
		 */
		const_iterator end() const {
			return const_iterator::end(*this);
		}

		/**
		 * Returns a subvector of this vector.
		 *
		 * A returned subvector shares the elements with this vector.
		 *
		 * If `start == this->size()`, the subvector is empty.
		 *
		 * The behavior is undefined if `start < 0` or `start > this->size()`
		 *
		 * @param start
		 *     Index of the beginning of the slice.
		 * @return
		 *     Sliced vector.
		 */
		Vector slice(size_t start) const {
			assert(start >= 0 && start <= this->size());
			return Vector(this->pBlock + start * this->delta,
						  this->size() - start,
						  this->delta);
		}

		/**
		 * Implicit conversion to a const variant.
		 *
		 * @return
		 *     Const variant of this vector.
		 */
		inline operator Vector< const T >() const {
			return Vector< const T >(this->pBlock, this->size(), this->delta);
		}
	};

}

#endif
