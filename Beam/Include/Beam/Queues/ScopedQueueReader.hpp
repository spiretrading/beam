#ifndef BEAM_SCOPED_QUEUE_READER_HPP
#define BEAM_SCOPED_QUEUE_READER_HPP
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/QueueReader.hpp"

namespace Beam {

  /**
   * Stores a handle to a QueueReader that breaks when the object goes
   * out of scope.
   * @param <T> The data to read from the Queue.
   */
  template<typename T>
  class ScopedQueueReader {
    public:

      /** The type of QueueReader to manage. */
      using Queue = QueueReader<T>;

      /**
       * Constructs a ScopedQueueReader.
       * @param queue The QueueReader to manage.
       */
      ScopedQueueReader(std::shared_ptr<Queue> queue);

      ScopedQueueReader(ScopedQueueReader&&) = default;

      ~ScopedQueueReader();

      //! Returns a reference to the QueueReader.
      Queue& operator *() const;

      //! Returns a pointer to the QueueReader.
      Queue* operator ->() const;

      ScopedQueueReader& operator =(ScopedQueueReader&&) = default;

    private:
      std::shared_ptr<Queue> m_queue;

      ScopedQueueReader(const ScopedQueueReader&) = delete;
      ScopedQueueReader& operator =(const ScopedQueueReader&) = delete;
  };

  template<typename T>
  ScopedQueueReader<T>::ScopedQueueReader(std::shared_ptr<Queue> queue)
    : m_queue{std::move(queue)} {}

  template<typename T>
  ScopedQueueReader<T>::~ScopedQueueReader() {
    if(m_queue != nullptr) {
      m_queue->Break();
    }
  }

  template<typename T>
  typename ScopedQueueReader<T>::Queue&
      ScopedQueueReader<T>::operator *() const {
    return **m_queue;
  }

  template<typename T>
  typename ScopedQueueReader<T>::Queue*
      ScopedQueueReader<T>::operator ->() const {
    return &(*m_queue);
  }
}

#endif
