#ifndef BEAM_SHAREDITERATOR_HPP
#define BEAM_SHAREDITERATOR_HPP
#include <memory>
#include "Beam/Collections/Collections.hpp"

namespace Beam {

  /*! \class SharedIterator
      \brief An iterator that shares ownership of its collection.
      \tparam CollectionType The type of collection this iterator shares
              ownership of.
   */
  template<typename CollectionType>
  class SharedIterator {
    public:

      //! CollectionType The type of collection this iterator shares ownership
      //! of.
      using Collection = CollectionType;

      using difference_type = std::ptrdiff_t;
      using iterator_category = std::forward_iterator_tag;

      //! The type to iterate over.
      using value_type = typename Collection::iterator::value_type;

      //! A reference to the type being iterated over.
      using reference = typename Collection::iterator::reference;

      //! A pointer to the type being iterated over.
      using pointer = typename Collection::iterator::pointer;

      //! Constructs a SharedIterator from a collection and an iterator to the
      //! collection.
      /*!
        \param collection The collection to make the SharedIterator over.
        \param iterator The iterator into the collection.
      */
      SharedIterator(std::shared_ptr<Collection> collection,
        typename Collection::iterator iterator);

      //! Returns a SharedIterator to an offset from this SharedIterator.
      /*!
        \param rhs The offset.
        \return A SharedIterator offset from <i>this</i> by <i>rhs</i>.
      */
      SharedIterator operator +(std::size_t rhs) const;

      //! Returns a reference to the current value.
      reference operator *() const;

      //! Returns a pointer to the value.
      pointer operator ->() const;

      //! Increments this iterator.
      /*!
        \return <code>*this</code>
      */
      SharedIterator& operator ++();

      //! Decrements this iterator.
      /*!
        \return <code>*this</code>
      */
      SharedIterator& operator --();

      //! Tests two iterators for equality.
      bool operator ==(const SharedIterator& rhs) const;

      //! Tests two iterators for inequality.
      bool operator !=(const SharedIterator& rhs) const;

    private:
      std::shared_ptr<Collection> m_collection;
      typename Collection::iterator m_iterator;
  };

  //! Makes a SharedIterator from a collection and an iterator to the
  //! collection.
  /*!
    \param collection The collection to make the SharedIterator over.
    \param iterator The iterator into the collection.
  */
  template<typename Collection>
  SharedIterator<typename std::decay<Collection>::type> MakeSharedIterator(
      std::shared_ptr<Collection> collection,
      const typename Collection::iterator& iterator) {
    return SharedIterator<typename std::decay<Collection>::type>(
      std::move(collection), iterator);
  }

  template<typename CollectionType>
  SharedIterator<CollectionType>::SharedIterator(
      std::shared_ptr<Collection> collection,
      typename Collection::iterator iterator)
      : m_collection(std::move(collection)),
        m_iterator(std::move(iterator)) {}

  template<typename CollectionType>
  SharedIterator<CollectionType> SharedIterator<CollectionType>::operator +(
      std::size_t rhs) const {
    return SharedIterator(m_collection, m_iterator + rhs);
  }

  template<typename CollectionType>
  typename SharedIterator<CollectionType>::reference SharedIterator<
      CollectionType>::operator *() const {
    return *m_iterator;
  }

  template<typename CollectionType>
  typename SharedIterator<CollectionType>::pointer SharedIterator<
      CollectionType>::operator ->() const {
    return &*m_iterator;
  }

  template<typename CollectionType>
  SharedIterator<CollectionType>& SharedIterator<CollectionType>::
      operator ++() {
    ++m_iterator;
    return *this;
  }

  template<typename CollectionType>
  SharedIterator<CollectionType>& SharedIterator<CollectionType>::
      operator --() {
    --m_iterator;
    return *this;
  }

  template<typename CollectionType>
  bool SharedIterator<CollectionType>::operator ==(
      const SharedIterator& rhs) const {
    return m_iterator == rhs.m_iterator;
  }

  template<typename CollectionType>
  bool SharedIterator<CollectionType>::operator !=(
      const SharedIterator& rhs) const {
    return !(*this == rhs);
  }
}

#endif
