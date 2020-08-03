#ifndef BEAM_SCOPED_BASE_QUEUE_HPP
#define BEAM_SCOPED_BASE_QUEUE_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/BaseQueue.hpp"

namespace Beam {

  /**
   * Stores a handle to a BaseQueue that breaks when the object goes out of
   * scope.
   */
  template<typename Q = std::shared_ptr<BaseQueue>>
  class ScopedBaseQueue : public BaseQueue {
    public:

      /** The type of BaseQueue being handled. */
      using BaseQueue = GetTryDereferenceType<Q>;

      /**
       * Constructs a ScopedBaseQueue.
       * @param queue The BaseQueue to manage.
       */
      template<typename QF, typename = std::enable_if_t<
        !std::is_base_of_v<ScopedBaseQueue, std::decay_t<QF>>>>
      ScopedBaseQueue(QF&& queue);

      template<typename U>
      ScopedBaseQueue(ScopedBaseQueue<U>&& queue);

      ScopedBaseQueue(ScopedBaseQueue&& queue);

      ~ScopedBaseQueue() override;

      void Break(const std::exception_ptr& e) override;

      ScopedBaseQueue& operator =(ScopedBaseQueue&& queue);

      template<typename U>
      ScopedBaseQueue& operator =(ScopedBaseQueue<U>&& queue);

      using Beam::BaseQueue::Break;

    private:
      template<typename> friend class ScopedBaseQueue;
      GetOptionalLocalPtr<Q> m_queue;
  };

  template<typename Q, typename = std::enable_if_t<
    !std::is_base_of_v<ScopedBaseQueue<std::decay_t<Q>>, std::decay_t<Q>>>>
  ScopedBaseQueue(Q&&) -> ScopedBaseQueue<std::decay_t<Q>>;

  template<typename Q>
  template<typename QF, typename>
  ScopedBaseQueue<Q>::ScopedBaseQueue(QF&& queue)
    : m_queue(std::forward<QF>(queue)) {}

  template<typename Q>
  template<typename U>
  ScopedBaseQueue<Q>::ScopedBaseQueue(ScopedBaseQueue<U>&& queue)
    : m_queue(std::move(queue.m_queue)) {}

  template<typename Q>
  ScopedBaseQueue<Q>::ScopedBaseQueue(ScopedBaseQueue&& queue)
    : m_queue(std::move(queue.m_queue)) {}

  template<typename Q>
  ScopedBaseQueue<Q>::~ScopedBaseQueue() {
    if(m_queue) {
      m_queue->Break();
    }
  }

  template<typename Q>
  void ScopedBaseQueue<Q>::Break(const std::exception_ptr& e) {
    if(m_queue) {
      m_queue->Break(e);
    }
  }

  template<typename Q>
  ScopedBaseQueue<Q>& ScopedBaseQueue<Q>::operator =(ScopedBaseQueue&& queue) {
    Break();
    m_queue = std::move(queue.m_queue);
    return *this;
  }

  template<typename Q>
  template<typename U>
  ScopedBaseQueue<Q>& ScopedBaseQueue<Q>::operator =(
      ScopedBaseQueue<U>&& queue) {
    Break();
    m_queue = std::move(queue.m_queue);
    return *this;
  }
}

#endif
