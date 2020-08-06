#ifndef BEAM_INDEXED_ITERATOR_HPP
#define BEAM_INDEXED_ITERATOR_HPP
#include <boost/iterator/iterator_facade.hpp>
#include "Beam/Collections/Collections.hpp"
#include "Beam/Collections/View.hpp"

namespace Beam {

  /**
   * Stores a pair consisting of an iterator and an index.
   * @param <I> The type of iterator to store.
   */
  template<typename I>
  class IndexedIteratorValue {
    public:

      /** The type of iterator to store. */
      using Iterator = I;

      /** Constructs an IndexedIteratorValue. */
      IndexedIteratorValue();

      /**
       * Constructs an IndexedIteratorValue.
       * @param iterator The iterator to store.
       * @param index The index of the iterator.
       */
      IndexedIteratorValue(Iterator iterator, std::size_t index);

      /** Returns the value represented by the iterator. */
      const typename Iterator::value_type& GetValue() const;

      /** Returns the value represented by the iterator. */
      typename Iterator::value_type& GetValue();

      /** Returns the index of the iterator. */
      std::size_t GetIndex() const;

    private:
      friend class IndexedIterator<Iterator>;
      Iterator m_iterator;
      std::size_t m_index;
  };

  /**
   * An iterator that consists of a base iterator and an index.
   * @param <I> The type of iterator to store.
   */
  template<typename I>
  class IndexedIterator : public boost::iterators::iterator_facade<
      IndexedIterator<I>, IndexedIteratorValue<I>,
      boost::iterators::random_access_traversal_tag> {
    public:

      /** The type of iterator to store. */
      using Iterator = I;

      /** The type used to compute the difference between two iterators. */
      using difference_type = typename boost::iterators::iterator_facade<
        IndexedIterator<I>, IndexedIteratorValue<I>,
        boost::iterators::random_access_traversal_tag>::difference_type;

      /** Constructs an IndexedIterator. */
      IndexedIterator() = default;

      /**
       * Constructs an IndexedIterator.
       * @param iterator The iterator to store.
       */
      IndexedIterator(Iterator iterator);

    private:
      friend class boost::iterators::iterator_core_access;
      mutable IndexedIteratorValue<I> m_value;

      void decrement();
      void increment();
      bool equal(const IndexedIterator& other) const;
      void advance(difference_type n);
      difference_type distance_to(const IndexedIterator& other) const;
      IndexedIteratorValue<I>& dereference() const;
  };

  /**
   * Makes a View that indexes over a collection.
   * @param collection The Collection to index.
   */
  template<typename Collection>
  auto MakeIndexedView(Collection&& collection) {
    return View(IndexedIterator(collection.begin()),
      IndexedIterator(collection.end()));
  }

  template<typename I>
  IndexedIteratorValue<I>::IndexedIteratorValue()
    : m_index(-1) {}

  template<typename I>
  IndexedIteratorValue<I>::IndexedIteratorValue(Iterator iterator,
    std::size_t index)
    : m_iterator(std::move(iterator)),
      m_index(index) {}

  template<typename I>
  const typename IndexedIteratorValue<I>::Iterator::value_type&
      IndexedIteratorValue<I>::GetValue() const {
    return *m_iterator;
  }

  template<typename I>
  typename IndexedIteratorValue<I>::Iterator::value_type&
      IndexedIteratorValue<I>::GetValue() {
    return *m_iterator;
  }

  template<typename I>
  std::size_t IndexedIteratorValue<I>::GetIndex() const {
    return m_index;
  }

  template<typename I>
  IndexedIterator<I>::IndexedIterator(Iterator iterator)
      : m_value(std::move(iterator), 0) {}

  template<typename I>
  void IndexedIterator<I>::decrement() {
    --m_value.m_iterator;
    --m_value.m_index;
  }

  template<typename I>
  void IndexedIterator<I>::increment() {
    ++m_value.m_iterator;
    ++m_value.m_index;
  }

  template<typename I>
  void IndexedIterator<I>::advance(difference_type n) {
    m_value.m_iterator = m_value.m_iterator + n;
    m_value.m_index += n;
  }

  template<typename I>
  bool IndexedIterator<I>::equal(const IndexedIterator& rhs) const {
    return m_value.m_iterator == rhs.m_value.m_iterator;
  }

  template<typename I>
  typename IndexedIterator<I>::difference_type
      IndexedIterator<I>::distance_to(const IndexedIterator& other) const {
    return std::distance(m_value.m_iterator, other.m_value.m_iterator);
  }

  template<typename I>
  IndexedIteratorValue<I>& IndexedIterator<I>::dereference() const {
    return m_value;
  }
}

#endif
