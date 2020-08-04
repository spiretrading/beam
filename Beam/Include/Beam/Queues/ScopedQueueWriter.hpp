#ifndef BEAM_SCOPED_QUEUE_WRITER_HPP
#define BEAM_SCOPED_QUEUE_WRITER_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/QueueWriter.hpp"

namespace Beam {

  /**
   * Stores a handle to a QueueWriter that breaks when the object goes out of
   * scope.
   * @param <T> The type of data to read from the QueueWriter.
   * @param <Q> The type of QueueWriter to handle.
   */
  template<typename T, typename Q = std::shared_ptr<QueueWriter<T>>>
  class ScopedQueueWriter : public QueueWriter<T> {
    public:

      /** The type of QueueWriter to handle. */
      using QueueWriter = GetTryDereferenceType<Q>;
      using Source = typename Beam::QueueWriter<T>::Source;

      /**
       * Constructs a ScopedQueueWriter.
       * @param queue The QueueWriter to manage.
       */
      template<typename QF, typename = std::enable_if_t<
        !std::is_base_of_v<ScopedQueueWriter, std::decay_t<QF>> &&
        std::is_same_v<typename GetTryDereferenceType<QF>::Source,
        typename QueueWriter::Source>>>
      ScopedQueueWriter(QF&& queue);

      template<typename U>
      ScopedQueueWriter(ScopedQueueWriter<Source, U>&& queue);

      ScopedQueueWriter(ScopedQueueWriter&& queue);

      ~ScopedQueueWriter() override;

      void Push(const Source& value) override;

      void Push(Source&& value) override;

      void Break(const std::exception_ptr& e) override;

      ScopedQueueWriter& operator =(ScopedQueueWriter&& queue);

      template<typename U>
      ScopedQueueWriter& operator =(ScopedQueueWriter<Source, U>&& queue);

     using Beam::QueueWriter<T>::Break;

    private:
      template<typename, typename> friend class ScopedQueueWriter;
      GetOptionalLocalPtr<Q> m_queue;
  };

  template<typename Q, typename = std::enable_if_t<!std::is_base_of_v<
    ScopedQueueWriter<typename GetTryDereferenceType<Q>::Source,
    std::decay_t<Q>>, std::decay_t<Q>>>>
  ScopedQueueWriter(Q&&) -> ScopedQueueWriter<
    typename GetTryDereferenceType<Q>::Source, std::decay_t<Q>>;

  template<typename T, typename Q>
  template<typename QF, typename>
  ScopedQueueWriter<T, Q>::ScopedQueueWriter(QF&& queue)
    : m_queue(std::forward<QF>(queue)) {}

  template<typename T, typename Q>
  template<typename U>
  ScopedQueueWriter<T, Q>::ScopedQueueWriter(
    ScopedQueueWriter<Source, U>&& queue)
    : m_queue(std::move(queue.m_queue)) {}

  template<typename T, typename Q>
  ScopedQueueWriter<T, Q>::ScopedQueueWriter(ScopedQueueWriter&& queue)
    : m_queue(std::move(queue.m_queue)) {}

  template<typename T, typename Q>
  ScopedQueueWriter<T, Q>::~ScopedQueueWriter() {
    if(m_queue) {
      m_queue->Break();
    }
  }

  template<typename T, typename Q>
  void ScopedQueueWriter<T, Q>::Push(const Source& value) {
    m_queue->Push(value);
  }

  template<typename T, typename Q>
  void ScopedQueueWriter<T, Q>::Push(Source&& value) {
    m_queue->Push(std::move(value));
  }

  template<typename T, typename Q>
  void ScopedQueueWriter<T, Q>::Break(const std::exception_ptr& e) {
    if(m_queue) {
      m_queue->Break(e);
    }
  }

  template<typename T, typename Q>
  ScopedQueueWriter<T, Q>& ScopedQueueWriter<T, Q>::operator =(
      ScopedQueueWriter&& queue) {
    Break();
    m_queue = std::move(queue.m_queue);
    return *this;
  }

  template<typename T, typename Q>
  template<typename U>
  ScopedQueueWriter<T, Q>& ScopedQueueWriter<T, Q>::operator =(
      ScopedQueueWriter<Source, U>&& queue) {
    Break();
    m_queue = std::move(queue.m_queue);
    return *this;
  }
}

#endif
