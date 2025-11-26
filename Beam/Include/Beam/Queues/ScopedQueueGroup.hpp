#ifndef BEAM_SCOPED_QUEUE_GROUP_HPP
#define BEAM_SCOPED_QUEUE_GROUP_HPP
#include <memory>
#include "Beam/Collections/SynchronizedList.hpp"
#include "Beam/Queues/BaseQueue.hpp"

namespace Beam {

  /**
   * Container for a group of queues that get closed together on destruction.
   */
  class ScopedQueueGroup {
    public:

      /** Constructs an empty group. */
      ScopedQueueGroup() = default;

      ~ScopedQueueGroup();

      /**
       * Adds a queue to this group.
       * @param queue The queue to add.
       */
      template<typename T>
      T add(const T& queue);

      /** Closes all queues in the group with the default exception. */
      void close();

      /**
       * Closes all queues in the group with a specified exception.
       * @param e The reason why the Queue was broken.
       */
      void close(const std::exception_ptr& e);

      /**
       * Closes all queues in the group with a specified exception.
       * @param e The reason why the Queue was broken.
       */
      template<typename E>
      void close(const E& e);

    private:
      SynchronizedVector<std::shared_ptr<BaseQueue>> m_queues;

      ScopedQueueGroup(const ScopedQueueGroup&) = delete;
      ScopedQueueGroup(ScopedQueueGroup&&) = delete;
  };

  ScopedQueueGroup::~ScopedQueueGroup() {
    close();
  }

  template<typename T>
  T ScopedQueueGroup::add(const T& queue) {
    m_queues.push_back(queue);
    return queue;
  }

  inline void ScopedQueueGroup::close() {
    close(PipeBrokenException());
  }

  inline void ScopedQueueGroup::close(const std::exception_ptr& e) {
    auto queues = std::vector<std::shared_ptr<BaseQueue>>();
    m_queues.swap(queues);
    for(auto& queue : queues) {
      queue->close(e);
    }
  }

  template<typename E>
  void ScopedQueueGroup::close(const E& e) {
    close(std::make_exception_ptr(e));
  }
}

#endif
