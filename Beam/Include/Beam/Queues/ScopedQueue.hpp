#ifndef BEAM_SCOPED_QUEUE_HPP
#define BEAM_SCOPED_QUEUE_HPP
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/QueueReader.hpp"

namespace Beam {

  /**
   * Stores a handle to a BaseQueue that breaks when the object goes out of
   * scope.
   * @param <Q> The data to read from the Queue.
   */
  template<typename Q>
  class ScopedQueue {
    public:

      /** The type of queue to manage. */
      using Queue = GetTryDereferenceType<Q>;

      /**
       * Constructs a ScopedQueue.
       * @param queue The BaseQueue to manage.
       */
      template<typename QF>
      ScopedQueue(QF&& queue);

      ScopedQueue(ScopedQueue&& queue);

      ~ScopedQueue();

      //! Returns a reference to the QueueReader.
      Queue& operator *() const;

      //! Returns a pointer to the QueueReader.
      Queue* operator ->() const;

      ScopedQueue& operator =(ScopedQueue&& queue);

    private:
      bool m_isBroken;
      GetOptionalLocalPtr<Q> m_queue;

      ScopedQueue(const ScopedQueue&) = delete;
      ScopedQueue& operator =(const ScopedQueue&) = delete;
  };

  template<typename Q>
  template<typename QF>
  ScopedQueue<Q>::ScopedQueue(QF&& queue)
    : m_isBroken(false),
      m_queue(std::forward<QF>(queue)) {}

  template<typename Q>
  ScopedQueue<Q>::ScopedQueue(ScopedQueue&& queue)
    : m_isBroken(queue.m_isBroken),
      m_queue(std::move(queue.m_queue)) {
    queue.m_isBroken = true;
  }

  template<typename Q>
  ScopedQueue<Q>::~ScopedQueue() {
    if(!m_isBroken) {
      m_queue->Break();
    }
  }

  template<typename Q>
  typename ScopedQueue<Q>::Queue& ScopedQueue<Q>::operator *() const {
    return *m_queue;
  }

  template<typename Q>
  typename ScopedQueue<Q>::Queue* ScopedQueue<Q>::operator ->() const {
    return &(*m_queue);
  }

  template<typename Q>
  ScopedQueue<Q>& ScopedQueue<Q>::operator =(ScopedQueue&& queue) {
    m_isBroken = queue.m_isBroken;
    m_queue = std::move(queue.m_queue);
    queue.m_isBroken = true;
    return *this;
  }
}

#endif
