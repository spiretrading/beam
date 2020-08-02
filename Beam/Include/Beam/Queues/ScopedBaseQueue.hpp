#ifndef BEAM_SCOPED_BASE_QUEUE_HPP
#define BEAM_SCOPED_BASE_QUEUE_HPP
#include <memory>
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/BaseQueue.hpp"

namespace Beam {

  /**
   * Stores a handle to a BaseQueue that breaks when the object goes out of
   * scope.
   */
  class ScopedBaseQueue : public BaseQueue {
    public:

      /**
       * Constructs a ScopedBaseQueue.
       * @param queue The BaseQueue to manage.
       */
      ScopedBaseQueue(std::shared_ptr<BaseQueue> queue);

      ScopedBaseQueue(ScopedBaseQueue&& queue) = default;

      ~ScopedBaseQueue() override;

      void Break(const std::exception_ptr& e) override;

      ScopedBaseQueue& operator =(ScopedBaseQueue&& queue) = default;

    private:
      std::shared_ptr<BaseQueue> m_queue;

      ScopedBaseQueue(const ScopedBaseQueue&) = delete;
      ScopedBaseQueue& operator =(const ScopedBaseQueue&) = delete;
  };

  inline ScopedBaseQueue::ScopedBaseQueue(std::shared_ptr<BaseQueue> queue)
    : m_queue(std::move(queue)) {}

  inline ScopedBaseQueue::~ScopedBaseQueue() {
    if(m_queue) {
      m_queue->Break();
    }
  }

  inline void ScopedBaseQueue::Break(const std::exception_ptr& e) {
    m_queue->Break(e);
  }
}

#endif
