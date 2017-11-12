#ifndef BEAM_QUEUE_REACTOR_HPP
#define BEAM_QUEUE_REACTOR_HPP
#include <memory>
#include <boost/thread/mutex.hpp>
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Queues/QueueReader.hpp"
#include "Beam/Reactors/Reactor.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Reactors/ReactorUnavailableException.hpp"
#include "Beam/Reactors/Trigger.hpp"
#include "Beam/Routines/RoutineHandler.hpp"
#include "Beam/Threading/ConditionVariable.hpp"
#include "Beam/Utilities/Expect.hpp"

namespace Beam {
namespace Reactors {

  /*! \class QueueReactor
      \brief Emits values published by a Queue.
   */
  template<typename T>
  class QueueReactor : public Reactor<T> {
    public:
      using Type = typename Reactor<T>::Type;

      //! Constructs a QueueReactor.
      /*!
        \param queue The Queue to monitor.
      */
      QueueReactor(std::shared_ptr<QueueReader<Type>> queue);

      virtual ~QueueReactor() override final;

      virtual BaseReactor::Update Commit(int sequenceNumber) override final;

      virtual Type Eval() const override final;

    private:
      mutable Threading::Mutex m_mutex;
      std::shared_ptr<QueueReader<Type>> m_queue;
      Trigger* m_trigger;
      Expect<Type> m_value;
      int m_nextSequenceNumber;
      int m_currentSequenceNumber;
      BaseReactor::Update m_update;
      BaseReactor::Update m_state;
      Threading::ConditionVariable m_monitorCondition;
      Routines::RoutineHandler m_monitorRoutine;

      void MonitorQueue();
  };

  //! Makes a QueueReactor.
  /*!
    \param queue The Queue to monitor.
  */
  template<typename QR>
  auto MakeQueueReactor(std::shared_ptr<QR> queue) {
    return std::make_shared<QueueReactor<typename QR::Target>>(
      std::static_pointer_cast<QueueReader<typename QR::Target>>(queue));
  }

  template<typename T>
  QueueReactor<T>::QueueReactor(std::shared_ptr<QueueReader<Type>> queue)
      : m_queue{std::move(queue)},
        m_value{std::make_exception_ptr(ReactorUnavailableException{})},
        m_nextSequenceNumber{0},
        m_currentSequenceNumber{-1},
        m_state{BaseReactor::Update::NONE} {}

  template<typename T>
  QueueReactor<T>::~QueueReactor() {
    m_queue->Break();
    boost::lock_guard<Threading::Mutex> lock{m_mutex};
    m_state = BaseReactor::Update::COMPLETE;
    m_monitorCondition.notify_one();
  }

  template<typename T>
  BaseReactor::Update QueueReactor<T>::Commit(int sequenceNumber) {
    if(m_currentSequenceNumber == sequenceNumber) {
      return m_update;
    } else if(sequenceNumber == 0 && m_currentSequenceNumber != -1) {
      return m_state;
    } else if(m_state & BaseReactor::Update::COMPLETE) {
      return BaseReactor::Update::NONE;
    }
    {
      boost::lock_guard<Threading::Mutex> lock{m_mutex};
      if(sequenceNumber != m_nextSequenceNumber) {
        return BaseReactor::Update::NONE;
      }
      if(sequenceNumber == 0) {
        m_nextSequenceNumber = -1;
        m_currentSequenceNumber = 0;
        m_update = BaseReactor::Update::NONE;
        m_trigger = &Trigger::GetEnvironmentTrigger();
        m_monitorRoutine = Routines::Spawn(
          std::bind(&QueueReactor::MonitorQueue, this));
        return BaseReactor::Update::NONE;
      }
    }
    try {
      m_value = std::move(m_queue->Top());
      m_queue->Pop();
      m_update = BaseReactor::Update::EVAL;
    } catch(const PipeBrokenException&) {
      m_update = BaseReactor::Update::COMPLETE;
    } catch(const std::exception&) {
      m_value = std::current_exception();
      m_update = BaseReactor::Update::EVAL;
    }
    {
      boost::lock_guard<Threading::Mutex> lock{m_mutex};
      m_nextSequenceNumber = -1;
      m_monitorCondition.notify_one();
    }
    m_currentSequenceNumber = sequenceNumber;
    m_state |= m_update;
    return m_update;
  }

  template<typename T>
  typename QueueReactor<T>::Type QueueReactor<T>::Eval() const {
    return m_value.Get();
  }

  template<typename T>
  void QueueReactor<T>::MonitorQueue() {
    while(true) {
      try {
        m_queue->Top();
      } catch(const std::exception&) {}
      boost::unique_lock<Threading::Mutex> lock{m_mutex};
      if(m_state & BaseReactor::Update::COMPLETE) {
        break;
      }
      m_trigger->SignalUpdate(Store(m_nextSequenceNumber));
      m_monitorCondition.wait(lock);
      if(m_state & BaseReactor::Update::COMPLETE) {
        break;
      }
    }
  }
}
}

#endif
