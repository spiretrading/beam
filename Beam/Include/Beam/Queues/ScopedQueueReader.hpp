#ifndef BEAM_SCOPED_QUEUE_READER_HPP
#define BEAM_SCOPED_QUEUE_READER_HPP
#include <memory>
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/QueueReader.hpp"

namespace Beam {

  /**
   * Stores a handle to a QueueReader that breaks when the object goes out of
   * scope.
   * @param <T> The type of data to read from the QueueReader.
   */
  template<typename T>
  class ScopedQueueReader : public QueueReader<T> {
    public:
      using Target = typename QueueReader<T>::Target;

      /**
       * Constructs a ScopedQueueReader.
       * @param queue The QueueReader to manage.
       */
      ScopedQueueReader(std::shared_ptr<QueueReader<Target>> queue);

      ScopedQueueReader(ScopedQueueReader&& queue) = default;

      ~ScopedQueueReader() override;

      bool IsEmpty() const override;

      Target Pop() override;

      ScopedQueueReader& operator =(ScopedQueueReader&& queue) = default;

    private:
      std::shared_ptr<QueueReader<Target>> m_queue;

      ScopedQueueReader(const ScopedQueueReader&) = delete;
      ScopedQueueReader& operator =(const ScopedQueueReader&) = delete;
  };

  template<typename T>
  ScopedQueueReader<T>::ScopedQueueReader(
    std::shared_ptr<QueueReader<Target>> queue)
    : m_queue(std::move(queue)) {}

  template<typename T>
  ScopedQueueReader<T>::~ScopedQueueReader() {
    if(m_queue) {
      m_queue->Break();
    }
  }

  template<typename T>
  bool ScopedQueueReader<T>::IsEmpty() const {
    return m_queue->IsEmpty();
  }

  template<typename T>
  typename ScopedQueueReader<T>::Target ScopedQueueReader<T>::Pop() {
    return m_queue->Pop();
  }
}

#endif
