#ifndef BEAM_SCOPED_QUEUE_READER_HPP
#define BEAM_SCOPED_QUEUE_READER_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/QueueReader.hpp"

namespace Beam {

  /**
   * Stores a handle to a QueueReader that breaks when the object goes out of
   * scope.
   * @param <T> The type of data to read from the QueueReader.
   * @param <Q> The type of QueueReader to handle.
   */
  template<typename T, typename Q = std::shared_ptr<QueueReader<T>>>
  class ScopedQueueReader : public QueueReader<T> {
    public:

      /** The type of QueueReader to handle. */
      using QueueReader = GetTryDereferenceType<Q>;
      using Target = typename Beam::QueueReader<T>::Target;

      /**
       * Constructs a ScopedQueueReader.
       * @param queue The QueueReader to manage.
       */
      template<typename QF, typename = std::enable_if_t<
        !std::is_base_of_v<ScopedQueueReader, std::decay_t<QF>> &&
        std::is_same_v<typename GetTryDereferenceType<QF>::Target,
        typename QueueReader::Target>>>
      ScopedQueueReader(QF&& queue);

      template<typename U>
      ScopedQueueReader(ScopedQueueReader<Target, U>&& queue);

      ScopedQueueReader(ScopedQueueReader&& queue);

      ~ScopedQueueReader() override;

      Target Top() const override;

      boost::optional<Target> TryTop() const override;

      Target Pop() override;

      boost::optional<Target> TryPop() override;

      void Break(const std::exception_ptr& e) override;

      ScopedQueueReader& operator =(ScopedQueueReader&& queue);

      template<typename U>
      ScopedQueueReader& operator =(ScopedQueueReader<Target, U>&& queue);

      using Beam::QueueReader<T>::Break;

    private:
      GetOptionalLocalPtr<Q> m_queue;
  };

  template<typename Q, typename = std::enable_if_t<!std::is_base_of_v<
    ScopedQueueReader<typename GetTryDereferenceType<Q>::Target,
    std::decay_t<Q>>, std::decay_t<Q>>>>
  ScopedQueueReader(Q&&) -> ScopedQueueReader<
    typename GetTryDereferenceType<Q>::Target, std::decay_t<Q>>;

  template<typename T, typename Q>
  template<typename QF, typename>
  ScopedQueueReader<T, Q>::ScopedQueueReader(QF&& queue)
    : m_queue(std::forward<QF>(queue)) {}

  template<typename T, typename Q>
  template<typename U>
  ScopedQueueReader<T, Q>::ScopedQueueReader(
    ScopedQueueReader<Target, U>&& queue)
    : m_queue(std::move(queue.m_queue)) {}

  template<typename T, typename Q>
  ScopedQueueReader<T, Q>::ScopedQueueReader(ScopedQueueReader&& queue)
    : m_queue(std::move(queue.m_queue)) {}

  template<typename T, typename Q>
  ScopedQueueReader<T, Q>::~ScopedQueueReader() {
    if(m_queue) {
      m_queue->Break();
    }
  }

  template<typename T, typename Q>
  typename ScopedQueueReader<T, Q>::Target
      ScopedQueueReader<T, Q>::Top() const {
    return m_queue->Top();
  }

  template<typename T, typename Q>
  boost::optional<typename ScopedQueueReader<T, Q>::Target>
      ScopedQueueReader<T, Q>::TryTop() const {
    return m_queue->TryTop();
  }

  template<typename T, typename Q>
  typename ScopedQueueReader<T, Q>::Target ScopedQueueReader<T, Q>::Pop() {
    return m_queue->Pop();
  }

  template<typename T, typename Q>
  boost::optional<typename ScopedQueueReader<T, Q>::Target>
      ScopedQueueReader<T, Q>::TryPop() {
    return m_queue->TryPop();
  }

  template<typename T, typename Q>
  void ScopedQueueReader<T, Q>::Break(const std::exception_ptr& e) {
    if(m_queue) {
      m_queue->Break(e);
    }
  }

  template<typename T, typename Q>
  ScopedQueueReader<T, Q>& ScopedQueueReader<T, Q>::operator =(
      ScopedQueueReader&& queue) {
    Break();
    m_queue = std::move(queue.m_queue);
    return *this;
  }

  template<typename T, typename Q>
  template<typename U>
  ScopedQueueReader<T, Q>& ScopedQueueReader<T, Q>::operator =(
      ScopedQueueReader<Target, U>&& queue) {
    Break();
    m_queue = std::move(queue.m_queue);
    return *this;
  }
}

#endif
