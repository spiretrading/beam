#ifndef BEAM_ANYITERATOR_HPP
#define BEAM_ANYITERATOR_HPP
#include <iterator>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/Pointers/UniquePtr.hpp"
#include "Beam/Utilities/Utilities.hpp"
#include "Beam/Utilities/NotSupportedException.hpp"

namespace Beam {
namespace Details {
  template<typename T>
  void Decrement(T& iterator, std::bidirectional_iterator_tag) {
    --iterator;
  }

  template<typename T>
  void Decrement(T& iterator, std::forward_iterator_tag) {
    BOOST_THROW_EXCEPTION(NotSupportedException("AnyIterator::Decrement"));
  }
}

  /*! \class AnyIterator
      \brief A generic iterator that can be used with any underlying iterator.
      \tparam T The type to iterate over.
   */
  template<typename T>
  class AnyIterator : public boost::iterators::iterator_facade<AnyIterator<T>,
      T, boost::iterators::random_access_traversal_tag> {
    public:

      //! The type to iterate over.
      using value_type = T;

      //! The type used to compute the difference between two iterators.
      using difference_type = typename boost::iterators::iterator_facade<
        AnyIterator<T>, T, boost::iterators::random_access_traversal_tag>::
        difference_type;

      //! Constructs an AnyIterator.
      AnyIterator() = default;

      //! Copies an AnyIterator.
      /*!
        \param iterator The iterator to copy.
      */
      AnyIterator(const AnyIterator& iterator);

      //! Constructs an AnyIterator.
      /*!
        \param iterator The iterator to wrap.
      */
      template<typename IteratorType>
      AnyIterator(const IteratorType& iterator);

      //! Assigns an AnyIterator.
      /*!
        \param rhs The AnyIterator to assign from.
      */
      AnyIterator& operator =(const AnyIterator& rhs);

    private:
      friend class boost::iterators::iterator_core_access;
      class BaseIterator;
      std::unique_ptr<BaseIterator> m_iterator;
      class BaseIterator {
        public:
          virtual ~BaseIterator() = default;
          virtual std::unique_ptr<BaseIterator> Clone() const = 0;
          virtual bool Equals(const BaseIterator& rhs) const = 0;
          virtual value_type& Dereference() const = 0;
          virtual void Increment() = 0;
          virtual void Decrement() = 0;
          virtual void Advance(difference_type n) = 0;
          virtual difference_type DistanceTo(const BaseIterator& rhs) const = 0;
      };
      template<typename WrappedIterator>
      class Iterator : public BaseIterator {
        public:
          Iterator(const WrappedIterator& iterator);
          virtual std::unique_ptr<BaseIterator> Clone() const;
          virtual bool Equals(const BaseIterator& rhs) const;
          virtual value_type& Dereference() const;
          virtual void Increment();
          virtual void Decrement();
          virtual void Advance(difference_type n);
          virtual difference_type DistanceTo(const BaseIterator& rhs) const;

        private:
          WrappedIterator m_iterator;
      };

      AnyIterator(std::unique_ptr<BaseIterator> iterator);
      void decrement();
      void increment();
      bool equal(const AnyIterator& other) const;
      void advance(difference_type n);
      difference_type distance_to(const AnyIterator& other) const;
      value_type& dereference() const;
  };

  template<typename T>
  AnyIterator<T>::AnyIterator(const AnyIterator& iterator) {
    if(iterator.m_iterator != nullptr) {
      m_iterator = iterator.m_iterator->Clone();
    }
  }

  template<typename T>
  template<typename IteratorType>
  AnyIterator<T>::AnyIterator(const IteratorType& iterator)
      : m_iterator(std::make_unique<Iterator<IteratorType>>(iterator)) {}

  template<typename T>
  AnyIterator<T>& AnyIterator<T>::operator =(const AnyIterator& rhs) {
    if(this == &rhs) {
      return *this;
    }
    if(rhs.m_iterator == nullptr) {
      m_iterator = nullptr;
    } else {
      m_iterator = rhs.m_iterator->Clone();
    }
    return *this;
  }

  template<typename T>
  void AnyIterator<T>::decrement() {
    m_iterator->Decrement();
  }

  template<typename T>
  void AnyIterator<T>::increment() {
    m_iterator->Increment();
  }

  template<typename T>
  bool AnyIterator<T>::equal(const AnyIterator& other) const {
    if(m_iterator == nullptr) {
      return other.m_iterator == nullptr;
    } else if(other.m_iterator == nullptr) {
      return false;
    }
    return m_iterator->Equals(*other.m_iterator);
  }

  template<typename T>
  void AnyIterator<T>::advance(difference_type n) {
    m_iterator->Advance(n);
  }

  template<typename T>
  typename AnyIterator<T>::difference_type AnyIterator<T>::distance_to(
      const AnyIterator& other) const {
    return m_iterator->DistanceTo(*other.m_iterator);
  }

  template<typename T>
  typename AnyIterator<T>::value_type& AnyIterator<T>::dereference() const {
    return m_iterator->Dereference();
  }

  template<typename T>
  AnyIterator<T>::AnyIterator(std::unique_ptr<BaseIterator> iterator)
      : m_iterator(std::move(iterator)) {}

  template<typename T>
  template<typename WrappedIterator>
  AnyIterator<T>::Iterator<WrappedIterator>::Iterator(
      const WrappedIterator& iterator)
      : m_iterator(iterator) {}

  template<typename T>
  template<typename WrappedIterator>
  std::unique_ptr<typename AnyIterator<T>::BaseIterator>
      AnyIterator<T>::Iterator<WrappedIterator>::Clone() const {
    return std::make_unique<Iterator>(m_iterator);
  }

  template<typename T>
  template<typename WrappedIterator>
  bool AnyIterator<T>::Iterator<WrappedIterator>::Equals(
      const BaseIterator& rhs) const {
    auto& rhsIterator = static_cast<const Iterator&>(rhs);
    return m_iterator == rhsIterator.m_iterator;
  }

  template<typename T>
  template<typename WrappedIterator>
  typename AnyIterator<T>::value_type&
      AnyIterator<T>::Iterator<WrappedIterator>::Dereference() const {
    return *m_iterator;
  }

  template<typename T>
  template<typename WrappedIterator>
  void AnyIterator<T>::Iterator<WrappedIterator>::Increment() {
    ++m_iterator;
  }

  template<typename T>
  template<typename WrappedIterator>
  void AnyIterator<T>::Iterator<WrappedIterator>::Decrement() {
    Details::Decrement(m_iterator,
      typename std::iterator_traits<WrappedIterator>::iterator_category());
  }

  template<typename T>
  template<typename WrappedIterator>
  void AnyIterator<T>::Iterator<WrappedIterator>::Advance(difference_type n) {
    std::advance(m_iterator, n);
  }

  template<typename T>
  template<typename WrappedIterator>
  typename AnyIterator<T>::difference_type AnyIterator<T>::Iterator<
      WrappedIterator>::DistanceTo(const BaseIterator& rhs) const {
    auto& rhsIterator = static_cast<const Iterator&>(rhs);
    return std::distance(m_iterator, rhsIterator.m_iterator);
  }
}

#endif
