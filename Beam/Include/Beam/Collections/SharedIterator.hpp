#ifndef BEAM_SHARED_ITERATOR_HPP
#define BEAM_SHARED_ITERATOR_HPP
#include <memory>
#include "Beam/Collections/Collections.hpp"

namespace Beam {

  /**
   * An iterator that shares ownership of its collection.
   * @param <C> The type of collection this iterator shares ownership of.
   */
  template<typename C>
  class SharedIterator {
    public:

      /** The type of collection this iterator shares ownership of. */
      using Collection = C;

      /** The type of iterator being wrapped. */
      using base_iterator = typename Collection::iterator;

      /** The type used to identify distance between iterators. */
      using difference_type = typename std::iterator_traits<
        base_iterator>::difference_type;

      /** The category of the iterator. */
      using iterator_category = typename std::iterator_traits<
        base_iterator>::iterator_category;

      /** The type to iterate over. */
      using value_type = typename std::iterator_traits<
        base_iterator>::value_type;

      /** A reference to the type being iterated over. */
      using reference = typename std::iterator_traits<base_iterator>::reference;

      /** A pointer to the type being iterated over. */
      using pointer = typename std::iterator_traits<base_iterator>::pointer;

      /**
       * Constructs a SharedIterator from a collection and an iterator to the
       * collection.
       * @param collection The collection to make the SharedIterator over.
       * @param iterator The iterator into the collection.
       */
      SharedIterator(std::shared_ptr<Collection> collection,
        base_iterator iterator);

      /**
       * Returns a SharedIterator to an offset from this SharedIterator.
       * @param rhs The offset.
       * @return A SharedIterator offset from <i>this</i> by <i>rhs</i>.
       */
      SharedIterator operator +(std::size_t rhs) const;

      /** Returns a reference to the current value. */
      reference operator *() const;

      /** Returns a pointer to the value. */
      pointer operator ->() const;

      /**
       * Increments this iterator.
       * @return <code>*this</code>
       */
      SharedIterator& operator ++();

      /**
       * Decrements this iterator.
       * @return <code>*this</code>
       */
      SharedIterator& operator --();

      /** Tests two iterators for equality. */
      bool operator ==(const SharedIterator& rhs) const;

      /** Tests two iterators for inequality. */
      bool operator !=(const SharedIterator& rhs) const;

    private:
      std::shared_ptr<Collection> m_collection;
      base_iterator m_iterator;
  };

  template<typename C>
  SharedIterator<C>::SharedIterator(std::shared_ptr<Collection> collection,
    base_iterator iterator)
    : m_collection(std::move(collection)),
      m_iterator(std::move(iterator)) {}

  template<typename C>
  SharedIterator<C> SharedIterator<C>::operator +(std::size_t rhs) const {
    return SharedIterator(m_collection, m_iterator + rhs);
  }

  template<typename C>
  typename SharedIterator<C>::reference SharedIterator<C>::operator *() const {
    return *m_iterator;
  }

  template<typename C>
  typename SharedIterator<C>::pointer SharedIterator<C>::operator ->() const {
    return &*m_iterator;
  }

  template<typename C>
  SharedIterator<C>& SharedIterator<C>::operator ++() {
    ++m_iterator;
    return *this;
  }

  template<typename C>
  SharedIterator<C>& SharedIterator<C>::operator --() {
    --m_iterator;
    return *this;
  }

  template<typename C>
  bool SharedIterator<C>::operator ==(const SharedIterator& rhs) const {
    return m_iterator == rhs.m_iterator;
  }

  template<typename C>
  bool SharedIterator<C>::operator !=(const SharedIterator& rhs) const {
    return !(*this == rhs);
  }
}

#endif
