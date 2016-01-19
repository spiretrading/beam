#ifndef BEAM_INDEXEDITERATOR_HPP
#define BEAM_INDEXEDITERATOR_HPP
#include <boost/iterator/iterator_facade.hpp>
#include "Beam/Collections/Collections.hpp"
#include "Beam/Collections/View.hpp"

namespace Beam {

  /*! \class IndexedIteratorValue
      \brief Stores a pair consisting of an iterator and an index.
      \tparam IteratorType The type of iterator to store.
   */
  template<typename IteratorType>
  class IndexedIteratorValue {
    public:

      //! The type of iterator to store.
      using Iterator = IteratorType;

      //! Constructs an IndexedIteratorValue.
      IndexedIteratorValue();

      //! Constructs an IndexedIteratorValue.
      /*!
        \param iterator The iterator to store.
        \param index The index of the iterator.
      */
      IndexedIteratorValue(const Iterator& iterator, std::size_t index);

      //! Returns the value represented by the iterator.
      const typename Iterator::value_type& GetValue() const;

      //! Returns the value represented by the iterator.
      typename Iterator::value_type& GetValue();

      //! Returns the index of the iterator.
      std::size_t GetIndex() const;

    private:
      friend class IndexedIterator<Iterator>;
      Iterator m_iterator;
      std::size_t m_index;
  };

  /*! \class IndexedIterator
      \brief An iterator that consists of a base iterator and an index.
      \tparam IteratorType The type of iterator to store.
   */
  template<typename IteratorType>
  class IndexedIterator : public boost::iterators::iterator_facade<
      IndexedIterator<IteratorType>, IndexedIteratorValue<IteratorType>,
      boost::iterators::random_access_traversal_tag> {
    public:

      //! The type of iterator to store.
      using Iterator = IteratorType;

      //! The type used to compute the difference between two iterators.
      using difference_type = typename boost::iterators::iterator_facade<
        IndexedIterator<IteratorType>, IndexedIteratorValue<IteratorType>,
        boost::iterators::random_access_traversal_tag>::difference_type;

      //! Constructs an IndexedIterator.
      IndexedIterator() = default;

      //! Constructs an IndexedIterator.
      /*!
        \param iterator The iterator to store.
      */
      IndexedIterator(const Iterator& iterator);

    private:
      friend class boost::iterators::iterator_core_access;
      mutable IndexedIteratorValue<IteratorType> m_value;

      void decrement();
      void increment();
      bool equal(const IndexedIterator& other) const;
      void advance(difference_type n);
      difference_type distance_to(const IndexedIterator& other) const;
      IndexedIteratorValue<IteratorType>& dereference() const;
  };

  template<typename Iterator>
  IndexedIterator<typename std::decay<Iterator>::type> MakeIndexedIterator(
      Iterator&& iterator) {
    return IndexedIterator<typename std::decay<Iterator>::type>(
      std::forward<Iterator>(iterator));
  }

  template<typename Collection>
  auto MakeIndexedView(const Collection& collection) {
    return MakeView(MakeIndexedIterator(collection.begin()),
      MakeIndexedIterator(collection.end()));
  }

  template<typename IteratorType>
  IndexedIteratorValue<IteratorType>::IndexedIteratorValue()
      : m_index(-1) {}

  template<typename IteratorType>
  IndexedIteratorValue<IteratorType>::IndexedIteratorValue(
      const Iterator& iterator, std::size_t index)
      : m_iterator(iterator),
        m_index(index) {}

  template<typename IteratorType>
  const typename IndexedIteratorValue<IteratorType>::Iterator::value_type&
      IndexedIteratorValue<IteratorType>::GetValue() const {
    return *m_iterator;
  }

  template<typename IteratorType>
  typename IndexedIteratorValue<IteratorType>::Iterator::value_type&
      IndexedIteratorValue<IteratorType>::GetValue() {
    return *m_iterator;
  }

  template<typename IteratorType>
  std::size_t IndexedIteratorValue<IteratorType>::GetIndex() const {
    return m_index;
  }

  template<typename IteratorType>
  IndexedIterator<IteratorType>::IndexedIterator(const Iterator& iterator)
      : m_value(iterator, 0) {}

  template<typename IteratorType>
  void IndexedIterator<IteratorType>::decrement() {
    --m_value.m_iterator;
    --m_value.m_index;
  }

  template<typename IteratorType>
  void IndexedIterator<IteratorType>::increment() {
    ++m_value.m_iterator;
    ++m_value.m_index;
  }

  template<typename IteratorType>
  void IndexedIterator<IteratorType>::advance(difference_type n) {
    m_value.m_iterator = m_value.m_iterator + n;
    m_value.m_index += n;
  }

  template<typename IteratorType>
  bool IndexedIterator<IteratorType>::equal(const IndexedIterator& rhs) const {
    return m_value.m_iterator == rhs.m_value.m_iterator;
  }

  template<typename IteratorType>
  typename IndexedIterator<IteratorType>::difference_type
      IndexedIterator<IteratorType>::distance_to(
      const IndexedIterator& other) const {
    return std::distance(m_value.m_iterator, other.m_value.m_iterator);
  }

  template<typename IteratorType>
  IndexedIteratorValue<IteratorType>& IndexedIterator<IteratorType>::
      dereference() const {
    return m_value;
  }
}

#endif
