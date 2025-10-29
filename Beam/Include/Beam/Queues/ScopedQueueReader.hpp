#ifndef BEAM_SCOPED_QUEUE_READER_HPP
#define BEAM_SCOPED_QUEUE_READER_HPP
#include <memory>
#include <type_traits>
#include <boost/throw_exception.hpp>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queues/QueueReader.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /**
   * Stores a handle to a QueueReader that breaks when the object goes out of
   * scope.
   * @tparam T The type of data to read from the QueueReader.
   * @tparam Q The type of QueueReader to handle.
   */
  template<typename T, typename Q = std::shared_ptr<QueueReader<T>>>
  class ScopedQueueReader : public QueueReader<T> {
    public:

      /** The type of QueueReader to handle. */
      using QueueReader = dereference_t<Q>;
      using Source = typename Beam::QueueReader<T>::Source;

      /**
       * Constructs a ScopedQueueReader.
       * @param queue The QueueReader to manage.
       */
      template<Initializes<Q> QF,
        typename = disable_copy_constructor_t<ScopedQueueReader, QF>>
      ScopedQueueReader(QF&& queue) noexcept(
        std::is_nothrow_constructible_v<local_ptr_t<Q>, QF&&>);

      template<typename U>
      ScopedQueueReader(ScopedQueueReader<Source, U>&& queue) noexcept(
        std::is_nothrow_constructible_v<local_ptr_t<Q>, local_ptr_t<U>&&>);
      ~ScopedQueueReader() override;

      Source pop() override;
      boost::optional<Source> try_pop() override;
      void close(const std::exception_ptr& e) override;
      template<typename U>
      ScopedQueueReader& operator =(
        ScopedQueueReader<Source, U>&& queue) noexcept(
          std::is_nothrow_assignable_v<local_ptr_t<Q>&, local_ptr_t<U>&&>);
      using Beam::QueueReader<T>::close;

    private:
      local_ptr_t<Q> m_queue;
  };

  template<typename Q>
  ScopedQueueReader(Q&&) -> ScopedQueueReader<
    typename dereference_t<Q>::Source, std::remove_cvref_t<Q>>;

  template<typename T, typename Q>
  template<Initializes<Q> QF, typename>
  ScopedQueueReader<T, Q>::ScopedQueueReader(QF&& queue) noexcept(
    std::is_nothrow_constructible_v<local_ptr_t<Q>, QF&&>)
    : m_queue(std::forward<QF>(queue)) {}

  template<typename T, typename Q>
  template<typename U>
  ScopedQueueReader<T, Q>::ScopedQueueReader(
    ScopedQueueReader<Source, U>&& queue) noexcept(
      std::is_nothrow_constructible_v<local_ptr_t<Q>, local_ptr_t<U>&&>)
    : m_queue(std::move(queue.m_queue)) {}

  template<typename T, typename Q>
  ScopedQueueReader<T, Q>::~ScopedQueueReader() {
    close();
  }

  template<typename T, typename Q>
  typename ScopedQueueReader<T, Q>::Source ScopedQueueReader<T, Q>::pop() {
    if(m_queue) {
      return m_queue->pop();
    }
    boost::throw_with_location(PipeBrokenException());
  }

  template<typename T, typename Q>
  boost::optional<typename ScopedQueueReader<T, Q>::Source>
      ScopedQueueReader<T, Q>::try_pop() {
    if(m_queue) {
      return m_queue->try_pop();
    }
    return boost::none;
  }

  template<typename T, typename Q>
  void ScopedQueueReader<T, Q>::close(const std::exception_ptr& e) {
    if(m_queue) {
      m_queue->close(e);
    }
  }

  template<typename T, typename Q>
  template<typename U>
  ScopedQueueReader<T, Q>& ScopedQueueReader<T, Q>::operator =(
      ScopedQueueReader<Source, U>&& queue) noexcept(
        std::is_nothrow_assignable_v<local_ptr_t<Q>&, local_ptr_t<U>&&>) {
    close();
    m_queue = std::move(queue.m_queue);
    return *this;
  }
}

#endif
