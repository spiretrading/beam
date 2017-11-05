#ifndef BEAM_PYTHON_QUEUE_REACTOR_HPP
#define BEAM_PYTHON_QUEUE_REACTOR_HPP
#include <memory>
#include <boost/thread/mutex.hpp>
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Python/GilRelease.hpp"
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

  /*! \class PythonQueueReactor
      \brief Implements a QueueReactor that can safely be used within Python.
   */
  class PythonQueueReactor : public Reactor<boost::python::object> {
    public:

      //! Constructs a PythonQueueReactor.
      /*!
        \param queue The Queue to monitor.
        \param trigger The Trigger to signal when an update is available.
      */
      PythonQueueReactor(std::shared_ptr<QueueReader<Type>> queue,
        RefType<Trigger> trigger);

      virtual ~PythonQueueReactor() override final;

      virtual bool IsComplete() const override final;

      virtual BaseReactor::Update Commit(int sequenceNumber) override final;

      virtual Type Eval() const override final;

    private:
      mutable Threading::Mutex m_mutex;
      std::shared_ptr<QueueReader<Type>> m_queue;
      Trigger* m_trigger;
      Expect<Type> m_value;
      bool m_hasValue;
      bool m_isComplete;
      int m_currentSequenceNumber;
      int m_nextSequenceNumber;
      BaseReactor::Update m_update;
      Threading::ConditionVariable m_monitorCondition;
      Routines::RoutineHandler m_monitorRoutine;

      void MonitorQueue();
  };

  //! Makes a PythonQueueReactor.
  /*!
    \param queue The Queue to monitor.
    \param trigger The Trigger to signal when an update is available.
  */
  inline auto MakeQueueReactor(
      std::shared_ptr<QueueReader<boost::python::object>> queue,
      RefType<Trigger> trigger) {
    return std::make_shared<PythonQueueReactor>(std::move(queue),
      Ref(trigger));
  }

  inline PythonQueueReactor::PythonQueueReactor(
      std::shared_ptr<QueueReader<Type>> queue, RefType<Trigger> trigger)
      : m_queue{std::move(queue)},
        m_trigger{trigger.Get()},
        m_value{std::make_exception_ptr(ReactorUnavailableException{})},
        m_hasValue{false},
        m_isComplete{false},
        m_currentSequenceNumber{-1},
        m_nextSequenceNumber{0} {}

  inline PythonQueueReactor::~PythonQueueReactor() {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> gilLock{gil};
    m_queue->Break();
    boost::lock_guard<Threading::Mutex> lock{m_mutex};
    m_isComplete = true;
    m_monitorCondition.notify_one();
  }

  inline bool PythonQueueReactor::IsComplete() const {
    return m_isComplete;
  }

  inline BaseReactor::Update PythonQueueReactor::Commit(int sequenceNumber) {
    if(sequenceNumber == m_currentSequenceNumber) {
      return m_update;
    } else if(sequenceNumber == 0 && m_currentSequenceNumber != -1) {
      if(m_hasValue) {
        return BaseReactor::Update::EVAL;
      }
      return BaseReactor::Update::COMPLETE;
    }
    {
      Python::GilRelease gil;
      boost::lock_guard<Python::GilRelease> gilLock{gil};
      boost::lock_guard<Threading::Mutex> lock{m_mutex};
      if(sequenceNumber != m_nextSequenceNumber) {
        return BaseReactor::Update::NONE;
      }
      if(sequenceNumber == 0) {
        m_currentSequenceNumber = 0;
        m_nextSequenceNumber = -1;
        m_update = BaseReactor::Update::NONE;
        m_monitorRoutine = Routines::Spawn(
          std::bind(&PythonQueueReactor::MonitorQueue, this));
        return BaseReactor::Update::NONE;
      }
    }
    try {
      m_value = std::move(m_queue->Top());
      m_queue->Pop();
      m_hasValue = true;
      m_update = BaseReactor::Update::EVAL;
    } catch(const PipeBrokenException&) {
      m_update = BaseReactor::Update::COMPLETE;
      m_isComplete = true;
    } catch(const std::exception&) {
      m_value = std::current_exception();
      m_isComplete = true;
      m_hasValue = true;
      m_update = BaseReactor::Update::EVAL;
    }
    {
      Python::GilRelease gil;
      boost::lock_guard<Python::GilRelease> gilLock{gil};
      boost::lock_guard<Threading::Mutex> lock{m_mutex};
      m_currentSequenceNumber = m_nextSequenceNumber;
      m_nextSequenceNumber = -1;
      m_monitorCondition.notify_one();
    }
    return m_update;
  }

  inline PythonQueueReactor::Type PythonQueueReactor::Eval() const {
    return m_value.Get();
  }

  inline void PythonQueueReactor::MonitorQueue() {
    while(true) {
      try {
        m_queue->Top();
      } catch(const std::exception&) {}
      boost::unique_lock<Threading::Mutex> lock{m_mutex};
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
