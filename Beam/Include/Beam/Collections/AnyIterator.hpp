#ifndef BEAM_ANY_ITERATOR_HPP
#define BEAM_ANY_ITERATOR_HPP
#include <iterator>
#include <memory>
#include <type_traits>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/Collections/Collections.hpp"
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

  /**
   * A generic iterator that can be used with any underlying iterator.
   * @param <T> The type to iterate over.
   */
  template<typename T>
  class AnyIterator : public boost::iterators::iterator_facade<AnyIterator<T>,
      T, boost::iterators::random_access_traversal_tag> {
    public:

      /** The type to iterate over. */
      using value_type = T;

      /** The type used to compute the difference between two iterators. */
      using difference_type = typename boost::iterators::iterator_facade<
        AnyIterator<T>, T, boost::iterators::random_access_traversal_tag>::
        difference_type;

      /** Constructs an AnyIterator. */
      AnyIterator() = default;

      /**
       * Copies an AnyIterator.
       * @param iterator The iterator to copy.
       */
      AnyIterator(const AnyIterator& iterator);

      /**
       * Acquires an AnyIterator.
       * @param iterator The iterator to acquire.
       */
      AnyIterator(AnyIterator&& iterator) = default;

      /**
       * Constructs an AnyIterator.
       * @param iterator The iterator to wrap.
       */
      template<typename I>
      AnyIterator(I iterator);

      /**
       * Assigns an AnyIterator.
       * \param rhs The AnyIterator to assign from.
       */
      AnyIterator& operator =(const AnyIterator& rhs);

      /**
       * Acquires an AnyIterator.
       * \param rhs The AnyIterator to acquire.
       */
      AnyIterator& operator =(AnyIterator&& rhs) = default;

    private:
      friend class boost::iterators::iterator_core_access;
      class BaseIterator;
      std::unique_ptr<BaseIterator> m_iterator;
      class BaseIterator {
        public:
          virtual ~BaseIterator() = default;
          virtual std::unique_ptr<BaseIterator> Clone() const = 0;
          virtual bool Equals(const BaseIterator& rhs) const = 0;
          virtual const value_type& Dereference() const = 0;
          virtual value_type& Dereference() = 0;
          virtual void Increment() = 0;
          virtual void Decrement() = 0;
          virtual void Advance(difference_type n) = 0;
          virtual difference_type DistanceTo(const BaseIterator& rhs) const = 0;
      };
      template<typename I>
      class Iterator : public BaseIterator {
        public:
          using WrappedIterator = I;

          Iterator(WrappedIterator iterator);
          std::unique_ptr<BaseIterator> Clone() const override;
          bool Equals(const BaseIterator& rhs) const override;
          const value_type& Dereference() const override;
          value_type& Dereference() override;
          void Increment() override;
          void Decrement() override;
          void Advance(difference_type n) override;
          difference_type DistanceTo(const BaseIterator& rhs) const override;

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

  template<typename I>
  AnyIterator(I&&) -> AnyIterator<std::remove_reference_t<
    decltype(*std::declval<I>())>>;

  template<typename T>
  AnyIterator<T>::AnyIterator(const AnyIterator& iterator) {
    if(iterator.m_iterator != nullptr) {
      m_iterator = iterator.m_iterator->Clone();
    }
  }

  template<typename T>
  template<typename I>
  AnyIterator<T>::AnyIterator(I iterator)
    : m_iterator(std::make_unique<Iterator<I>>(std::move(iterator))) {}

  template<typename T>
  AnyIterator<T>& AnyIterator<T>::operator =(const AnyIterator& rhs) {
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
    if(m_iterator == nullptr && other.m_iterator == nullptr) {
      return 0;
    }
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
  template<typename I>
  AnyIterator<T>::Iterator<I>::Iterator(WrappedIterator iterator)
    : m_iterator(std::move(iterator)) {}

  template<typename T>
  template<typename I>
  std::unique_ptr<typename AnyIterator<T>::BaseIterator>
      AnyIterator<T>::Iterator<I>::Clone() const {
    return std::make_unique<Iterator>(m_iterator);
  }

  template<typename T>
  template<typename I>
  bool AnyIterator<T>::Iterator<I>::Equals(const BaseIterator& rhs) const {
    auto& rhsIterator = static_cast<const Iterator&>(rhs);
    return m_iterator == rhsIterator.m_iterator;
  }

  template<typename T>
  template<typename I>
  const typename AnyIterator<T>::value_type&
      AnyIterator<T>::Iterator<I>::Dereference() const {
    return *m_iterator;
  }

  template<typename T>
  template<typename I>
  typename AnyIterator<T>::value_type&
      AnyIterator<T>::Iterator<I>::Dereference() {
    return *m_iterator;
  }

  template<typename T>
  template<typename I>
  void AnyIterator<T>::Iterator<I>::Increment() {
    ++m_iterator;
  }

  template<typename T>
  template<typename I>
  void AnyIterator<T>::Iterator<I>::Decrement() {
    Details::Decrement(m_iterator,
      typename std::iterator_traits<WrappedIterator>::iterator_category());
  }

  template<typename T>
  template<typename I>
  void AnyIterator<T>::Iterator<I>::Advance(difference_type n) {
    std::advance(m_iterator, n);
  }

  template<typename T>
  template<typename I>
  typename AnyIterator<T>::difference_type AnyIterator<T>::Iterator<I>::
      DistanceTo(const BaseIterator& rhs) const {
    auto& rhsIterator = static_cast<const Iterator&>(rhs);
    return std::distance(m_iterator, rhsIterator.m_iterator);
  }
}

#endif
