#ifndef BEAM_SORTED_VECTOR_HPP
#define BEAM_SORTED_VECTOR_HPP
#include <algorithm>
#include <functional>
#include <vector>
#include "Beam/Collections/Collections.hpp"

namespace Beam {

  /**
   * Wraps a standard vector whose elements are in sorted order.
   * @param <T> The type of element stored.
   * @param <C> The type of comparator to use.
   * @param <A> The type of allocator to use.
   */
  template<typename T, typename C = std::less<T>,
    typename A = std::allocator<T>>
  class SortedVector {
    public:
      using Vector = std::vector<T, A>;
      using Comparator = C;
      using value_type = typename Vector::value_type;
      using allocator_type = typename Vector::allocator_type;
      using reference = typename Vector::reference;
      using const_reference = typename Vector::const_reference;
      using pointer = typename Vector::pointer;
      using const_pointer = typename Vector::const_pointer;
      using iterator = typename Vector::iterator;
      using const_iterator = typename Vector::const_iterator;
      using reverse_iterator = typename Vector::reverse_iterator;
      using const_reverse_iterator = typename Vector::const_reverse_iterator;
      using difference_type = typename Vector::difference_type;
      using size_type = typename Vector::size_type;

      /** Constructs an empty SortedVector. */
      SortedVector() = default;

      /** Returns an iterator to the beginning. */
      iterator begin();

      /** Returns an iterator to the beginning. */
      const_iterator begin() const;

      /** Returns an iterator to the end. */
      iterator end();

      /** Returns an iterator to the end. */
      const_iterator end() const;

      /** Returns a reverse iterator to the beginning. */
      reverse_iterator rbegin();

      /** Returns a reverse iterator to the beginning. */
      const_reverse_iterator rbegin() const;

      /** Returns a reverse iterator to the end. */
      reverse_iterator rend();

      /** Returns a reverse iterator to the end. */
      const_reverse_iterator rend() const;

      /** Returns a const iterator to the beginning. */
      const_iterator cbegin() const;

      /** Returns a const iterator to the end. */
      const_iterator cend() const;

      /** Returns a const reverse iterator to the beginning. */
      const_reverse_iterator crbegin() const;

      /** Returns a const reverse iterator to the end. */
      const_reverse_iterator crend() const;

      /** Erases all elements stored. */
      void clear();

      /**
       * Inserts a value.
       * @param value The value to insert.
       * @return An iterator to the location where the <i>value</i> was
       *         inserted.
       */
      iterator insert(const value_type& value);

      /**
       * Inserts a value.
       * @param value The value to insert.
       * @return An iterator to the location where the <i>value</i> was
       *         inserted.
       */
      iterator insert(value_type&& value);

    private:
      Vector m_vector;
  };

  template<typename T, typename C, typename A>
  typename SortedVector<T, C, A>::iterator SortedVector<T, C, A>::begin() {
    return m_vector.begin();
  }

  template<typename T, typename C, typename A>
  typename SortedVector<T, C, A>::const_iterator
      SortedVector<T, C, A>::begin() const {
    return m_vector.begin();
  }

  template<typename T, typename C, typename A>
  typename SortedVector<T, C, A>::iterator SortedVector<T, C, A>::end() {
    return m_vector.end();
  }

  template<typename T, typename C, typename A>
  typename SortedVector<T, C, A>::const_iterator
      SortedVector<T, C, A>::end() const {
    return m_vector.end();
  }

  template<typename T, typename C, typename A>
  typename SortedVector<T, C, A>::reverse_iterator
      SortedVector<T, C, A>::rbegin() {
    return m_vector.rbegin();
  }

  template<typename T, typename C, typename A>
  typename SortedVector<T, C, A>::const_reverse_iterator
      SortedVector<T, C, A>::rbegin() const {
    return m_vector.rbegin();
  }

  template<typename T, typename C, typename A>
  typename SortedVector<T, C, A>::reverse_iterator
      SortedVector<T, C, A>::rend() {
    return m_vector.rend();
  }

  template<typename T, typename C, typename A>
  typename SortedVector<T, C, A>::const_reverse_iterator
      SortedVector<T, C, A>::rend() const {
    return m_vector.rend();
  }

  template<typename T, typename C, typename A>
  typename SortedVector<T, C, A>::const_iterator
      SortedVector<T, C, A>::cbegin() const {
    return m_vector.cbegin();
  }

  template<typename T, typename C, typename A>
  typename SortedVector<T, C, A>::const_iterator
      SortedVector<T, C, A>::cend() const {
    return m_vector.cend();
  }

  template<typename T, typename C, typename A>
  typename SortedVector<T, C, A>::const_reverse_iterator
      SortedVector<T, C, A>::crbegin() const {
    return m_vector.crbegin();
  }

  template<typename T, typename C, typename A>
  typename SortedVector<T, C, A>::const_reverse_iterator
      SortedVector<T, C, A>::crend() const {
    return m_vector.crend();
  }

  template<typename T, typename C, typename A>
  void SortedVector<T, C, A>::clear() {
    m_vector.clear();
  }

  template<typename T, typename C, typename A>
  typename SortedVector<T, C, A>::iterator SortedVector<T, C, A>::insert(
      const value_type& value) {
    auto insertIterator = std::lower_bound(m_vector.begin(), m_vector.end(),
      value, Comparator());
    return m_vector.insert(insertIterator, value);
  }

  template<typename T, typename C, typename A>
  typename SortedVector<T, C, A>::iterator SortedVector<T, C, A>::insert(
      value_type&& value) {
    auto insertIterator = std::lower_bound(m_vector.begin(), m_vector.end(),
      value, Comparator());
    return m_vector.insert(insertIterator, std::move(value));
  }
}

#endif
