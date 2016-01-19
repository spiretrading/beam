#ifndef BEAM_REACTORMONITORTASK_HPP
#define BEAM_REACTORMONITORTASK_HPP
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Reactors/Control.hpp"
#include "Beam/Reactors/PublisherReactor.hpp"
#include "Beam/Reactors/ReactorMonitor.hpp"
#include "Beam/Reactors/Trigger.hpp"
#include "Beam/SignalHandling/ScopedSlotAdaptor.hpp"
#include "Beam/Tasks/BasicTask.hpp"
#include "Beam/Tasks/Tasks.hpp"

namespace Beam {
namespace Tasks {

  /*! \class ReactorMonitorTask
      \brief A Task that starts a ReactorMonitor upon execution.
   */
  class ReactorMonitorTask : public BasicTask {
    public:

      //! Constructs a ReactorMonitorTask.
      /*!
        \param taskFactory Specifies the Task to execute.
        \param reactorMonitor The ReactorMonitor to start upon execution.
      */
      ReactorMonitorTask(const TaskFactory& taskFactory,
        RefType<Reactors::ReactorMonitor> reactorMonitor);

    protected:
      virtual void OnExecute();

      virtual void OnCancel();

    private:
      friend class ReactorMonitorTaskFactory;
      TaskFactory m_taskFactory;
      std::shared_ptr<Task> m_task;
      Reactors::ReactorMonitor* m_reactorMonitor;
      Reactors::Trigger m_trigger;
      int m_state;
      SignalHandling::ScopedSlotAdaptor m_callbacks;

      void OnTaskUpdate(const StateEntry& entry);
      void S0();
      void S1(const std::string& message);
      void S2();
      void S3(State state, const std::string& message);
  };

  /*! \class ReactorMonitorTaskFactory
      \brief Implements the TaskFactory for a ReactorMonitorTask.
   */
  class ReactorMonitorTaskFactory :
      public BasicTaskFactory<ReactorMonitorTaskFactory> {
    public:

      //! Constructs an ReactorMonitorTaskFactory.
      /*!
        \param taskFactory Specifies the Task to execute.
        \param reactorMonitor The ReactorMonitor to open upon execution.
      */
      ReactorMonitorTaskFactory(const TaskFactory& taskFactory,
        RefType<Reactors::ReactorMonitor> reactorMonitor);

      virtual std::shared_ptr<Task> Create();

      virtual void PrepareContinuation(const Task& task);

    private:
      TaskFactory m_taskFactory;
      Reactors::ReactorMonitor* m_reactorMonitor;
  };

  inline ReactorMonitorTask::ReactorMonitorTask(const TaskFactory& taskFactory,
      RefType<Reactors::ReactorMonitor> reactorMonitor)
      : m_taskFactory(taskFactory),
        m_reactorMonitor(reactorMonitor.Get()),
        m_trigger(*m_reactorMonitor) {}

  inline void ReactorMonitorTask::OnExecute() {
    return S0();
  }

  inline void ReactorMonitorTask::OnCancel() {
    m_trigger.Do(
      [=] {
        if(m_state != 1 && m_state != 3) {
          return S3(State::CANCELED, "");
        }
      });
  }

  inline void ReactorMonitorTask::OnTaskUpdate(const StateEntry& update) {
    if(m_state == 0) {
      if(update.m_state == State::ACTIVE) {
        return S2();
      } else if(update.m_state == State::FAILED) {
        return S1(update.m_message);
      }
    } else if(m_state == 2) {
      if(IsTerminal(update.m_state)) {
        return S3(update.m_state, update.m_message);
      }
    }
  }

  inline void ReactorMonitorTask::S0() {
    m_state = 0;
    SetActive();
    m_task = m_taskFactory->Create();
    Manage(m_task);
    auto publisher = Reactors::MakePublisherReactor(&m_task->GetPublisher());
    auto taskReactor = Reactors::Do(m_callbacks.GetCallback(
      std::bind(&ReactorMonitorTask::OnTaskUpdate, this,
      std::placeholders::_1)), publisher);
    m_reactorMonitor->AddEvent(publisher);
    m_reactorMonitor->AddReactor(taskReactor);
    m_task->Execute();
    m_reactorMonitor->Open();
  }

  inline void ReactorMonitorTask::S1(const std::string& message) {
    m_state = 1;
    m_reactorMonitor->Close();
    SetTerminal(State::FAILED, message);
  }

  inline void ReactorMonitorTask::S2() {
    m_state = 2;
  }

  inline void ReactorMonitorTask::S3(State state, const std::string& message) {
    m_state = 3;
    SetTerminal(state, message);
  }

  inline ReactorMonitorTaskFactory::ReactorMonitorTaskFactory(
      const TaskFactory& taskFactory,
      RefType<Reactors::ReactorMonitor> reactorMonitor)
      : m_taskFactory(taskFactory),
        m_reactorMonitor(reactorMonitor.Get()) {}

  inline std::shared_ptr<Task> ReactorMonitorTaskFactory::Create() {
    return std::make_shared<ReactorMonitorTask>(m_taskFactory,
      Ref(*m_reactorMonitor));
  }

  inline void ReactorMonitorTaskFactory::PrepareContinuation(const Task& task) {
    auto& reactorMonitorTask = static_cast<const ReactorMonitorTask&>(task);
    if(reactorMonitorTask.m_task != nullptr) {
      m_taskFactory->PrepareContinuation(*reactorMonitorTask.m_task);
    }
  }
}
}

#endif
