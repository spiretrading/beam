#ifndef BEAM_INDEX_ITERATOR_HPP
#define BEAM_INDEX_ITERATOR_HPP
#include <boost/iterator/iterator_facade.hpp>
#include "Beam/Collections/Collections.hpp"
#include "Beam/Collections/View.hpp"

namespace Beam {

  /**
   * Stores a pair consisting of an iterator and an index.
   * @param <I> The type of iterator to store.
   */
  template<typename I>
  class IndexIteratorValue {
    public:

      /** The type of iterator to store. */
      using Iterator = I;

      /** Constructs an IndexIteratorValue. */
      IndexIteratorValue();

      /**
       * Constructs an IndexIteratorValue.
       * @param iterator The iterator to store.
       * @param index The index of the iterator.
       */
      IndexIteratorValue(Iterator iterator, std::size_t index);

      /** Returns the value represented by the iterator. */
      const typename Iterator::value_type& GetValue() const;

      /** Returns the value represented by the iterator. */
      typename Iterator::value_type& GetValue();

      /** Returns the index of the iterator. */
      std::size_t GetIndex() const;

    private:
      friend class IndexIterator<Iterator>;
      Iterator m_iterator;
      std::size_t m_index;
  };

  /**
   * An iterator that consists of a base iterator and an index.
   * @param <I> The type of iterator to store.
   */
  template<typename I>
  class IndexIterator : public boost::iterators::iterator_facade<
      IndexIterator<I>, IndexIteratorValue<I>,
      boost::iterators::random_access_traversal_tag> {
    public:

      /** The type of iterator to store. */
      using Iterator = I;

      /** The type used to compute the difference between two iterators. */
      using difference_type = typename boost::iterators::iterator_facade<
        IndexIterator<Iterator>, IndexIteratorValue<Iterator>,
        boost::iterators::random_access_traversal_tag>::difference_type;

      /** Constructs an IndexIterator. */
      IndexIterator() = default;

      /**
       * Constructs an IndexIterator.
       * @param iterator The iterator to store.
       */
      IndexIterator(Iterator iterator);

    private:
      friend class boost::iterators::iterator_core_access;
      mutable IndexIteratorValue<I> m_value;

      void decrement();
      void increment();
      bool equal(const IndexIterator& other) const;
      void advance(difference_type n);
      difference_type distance_to(const IndexIterator& other) const;
      IndexIteratorValue<I>& dereference() const;
  };

  /**
   * Makes a View that indexes over a collection.
   * @param collection The Collection to index.
   */
  template<typename Collection>
  auto MakeIndexView(Collection&& collection) {
    return View(IndexIterator(collection.begin()),
      IndexIterator(collection.end()));
  }

  template<typename I>
  IndexIteratorValue<I>::IndexIteratorValue()
    : m_index(-1) {}

  template<typename I>
  IndexIteratorValue<I>::IndexIteratorValue(Iterator iterator,
    std::size_t index)
    : m_iterator(std::move(iterator)),
      m_index(index) {}

  template<typename I>
  const typename IndexIteratorValue<I>::Iterator::value_type&
      IndexIteratorValue<I>::GetValue() const {
    return *m_iterator;
  }

  template<typename I>
  typename IndexIteratorValue<I>::Iterator::value_type&
      IndexIteratorValue<I>::GetValue() {
    return *m_iterator;
  }

  template<typename I>
  std::size_t IndexIteratorValue<I>::GetIndex() const {
    return m_index;
  }

  template<typename I>
  IndexIterator<I>::IndexIterator(Iterator iterator)
    : m_value(std::move(iterator), 0) {}

  template<typename I>
  void IndexIterator<I>::decrement() {
    --m_value.m_iterator;
    --m_value.m_index;
  }

  template<typename I>
  void IndexIterator<I>::increment() {
    ++m_value.m_iterator;
    ++m_value.m_index;
  }

  template<typename I>
  void IndexIterator<I>::advance(difference_type n) {
    m_value.m_iterator = m_value.m_iterator + n;
    m_value.m_index += n;
  }

  template<typename I>
  bool IndexIterator<I>::equal(const IndexIterator& rhs) const {
    return m_value.m_iterator == rhs.m_value.m_iterator;
  }

  template<typename I>
  typename IndexIterator<I>::difference_type
      IndexIterator<I>::distance_to(const IndexIterator& other) const {
    return std::distance(m_value.m_iterator, other.m_value.m_iterator);
  }

  template<typename I>
  IndexIteratorValue<I>& IndexIterator<I>::dereference() const {
    return m_value;
  }
}

#endif
