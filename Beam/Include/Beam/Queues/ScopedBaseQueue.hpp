#ifndef BEAM_SCOPED_BASE_QUEUE_HPP
#define BEAM_SCOPED_BASE_QUEUE_HPP
#include <memory>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queues/BaseQueue.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /**
   * Stores a handle to a BaseQueue that breaks when the object goes out of
   * scope.
   */
  template<typename Q = std::shared_ptr<BaseQueue>>
  class ScopedBaseQueue : public BaseQueue {
    public:

      /** The type of BaseQueue being handled. */
      using BaseQueue = dereference_t<Q>;

      /**
       * Constructs a ScopedBaseQueue.
       * @param queue The BaseQueue to manage.
       */
      template<Initializes<Q> QF,
        typename = disable_copy_constructor_t<ScopedBaseQueue, QF>>
      ScopedBaseQueue(QF&& queue);

      template<typename U>
      ScopedBaseQueue(ScopedBaseQueue<U>&& queue);
      ~ScopedBaseQueue() override;

      void close(const std::exception_ptr& e) override;
      template<typename U>
      ScopedBaseQueue& operator =(ScopedBaseQueue<U>&& queue);
      using Beam::BaseQueue::close;

    private:
      template<typename> friend class ScopedBaseQueue;
      local_ptr_t<Q> m_queue;
  };

  template<typename Q, typename = disable_copy_constructor_t<
    ScopedBaseQueue<std::remove_cvref_t<Q>>, Q>>
  ScopedBaseQueue(Q&&) -> ScopedBaseQueue<std::remove_cvref_t<Q>>;

  template<typename Q>
  template<Initializes<Q> QF, typename>
  ScopedBaseQueue<Q>::ScopedBaseQueue(QF&& queue)
    : m_queue(std::forward<QF>(queue)) {}

  template<typename Q>
  template<typename U>
  ScopedBaseQueue<Q>::ScopedBaseQueue(ScopedBaseQueue<U>&& queue)
    : m_queue(std::move(queue.m_queue)) {}

  template<typename Q>
  ScopedBaseQueue<Q>::~ScopedBaseQueue() {
    if(m_queue) {
      m_queue->close();
    }
  }

  template<typename Q>
  void ScopedBaseQueue<Q>::close(const std::exception_ptr& e) {
    if(m_queue) {
      m_queue->close(e);
    }
  }

  template<typename Q>
  template<typename U>
  ScopedBaseQueue<Q>& ScopedBaseQueue<Q>::operator =(
      ScopedBaseQueue<U>&& queue) {
    close();
    m_queue = std::move(queue.m_queue);
    return *this;
  }
}

#endif
