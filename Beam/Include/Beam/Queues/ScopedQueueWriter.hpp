#ifndef BEAM_SCOPED_QUEUE_WRITER_HPP
#define BEAM_SCOPED_QUEUE_WRITER_HPP
#include <memory>
#include <type_traits>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queues/QueueWriter.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /**
   * Stores a handle to a QueueWriter that breaks when the object goes out of
   * scope.
   * @tparam T The type of data to read from the QueueWriter.
   * @tparam Q The type of QueueWriter to handle.
   */
  template<typename T, typename Q = std::shared_ptr<QueueWriter<T>>>
  class ScopedQueueWriter : public QueueWriter<T> {
    public:

      /** The type of QueueWriter to handle. */
      using QueueWriter = dereference_t<Q>;
      using Target = typename Beam::QueueWriter<T>::Target;

      /**
       * Constructs a ScopedQueueWriter.
       * @param queue The QueueWriter to manage.
       */
      template<Initializes<Q> QF,
        typename = disable_copy_constructor_t<ScopedQueueWriter, QF>>
      ScopedQueueWriter(QF&& queue) noexcept(
        std::is_nothrow_constructible_v<local_ptr_t<Q>, QF&&>);

      template<typename U>
      ScopedQueueWriter(ScopedQueueWriter<Target, U>&& queue) noexcept(
        std::is_nothrow_constructible_v<local_ptr_t<Q>, local_ptr_t<U>&&>);
      ~ScopedQueueWriter() override;

      void push(const Target& value) override;
      void push(Target&& value) override;
      void close(const std::exception_ptr& e) override;
      template<typename U>
      ScopedQueueWriter& operator =(ScopedQueueWriter<Target, U>&& queue)
        noexcept(
          std::is_nothrow_assignable_v<local_ptr_t<Q>&, local_ptr_t<U>&&>);
      using Beam::QueueWriter<T>::close;

    private:
      template<typename, typename> friend class ScopedQueueWriter;
      local_ptr_t<Q> m_queue;
  };

  template<typename Q>
  ScopedQueueWriter(Q&&) -> ScopedQueueWriter<
    typename dereference_t<Q>::Target, std::remove_cvref_t<Q>>;

  template<typename T, typename Q>
  template<Initializes<Q> QF, typename>
  ScopedQueueWriter<T, Q>::ScopedQueueWriter(QF&& queue) noexcept(
    std::is_nothrow_constructible_v<local_ptr_t<Q>, QF&&>)
    : m_queue(std::forward<QF>(queue)) {}

  template<typename T, typename Q>
  template<typename U>
  ScopedQueueWriter<T, Q>::ScopedQueueWriter(
    ScopedQueueWriter<Target, U>&& queue) noexcept(
      std::is_nothrow_constructible_v<local_ptr_t<Q>, local_ptr_t<U>&&>)
    : m_queue(std::move(queue.m_queue)) {}

  template<typename T, typename Q>
  ScopedQueueWriter<T, Q>::~ScopedQueueWriter() {
    if(m_queue) {
      m_queue->close();
    }
  }

  template<typename T, typename Q>
  void ScopedQueueWriter<T, Q>::push(const Target& value) {
    if(m_queue) {
      m_queue->push(value);
    }
  }

  template<typename T, typename Q>
  void ScopedQueueWriter<T, Q>::push(Target&& value) {
    if(m_queue) {
      m_queue->push(std::move(value));
    }
  }

  template<typename T, typename Q>
  void ScopedQueueWriter<T, Q>::close(const std::exception_ptr& e) {
    if(m_queue) {
      m_queue->close(e);
    }
  }

  template<typename T, typename Q>
  template<typename U>
  ScopedQueueWriter<T, Q>& ScopedQueueWriter<T, Q>::operator =(
      ScopedQueueWriter<Target, U>&& queue) noexcept(
        std::is_nothrow_assignable_v<local_ptr_t<Q>&, local_ptr_t<U>&&>) {
    close();
    m_queue = std::move(queue.m_queue);
    return *this;
  }
}

#endif
