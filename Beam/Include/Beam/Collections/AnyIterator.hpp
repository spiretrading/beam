#ifndef BEAM_ANY_ITERATOR_HPP
#define BEAM_ANY_ITERATOR_HPP
#include <iterator>
#include <memory>
#include <type_traits>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/Utilities/NotSupportedException.hpp"

namespace Beam {
namespace Details {
  template<typename T>
  void decrement(T& iterator, std::bidirectional_iterator_tag) {
    --iterator;
  }

  template<typename T>
  void decrement(T& iterator, std::forward_iterator_tag) {
    boost::throw_with_location(NotSupportedException("AnyIterator::Decrement"));
  }
}

  /**
   * A generic iterator that can be used with any underlying iterator.
   * @tparam T The type to iterate over.
   */
  template<typename T>
  class AnyIterator : public boost::iterators::iterator_facade<AnyIterator<T>,
      T, boost::iterators::random_access_traversal_tag> {
    public:

      /** The type to iterate over. */
      using value_type = T;

      /** The type used to compute the difference between two iterators. */
      using difference_type =
        typename boost::iterators::iterator_facade<AnyIterator<T>, T,
          boost::iterators::random_access_traversal_tag>::difference_type;

      /** Constructs an AnyIterator. */
      AnyIterator() = default;

      /**
       * Constructs an AnyIterator.
       * @param iterator The iterator to wrap.
       */
      template<typename I>
      AnyIterator(I iterator);

      AnyIterator(const AnyIterator& iterator);
      AnyIterator(AnyIterator&&) = default;

      AnyIterator& operator =(const AnyIterator& rhs);
      AnyIterator& operator =(AnyIterator&&) = default;

    private:
      friend class boost::iterators::iterator_core_access;
      struct VirtualIterator {
        virtual ~VirtualIterator() = default;

        virtual std::unique_ptr<VirtualIterator> clone() const = 0;
        virtual bool equals(const VirtualIterator&) const = 0;
        virtual const value_type& dereference() const = 0;
        virtual value_type& dereference() = 0;
        virtual void increment() = 0;
        virtual void decrement() = 0;
        virtual void advance(difference_type) = 0;
        virtual difference_type distance_to(const VirtualIterator&) const = 0;
      };
      template<typename I>
      struct WrappedIterator final : VirtualIterator {
        using Iterator = I;
        Iterator m_iterator;

        template<typename... Args>
        WrappedIterator(Args&&... args);

        std::unique_ptr<VirtualIterator> clone() const override;
        bool equals(const VirtualIterator& other) const override;
        const value_type& dereference() const override;
        value_type& dereference() override;
        void increment() override;
        void decrement() override;
        void advance(difference_type offset) override;
        difference_type distance_to(
          const VirtualIterator& other) const override;
      };
      std::unique_ptr<VirtualIterator> m_iterator;

      AnyIterator(std::unique_ptr<VirtualIterator> iterator);
      void decrement();
      void increment();
      bool equal(const AnyIterator& other) const;
      void advance(difference_type n);
      difference_type distance_to(const AnyIterator& other) const;
      value_type& dereference() const;
  };

  template<typename I>
  AnyIterator(I&&) ->
    AnyIterator<std::remove_cvref_t<decltype(*std::declval<I>())>>;

  template<typename T>
  template<typename I>
  AnyIterator<T>::AnyIterator(I iterator)
    : m_iterator(std::make_unique<WrappedIterator<I>>(std::move(iterator))) {}

  template<typename T>
  AnyIterator<T>::AnyIterator(const AnyIterator& iterator) {
    if(iterator.m_iterator) {
      m_iterator = iterator.m_iterator->clone();
    }
  }

  template<typename T>
  AnyIterator<T>& AnyIterator<T>::operator =(const AnyIterator& rhs) {
    if(!rhs.m_iterator) {
      m_iterator = nullptr;
    } else {
      m_iterator = rhs.m_iterator->clone();
    }
    return *this;
  }

  template<typename T>
  AnyIterator<T>::AnyIterator(std::unique_ptr<VirtualIterator> iterator)
    : m_iterator(std::move(iterator)) {}

  template<typename T>
  void AnyIterator<T>::decrement() {
    m_iterator->decrement();
  }

  template<typename T>
  void AnyIterator<T>::increment() {
    m_iterator->increment();
  }

  template<typename T>
  bool AnyIterator<T>::equal(const AnyIterator& other) const {
    if(this == &other) {
      return true;
    } else if(!m_iterator) {
      return !other.m_iterator;
    } else if(!other.m_iterator) {
      return false;
    }
    return m_iterator->equals(*other.m_iterator);
  }

  template<typename T>
  void AnyIterator<T>::advance(difference_type n) {
    m_iterator->advance(n);
  }

  template<typename T>
  typename AnyIterator<T>::difference_type AnyIterator<T>::distance_to(
      const AnyIterator& other) const {
    if(!m_iterator && !other.m_iterator) {
      return 0;
    }
    return m_iterator->distance_to(*other.m_iterator);
  }

  template<typename T>
  typename AnyIterator<T>::value_type& AnyIterator<T>::dereference() const {
    return m_iterator->dereference();
  }

  template<typename T>
  template<typename I>
  template<typename... Args>
  AnyIterator<T>::WrappedIterator<I>::WrappedIterator(Args&&... args)
    : m_iterator(std::forward<Args>(args)...) {}

  template<typename T>
  template<typename I>
  std::unique_ptr<typename AnyIterator<T>::VirtualIterator>
      AnyIterator<T>::WrappedIterator<I>::clone() const {
    return std::make_unique<WrappedIterator>(m_iterator);
  }

  template<typename T>
  template<typename I>
  bool AnyIterator<T>::WrappedIterator<I>::equals(
      const VirtualIterator& rhs) const {
    auto& rhs_iterator = static_cast<const WrappedIterator&>(rhs);
    return m_iterator == rhs_iterator.m_iterator;
  }

  template<typename T>
  template<typename I>
  const typename AnyIterator<T>::value_type&
      AnyIterator<T>::WrappedIterator<I>::dereference() const {
    return *m_iterator;
  }

  template<typename T>
  template<typename I>
  typename AnyIterator<T>::value_type&
      AnyIterator<T>::WrappedIterator<I>::dereference() {
    return *m_iterator;
  }

  template<typename T>
  template<typename I>
  void AnyIterator<T>::WrappedIterator<I>::increment() {
    ++m_iterator;
  }

  template<typename T>
  template<typename I>
  void AnyIterator<T>::WrappedIterator<I>::decrement() {
    Details::decrement(m_iterator,
      typename std::iterator_traits<Iterator>::iterator_category());
  }

  template<typename T>
  template<typename I>
  void AnyIterator<T>::WrappedIterator<I>::advance(difference_type n) {
    std::advance(m_iterator, n);
  }

  template<typename T>
  template<typename I>
  typename AnyIterator<T>::difference_type AnyIterator<T>::WrappedIterator<I>::
      distance_to(const VirtualIterator& rhs) const {
    auto& rhs_iterator = static_cast<const WrappedIterator&>(rhs);
    return std::distance(m_iterator, rhs_iterator.m_iterator);
  }
}

#endif
