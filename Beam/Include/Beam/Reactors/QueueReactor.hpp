#ifndef BEAM_QUEUE_REACTOR_HPP
#define BEAM_QUEUE_REACTOR_HPP
#include <Aspen/Queue.hpp>
#include "Beam/Queues/ScopedQueueReader.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Routines/RoutineHandler.hpp"

namespace Beam::Reactors {

  /**
   * Evaluates to values published by a Queue.
   * @param <T> The type of values to publish.
   */
  template<typename T>
  class QueueReactor {
    public:
      using Type = T;

      //! Constructs an empty QueueReactor.
      /*!
        \param queue The Queue to monitor.
      */
      QueueReactor(std::shared_ptr<QueueReader<Type>> queue);

      Aspen::State commit(int sequence) noexcept;

      Aspen::eval_result_t<Type> eval() const;

    private:
      Aspen::Queue<Type> m_reactor;
      Routines::RoutineHandler m_handler;
      ScopedQueueReader<Type> m_queue;

      void MonitorQueue();
  };

  template<typename Q>
  QueueReactor(std::shared_ptr<Q>) -> QueueReactor<typename Q::Target>;

  template<typename T>
  QueueReactor<T>::QueueReactor(std::shared_ptr<QueueReader<Type>> queue)
      : m_queue(std::move(queue)) {
    m_handler = Routines::Spawn([=] { MonitorQueue(); });
  }

  template<typename T>
  Aspen::State QueueReactor<T>::commit(int sequence) noexcept {
    return m_reactor.commit(sequence);
  }

  template<typename T>
  Aspen::eval_result_t<typename QueueReactor<T>::Type>
      QueueReactor<T>::eval() const {
    return m_reactor.eval();
  }

  template<typename T>
  void QueueReactor<T>::MonitorQueue() {
    while(true) {
      try {
        m_reactor.push(m_queue->Top());
        m_queue->Pop();
      } catch(const PipeBrokenException&) {
        m_reactor.set_complete();
        break;
      } catch(const std::exception&) {
        m_reactor.set_complete(std::current_exception());
        break;
      }
    }
  }
}

#endif
