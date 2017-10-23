#ifndef BEAM_UNTILTASK_HPP
#define BEAM_UNTILTASK_HPP
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Reactors/DoReactor.hpp"
#include "Beam/Reactors/ReactorMonitor.hpp"
#include "Beam/Reactors/PublisherReactor.hpp"
#include "Beam/SignalHandling/ScopedSlotAdaptor.hpp"
#include "Beam/Tasks/BasicTask.hpp"
#include "Beam/Tasks/Tasks.hpp"

namespace Beam {
namespace Tasks {

  /*! \class UntilTask
      \brief Executes a Task until a condition is triggered.
   */
  class UntilTask : public BasicTask {
    public:

      //! Constructs an UntilTask.
      /*!
        \param taskFactory The Task to execute.
        \param condition The condition that cancels the Task.
        \param reactorMonitor The ReactorMonitor to use.
      */
      UntilTask(TaskFactory taskFactory,
        std::shared_ptr<Reactors::Reactor<bool>> condition,
        RefType<Reactors::ReactorMonitor> reactorMonitor);

    protected:
      virtual void OnExecute() override final;

      virtual void OnCancel() override final;

    private:
      TaskFactory m_taskFactory;
      std::shared_ptr<Reactors::Reactor<bool>> m_condition;
      Reactors::ReactorMonitor* m_reactorMonitor;
      int m_state;
      SignalHandling::ScopedSlotAdaptor m_callbacks;

      void OnCondition(const Expect<bool>& condition);
      void OnTaskUpdate(const StateEntry& update);
      void S0();
      void S1(const StateEntry& state);
      void S2();
      void S3(const StateEntry& state);
      void S4();
  };

  /*! \class UntilTaskFactory
      \brief Implements the TaskFactory for UntilTasks.
   */
  class UntilTaskFactory : public BasicTaskFactory<UntilTaskFactory> {
    public:

      //! Constructs an UntilTaskFactory.
      /*!
        \param taskFactory The Task to execute.
        \param condition The condition that executes the Task.
        \param reactorMonitor The ReactorMonitor to use.
      */
      UntilTaskFactory(TaskFactory taskFactory,
        std::shared_ptr<Reactors::Reactor<bool>> condition,
        RefType<Reactors::ReactorMonitor> reactorMonitor);

      virtual std::shared_ptr<Task> Create() override final;

    private:
      TaskFactory m_taskFactory;
      std::shared_ptr<Reactors::Reactor<bool>> m_condition;
      Reactors::ReactorMonitor* m_reactorMonitor;
  };

  inline UntilTask::UntilTask(TaskFactory taskFactory,
      std::shared_ptr<Reactors::Reactor<bool>> condition,
      RefType<Reactors::ReactorMonitor> reactorMonitor)
      : m_taskFactory{std::move(taskFactory)},
        m_condition{std::move(condition)},
        m_reactorMonitor{reactorMonitor.Get()} {}

  inline void UntilTask::OnExecute() {
    return S0();
  }

  inline void UntilTask::OnCancel() {
    m_reactorMonitor->Do(
      [=] {
        if(m_state == 0) {
          return S1(State::CANCELED);
        } else if(m_state == 4) {
          return S1(State::CANCELED);
        }
      });
  }

  inline void UntilTask::OnCondition(const Expect<bool>& condition) {
    if(m_state == 0) {
      try {
        if(condition.Get()) {
          return S1(State::COMPLETE);
        } else {
          return S2();
        }
      } catch(const std::exception& e) {
        return S3(StateEntry{State::FAILED, e.what()});
      }
    } else if(m_state == 4) {
      try {
        if(condition.Get()) {
          return S1(State::COMPLETE);
        }
      } catch(const std::exception& e) {
        return S3(StateEntry{State::FAILED, e.what()});
      }
    }
  }

  inline void UntilTask::OnTaskUpdate(const StateEntry& update) {
    if(m_state == 4) {
      if(update.m_state == State::FAILED) {
        return S3(update);
      } else if(IsTerminal(update.m_state)) {
        return S1(update);
      }
    }
  }

  inline void UntilTask::S0() {
    m_state = 0;
    SetActive();
    m_reactorMonitor->Add(Reactors::Do(m_callbacks.GetCallback(
      std::bind(&UntilTask::OnCondition, this, std::placeholders::_1)),
      m_condition));
  }

  inline void UntilTask::S1(const StateEntry& state) {
    m_state = 1;
    SetTerminal(state.m_state, state.m_message);
  }

  inline void UntilTask::S2() {
    m_state = 2;
    auto task = m_taskFactory->Create();
    Manage(task);
    auto publisher = Reactors::MakePublisherReactor(task->GetPublisher(),
      Ref(m_reactorMonitor->GetTrigger()));
    auto taskReactor = Reactors::Do(m_callbacks.GetCallback(
      std::bind(&UntilTask::OnTaskUpdate, this, std::placeholders::_1)),
      publisher);
    m_reactorMonitor->Add(taskReactor);
    task->Execute();
    return S4();
  }

  inline void UntilTask::S3(const StateEntry& state) {
    m_state = 3;
    SetTerminal(state.m_state, state.m_message);
  }

  inline void UntilTask::S4() {
    m_state = 4;
  }

  inline UntilTaskFactory::UntilTaskFactory(TaskFactory taskFactory,
      std::shared_ptr<Reactors::Reactor<bool>> condition,
      RefType<Reactors::ReactorMonitor> reactorMonitor)
      : m_taskFactory{std::move(taskFactory)},
        m_condition{std::move(condition)},
        m_reactorMonitor{reactorMonitor.Get()} {}

  inline std::shared_ptr<Task> UntilTaskFactory::Create() {
    return std::make_shared<UntilTask>(m_taskFactory, m_condition,
      Ref(*m_reactorMonitor));
  }
}
}

#endif
