#ifndef BEAM_SORTEDVECTOR_HPP
#define BEAM_SORTEDVECTOR_HPP
#include <algorithm>
#include <functional>
#include <vector>
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {

  /*! \class SortedVector
      \brief Wraps a standard vector whose elements are in sorted order.
      \tparam T The type of element stored.
      \tparam ComparatorType The type of comparator to use.
      \tparam AllocatorType The type of allocator to use.
   */
  template<typename T, typename ComparatorType = std::less<T>,
      typename AllocatorType = std::allocator<T>>
  class SortedVector {
    public:
      typedef std::vector<T, AllocatorType> Vector;
      typedef ComparatorType Comparator;
      typedef typename Vector::value_type value_type;
      typedef typename Vector::allocator_type allocator_type;
      typedef typename Vector::reference reference;
      typedef typename Vector::const_reference const_reference;
      typedef typename Vector::pointer pointer;
      typedef typename Vector::const_pointer const_pointer;
      typedef typename Vector::iterator iterator;
      typedef typename Vector::const_iterator const_iterator;
      typedef typename Vector::reverse_iterator reverse_iterator;
      typedef typename Vector::const_reverse_iterator const_reverse_iterator;
      typedef typename Vector::difference_type difference_type;
      typedef typename Vector::size_type size_type;

      //! Constructs an empty SortedVector.
      SortedVector();

      //! Returns an iterator to the beginning.
      iterator begin();

      //! Returns an iterator to the beginning.
      const_iterator begin() const;

      //! Returns an iterator to the end.
      iterator end();

      //! Returns an iterator to the end.
      const_iterator end() const;

      //! Returns a reverse iterator to the beginning.
      reverse_iterator rbegin();

      //! Returns a reverse iterator to the beginning.
      const_reverse_iterator rbegin() const;

      //! Returns a reverse iterator to the end.
      reverse_iterator rend();

      //! Returns a reverse iterator to the end.
      const_reverse_iterator rend() const;

      //! Returns a const iterator to the beginning.
      const_iterator cbegin() const;

      //! Returns a const iterator to the end.
      const_iterator cend() const;

      //! Returns a const reverse iterator to the beginning.
      const_reverse_iterator crbegin() const;

      //! Returns a const reverse iterator to the end.
      const_reverse_iterator crend() const;

      //! Erases all elements stored.
      void clear();

      //! Inserts a value.
      /*!
        \param value The value to insert.
        \return An iterator to the location where the <i>value</i> was inserted.
      */
      iterator insert(const value_type& value);

      //! Inserts a value.
      /*!
        \param value The value to insert.
        \return An iterator to the location where the <i>value</i> was inserted.
      */
      iterator insert(value_type&& value);

    private:
      Vector m_vector;
  };

  template<typename T, typename ComparatorType, typename AllocatorType>
  SortedVector<T, ComparatorType, AllocatorType>::SortedVector() {}

  template<typename T, typename ComparatorType, typename AllocatorType>
  typename SortedVector<T, ComparatorType, AllocatorType>::iterator
      SortedVector<T, ComparatorType, AllocatorType>::begin() {
    return m_vector.begin();
  }

  template<typename T, typename ComparatorType, typename AllocatorType>
  typename SortedVector<T, ComparatorType, AllocatorType>::const_iterator
      SortedVector<T, ComparatorType, AllocatorType>::begin() const {
    return m_vector.begin();
  }

  template<typename T, typename ComparatorType, typename AllocatorType>
  typename SortedVector<T, ComparatorType, AllocatorType>::iterator
      SortedVector<T, ComparatorType, AllocatorType>::end() {
    return m_vector.end();
  }

  template<typename T, typename ComparatorType, typename AllocatorType>
  typename SortedVector<T, ComparatorType, AllocatorType>::const_iterator
      SortedVector<T, ComparatorType, AllocatorType>::end() const {
    return m_vector.end();
  }

  template<typename T, typename ComparatorType, typename AllocatorType>
  typename SortedVector<T, ComparatorType, AllocatorType>::reverse_iterator
      SortedVector<T, ComparatorType, AllocatorType>::rbegin() {
    return m_vector.rbegin();
  }

  template<typename T, typename ComparatorType, typename AllocatorType>
  typename SortedVector<T, ComparatorType, AllocatorType>::
      const_reverse_iterator SortedVector<T, ComparatorType,
      AllocatorType>::rbegin() const {
    return m_vector.rbegin();
  }

  template<typename T, typename ComparatorType, typename AllocatorType>
  typename SortedVector<T, ComparatorType, AllocatorType>::reverse_iterator
      SortedVector<T, ComparatorType, AllocatorType>::rend() {
    return m_vector.rend();
  }

  template<typename T, typename ComparatorType, typename AllocatorType>
  typename SortedVector<T, ComparatorType, AllocatorType>::
      const_reverse_iterator SortedVector<T, ComparatorType,
      AllocatorType>::rend() const {
    return m_vector.rend();
  }

  template<typename T, typename ComparatorType, typename AllocatorType>
  typename SortedVector<T, ComparatorType, AllocatorType>::const_iterator
      SortedVector<T, ComparatorType, AllocatorType>::cbegin() const {
    return m_vector.cbegin();
  }

  template<typename T, typename ComparatorType, typename AllocatorType>
  typename SortedVector<T, ComparatorType, AllocatorType>::const_iterator
      SortedVector<T, ComparatorType, AllocatorType>::cend() const {
    return m_vector.cend();
  }

  template<typename T, typename ComparatorType, typename AllocatorType>
  typename SortedVector<T, ComparatorType, AllocatorType>::
      const_reverse_iterator SortedVector<T, ComparatorType, AllocatorType>::
      crbegin() const {
    return m_vector.crbegin();
  }

  template<typename T, typename ComparatorType, typename AllocatorType>
  typename SortedVector<T, ComparatorType, AllocatorType>::
      const_reverse_iterator SortedVector<T, ComparatorType, AllocatorType>::
      crend() const {
    return m_vector.crend();
  }

  template<typename T, typename ComparatorType, typename AllocatorType>
  void SortedVector<T, ComparatorType, AllocatorType>::clear() {
    m_vector.clear();
  }

  template<typename T, typename ComparatorType, typename AllocatorType>
  typename SortedVector<T, ComparatorType, AllocatorType>::iterator
      SortedVector<T, ComparatorType, AllocatorType>::insert(
      const value_type& value) {
    auto insertIterator = std::lower_bound(m_vector.begin(), m_vector.end(),
      value, Comparator());
    return m_vector.insert(insertIterator, value);
  }

  template<typename T, typename ComparatorType, typename AllocatorType>
  typename SortedVector<T, ComparatorType, AllocatorType>::iterator
      SortedVector<T, ComparatorType, AllocatorType>::insert(
      value_type&& value) {
    auto insertIterator = std::lower_bound(m_vector.begin(), m_vector.end(),
      value, Comparator());
    return m_vector.insert(insertIterator, std::move(value));
  }
}

#endif
