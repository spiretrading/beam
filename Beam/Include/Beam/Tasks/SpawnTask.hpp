#ifndef BEAM_SPAWNTASK_HPP
#define BEAM_SPAWNTASK_HPP
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Reactors/BaseReactor.hpp"
#include "Beam/Reactors/Control.hpp"
#include "Beam/Reactors/ReactorMonitor.hpp"
#include "Beam/Reactors/Trigger.hpp"
#include "Beam/SignalHandling/ScopedSlotAdaptor.hpp"
#include "Beam/Tasks/BasicTask.hpp"
#include "Beam/Tasks/Tasks.hpp"

namespace Beam {
namespace Tasks {

  /*! \class SpawnTask
      \brief Executes a new Task when a Reactor signals an update.
   */
  class SpawnTask : public BasicTask {
    public:

      //! Constructs a SpawnTask.
      /*!
        \param taskFactory The Task to execute on a trigger.
        \param trigger The Expression that triggers execution of a Task.
        \param reactorMonitor The ReactorMonitor to use.
      */
      SpawnTask(const TaskFactory& taskFactory,
        const std::shared_ptr<Reactors::BaseReactor>& trigger,
        RefType<Reactors::ReactorMonitor> reactorMonitor);

    protected:
      virtual void OnExecute();

      virtual void OnCancel();

    private:
      TaskFactory m_taskFactory;
      std::shared_ptr<Reactors::BaseReactor> m_trigger;
      Reactors::ReactorMonitor* m_reactorMonitor;
      Reactors::Trigger m_syncTrigger;
      bool m_triggerIsComplete;
      int m_taskCount;
      int m_state;
      SignalHandling::ScopedSlotAdaptor m_callbacks;
      boost::signals2::scoped_connection m_completeConnection;

      void OnTaskUpdate(const StateEntry& entry);
      void OnTrigger(const Expect<void>& value);
      void OnTriggerComplete();
      void S0();
      void S1();
      void S2(const StateEntry& state);
      void S3(const StateEntry& state);
      void S4();
      void S5();
  };

  /*! \class SpawnTaskFactory
      \brief Implements the TaskFactory for SpawnTasks.
   */
  class SpawnTaskFactory : public BasicTaskFactory<SpawnTaskFactory> {
    public:

      //! Constructs a SpawnTaskFactory.
      /*!
        \param taskFactory The Task to execute on a trigger.
        \param trigger The Expression that triggers execution of a Task.
        \param reactorMonitor The ReactorMonitor to use.
      */
      SpawnTaskFactory(const TaskFactory& taskFactory,
        const std::shared_ptr<Reactors::BaseReactor>& trigger,
        RefType<Reactors::ReactorMonitor> reactorMonitor);

      virtual std::shared_ptr<Task> Create();

    private:
      TaskFactory m_taskFactory;
      std::shared_ptr<Reactors::BaseReactor> m_trigger;
      Reactors::ReactorMonitor* m_reactorMonitor;
  };

  inline SpawnTask::SpawnTask(const TaskFactory& taskFactory,
        const std::shared_ptr<Reactors::BaseReactor>& trigger,
        RefType<Reactors::ReactorMonitor> reactorMonitor)
      : m_taskFactory(taskFactory),
        m_trigger(trigger),
        m_reactorMonitor(reactorMonitor.Get()),
        m_syncTrigger(*m_reactorMonitor) {}

  inline void SpawnTask::OnExecute() {
    return S0();
  }

  inline void SpawnTask::OnCancel() {
    m_syncTrigger.Do(
      [=] {
        if(m_state == 1) {
          return S2(StateEntry(State::CANCELED));
        }
      });
  }

  inline void SpawnTask::OnTaskUpdate(const StateEntry& entry) {
    if(!IsTerminal(entry.m_state)) {
      return;
    }
    --m_taskCount;
    if(m_state == 1) {
      return S5();
    }
  }

  inline void SpawnTask::OnTrigger(const Expect<void>& value) {
    if(m_state != 1) {
      return;
    }
    try {
      value.Get();
    } catch(const std::exception& e) {
      return S3(StateEntry(State::FAILED, e.what()));
    }
    return S4();
  }

  inline void SpawnTask::OnTriggerComplete() {
    m_triggerIsComplete = true;
    if(m_state == 1) {
      return S5();
    }
  }

  inline void SpawnTask::S0() {
    m_state = 0;
    m_triggerIsComplete = false;
    m_taskCount = 0;
    SetActive();
    S1();
    auto trigger = Reactors::Do(m_callbacks.GetCallback(
      std::bind(&SpawnTask::OnTrigger, this, std::placeholders::_1)),
      m_trigger);
    m_completeConnection = m_reactorMonitor->ConnectCompleteSignal(*trigger,
      std::bind(&SpawnTask::OnTriggerComplete, this));
    m_reactorMonitor->AddReactor(trigger);
  }

  inline void SpawnTask::S1() {
    m_state = 1;
  }

  inline void SpawnTask::S2(const StateEntry& state) {
    m_state = 2;
    SetTerminal(state);
  }

  inline void SpawnTask::S3(const StateEntry& state) {
    m_state = 3;
    SetTerminal(state);
  }

  inline void SpawnTask::S4() {
    m_state = 4;
    auto task = m_taskFactory->Create();
    Manage(task);
    ++m_taskCount;
    auto publisher = Reactors::MakePublisherReactor(&task->GetPublisher());
    auto taskReactor = Reactors::Do(m_callbacks.GetCallback(
      std::bind(&SpawnTask::OnTaskUpdate, this, std::placeholders::_1)),
      publisher);
    m_reactorMonitor->AddEvent(publisher);
    m_reactorMonitor->AddReactor(taskReactor);
    task->Execute();
    return S1();
  }

  inline void SpawnTask::S5() {
    m_state = 5;
    if(m_taskCount == 0 && m_triggerIsComplete) {

      // C0
      return S2(StateEntry(State::COMPLETE));
    } else {

      //! ~C0
      return S1();
    }
  }

  inline SpawnTaskFactory::SpawnTaskFactory(const TaskFactory& taskFactory,
        const std::shared_ptr<Reactors::BaseReactor>& trigger,
        RefType<Reactors::ReactorMonitor> reactorMonitor)
      : m_taskFactory(taskFactory),
        m_trigger(trigger),
        m_reactorMonitor(reactorMonitor.Get()) {}

  inline std::shared_ptr<Task> SpawnTaskFactory::Create() {
    return std::make_shared<SpawnTask>(m_taskFactory, m_trigger,
      Ref(*m_reactorMonitor));
  }
}
}

#endif
