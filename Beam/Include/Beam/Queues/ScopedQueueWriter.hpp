#ifndef BEAM_SCOPED_QUEUE_WRITER_HPP
#define BEAM_SCOPED_QUEUE_WRITER_HPP
#include <memory>
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/QueueWriter.hpp"

namespace Beam {

  /**
   * Stores a handle to a QueueWriter that breaks when the object goes out of
   * scope.
   * @param <T> The type of data to read from the QueueWriter.
   */
  template<typename T>
  class ScopedQueueWriter : public QueueWriter<T> {
    public:
      using Source = typename QueueWriter<T>::Source;

      /**
       * Constructs a ScopedQueueWriter.
       * @param queue The QueueWriter to manage.
       */
      ScopedQueueWriter(std::shared_ptr<QueueWriter<Source>> queue);

      ScopedQueueWriter(ScopedQueueWriter&& queue) = default;

      ~ScopedQueueWriter() override;

      void Push(const Source& value) override;

      void Push(Source&& value) override;

      ScopedQueueWriter& operator =(ScopedQueueWriter&& queue) = default;

    private:
      std::shared_ptr<QueueWriter<Source>> m_queue;

      ScopedQueueWriter(const ScopedQueueWriter&) = delete;
      ScopedQueueWriter& operator =(const ScopedQueueWriter&) = delete;
  };

  template<typename T>
  ScopedQueueWriter<T>::ScopedQueueWriter(
    std::shared_ptr<QueueWriter<Source>> queue)
    : m_queue(std::move(queue)) {}

  template<typename T>
  ScopedQueueWriter<T>::~ScopedQueueWriter() {
    if(m_queue) {
      m_queue->Break();
    }
  }

  template<typename T>
  void ScopedQueueWriter<T>::Push(const Source& value) {
    m_queue->Push(value);
  }

  template<typename T>
  void ScopedQueueWriter<T>::Push(Source&& value) {
    m_queue->Push(std::move(value));
  }
}

#endif
