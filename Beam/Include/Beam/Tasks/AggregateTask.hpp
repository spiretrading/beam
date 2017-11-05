#ifndef BEAM_AGGREGATE_TASK_HPP
#define BEAM_AGGREGATE_TASK_HPP
#include <vector>
#include <boost/thread/locks.hpp>
#include "Beam/Queues/RoutineTaskQueue.hpp"
#include "Beam/Tasks/BasicTask.hpp"
#include "Beam/Threading/Mutex.hpp"

namespace Beam {
namespace Tasks {

  /*! \class AggregateTask
      \brief Executes a group of Tasks.
   */
  class AggregateTask : public BasicTask {
    public:

      //! Constructs an AggregateTask.
      /*!
        \param taskFactories Specifies the Tasks to aggregate.
      */
      AggregateTask(std::vector<TaskFactory> taskFactories);

    protected:
      virtual void OnExecute() override final;

      virtual void OnCancel() override final;

    private:
      friend class AggregateTaskFactory;
      Threading::Mutex m_mutex;
      std::vector<TaskFactory> m_taskFactories;
      std::vector<std::shared_ptr<Task>> m_tasks;
      int m_activeTasks;
      int m_state;
      RoutineTaskQueue m_taskQueue;

      void OnTaskUpdate(const StateEntry& update);
      bool C0();
      void S0();
      void S1();
      void S2(const std::string& message);
      void S3();
      void S4();
      void S5(State state, const std::string& message);
  };

  /*! \class AggregateTaskFactory
      \brief Implements the TaskFactory for AggregateTasks.
   */
  class AggregateTaskFactory : public BasicTaskFactory<AggregateTaskFactory> {
    public:

      //! Constructs an AggregateTaskFactory.
      /*!
        \param taskFactories Specifies the Tasks to aggregate.
      */
      AggregateTaskFactory(std::vector<TaskFactory> taskFactories);

      virtual std::shared_ptr<Task> Create() override final;

      virtual void PrepareContinuation(const Task& task) override final;

    private:
      std::vector<TaskFactory> m_taskFactories;
  };

  inline AggregateTask::AggregateTask(std::vector<TaskFactory> taskFactories)
      : m_taskFactories(std::move(taskFactories)) {}

  inline void AggregateTask::OnExecute() {
    boost::lock_guard<Threading::Mutex> lock{m_mutex};
    return S0();
  }

  inline void AggregateTask::OnCancel() {
    m_taskQueue.Push(
      [=] {
        if(m_state != 2 && m_state != 4 && m_state != 5) {
          return S5(Task::State::CANCELED, "");
        }
      });
  }

  inline void AggregateTask::OnTaskUpdate(const StateEntry& update) {
    boost::lock_guard<Threading::Mutex> lock{m_mutex};
    if(m_state == 1) {
      if(IsTerminal(update.m_state)) {
        return S3();
      }
    }
  }

  inline bool AggregateTask::C0() {
    return m_activeTasks == 0;
  }

  inline void AggregateTask::S0() {
    m_state = 0;
    SetActive();
    m_activeTasks = 0;
    for(auto& taskFactory : m_taskFactories) {
      auto task = taskFactory->Create();
      Manage(task);
      m_tasks.push_back(task);
      task->GetPublisher().Monitor(m_taskQueue.GetSlot<StateEntry>(
        std::bind(&AggregateTask::OnTaskUpdate, this, std::placeholders::_1)));
      ++m_activeTasks;
      task->Execute();
    }
    if(C0()) {
      return S4();
    } else {
      return S1();
    }
  }

  inline void AggregateTask::S1() {
    m_state = 1;
  }

  inline void AggregateTask::S2(const std::string& message) {
    m_state = 2;
    SetTerminal(State::FAILED, message);
  }

  inline void AggregateTask::S3() {
    m_state = 3;
    --m_activeTasks;
    if(C0()) {
      return S4();
    } else {
      return S1();
    }
  }

  inline void AggregateTask::S4() {
    m_state = 4;
    SetTerminal(State::COMPLETE);
  }

  inline void AggregateTask::S5(State state, const std::string& message) {
    m_state = 5;
    SetTerminal(state, message);
  }

  inline AggregateTaskFactory::AggregateTaskFactory(
      std::vector<TaskFactory> taskFactories)
      : m_taskFactories(std::move(taskFactories)) {}

  inline std::shared_ptr<Task> AggregateTaskFactory::Create() {
    return std::make_shared<AggregateTask>(m_taskFactories);
  }

  inline void AggregateTaskFactory::PrepareContinuation(const Task& task) {
    auto& aggregagteTask = static_cast<const AggregateTask&>(task);
    for(std::size_t i = 0; i < m_taskFactories.size(); ++i) {
      m_taskFactories[i]->PrepareContinuation(*aggregagteTask.m_tasks[i]);
    }
  }
}
}

#endif
