#ifndef BEAM_QUEUE_REACTOR_HPP
#define BEAM_QUEUE_REACTOR_HPP
#include <deque>
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
        \param trigger The Trigger to signal when an update is available.
      */
      QueueReactor(std::shared_ptr<QueueReader<Type>> queue,
        RefType<Trigger> trigger);

      ~QueueReactor();

      virtual bool IsComplete() const override final;

      virtual BaseReactor::Update Commit(int sequenceNumber) override final;

      virtual Type Eval() const override final;

    private:
      mutable boost::mutex m_mutex;
      std::shared_ptr<QueueReader<Type>> m_queue;
      Trigger* m_trigger;
      Expect<Type> m_value;
      bool m_isComplete;
      int m_currentSequenceNumber;
      int m_nextSequenceNumber;
      BaseReactor::Update m_update;
      Threading::ConditionVariable m_monitorCondition;
      Routines::RoutineHandler m_monitorRoutine;

      void MonitorQueue();
  };

  //! Makes a QueueReactor.
  /*!
    \param queue The Queue to monitor.
    \param trigger The Trigger to signal when an update is available.
  */
  template<typename QR>
  auto MakeQueueReactor(std::shared_ptr<QR> queue, RefType<Trigger> trigger) {
    return std::make_shared<QueueReactor<typename QR::Target>>(
      std::static_pointer_cast<QueueReader<typename QR::Target>>(queue),
      Ref(trigger));
  }

  template<typename Type>
  auto MakePublisherReactor(const Publisher<Type>& publisher,
      RefType<Trigger> trigger) {
    auto queue = std::make_shared<Queue<Type>>();
    publisher.Monitor(queue);
    return MakeQueueReactor(queue, Ref(trigger));
  }

  template<typename T>
  QueueReactor<T>::QueueReactor(std::shared_ptr<QueueReader<Type>> queue,
      RefType<Trigger> trigger)
      : m_queue{std::move(queue)},
        m_trigger{trigger.Get()},
        m_value{std::make_exception_ptr(ReactorUnavailableException{})},
        m_isComplete{false},
        m_currentSequenceNumber{-1},
        m_nextSequenceNumber{0} {}

  template<typename T>
  QueueReactor<T>::~QueueReactor() {
    m_queue->Break();
    boost::lock_guard<boost::mutex> lock{m_mutex};
    m_isComplete = true;
    m_monitorCondition.notify_one();
  }

  template<typename T>
  bool QueueReactor<T>::IsComplete() const {
    return m_isComplete;
  }

  template<typename T>
  BaseReactor::Update QueueReactor<T>::Commit(int sequenceNumber) {
    if(sequenceNumber == m_currentSequenceNumber) {
      return m_update;
    }
    {
      boost::lock_guard<boost::mutex> lock{m_mutex};
      if(sequenceNumber != m_nextSequenceNumber) {
        return BaseReactor::Update::NONE;
      }
    }
    try {
      m_value = std::move(m_queue->Top());
      m_queue->Pop();
      m_update = BaseReactor::Update::EVAL;
    } catch(const PipeBrokenException&) {
      m_update = BaseReactor::Update::COMPLETE;
      m_isComplete = true;
    } catch(const std::exception&) {
      m_value = std::current_exception();
      m_isComplete = true;
      m_update = BaseReactor::Update::EVAL;
    }
    {
      boost::lock_guard<boost::mutex> lock{m_mutex};
      if(m_nextSequenceNumber == 0) {
        m_monitorRoutine = Routines::Spawn(
          std::bind(&QueueReactor::MonitorQueue, this));
      }
      m_currentSequenceNumber = m_nextSequenceNumber;
      m_nextSequenceNumber = -1;
      m_monitorCondition.notify_one();
    }
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
      boost::unique_lock<boost::mutex> lock{m_mutex};
      if(m_isComplete) {
        break;
      }
      m_trigger->SignalUpdate(Store(m_nextSequenceNumber));
      m_monitorCondition.wait(lock);
      if(m_isComplete) {
        break;
      }
    }
  }
}
}

#endif
