#ifndef BEAM_CHAINED_TASK_HPP
#define BEAM_CHAINED_TASK_HPP
#include <vector>
#include "Beam/Queues/RoutineTaskQueue.hpp"
#include "Beam/Tasks/BasicTask.hpp"

namespace Beam {
namespace Tasks {

  /*! \class ChainedTask
      \brief Executes a list of Tasks one after another.
   */
  class ChainedTask : public BasicTask {
      struct Guard {};
    public:

      //! Constructs a ChainedTask.
      /*!
        \param taskFactories Specifies the Tasks to execute.
      */
      ChainedTask(std::vector<TaskFactory> taskFactories);

      ChainedTask(std::vector<TaskFactory> taskFactories, int currentIndex,
        Guard);

    protected:
      virtual void OnExecute() override final;

      virtual void OnCancel() override final;

    private:
      friend class ChainedTaskFactory;
      std::vector<TaskFactory> m_taskFactories;
      std::shared_ptr<Task> m_currentTask;
      int m_currentIndex;
      bool m_canceled;
      RoutineTaskQueue m_tasks;

      void StartNextTask();
      void OnUpdate(const StateEntry& update);
  };

  /*! \class ChainedTaskFactory
      \brief Implements the TaskFactory for ChainedTasks.
   */
  class ChainedTaskFactory : public BasicTaskFactory<ChainedTaskFactory> {
    public:

      //! Constructs a ChainedTaskFactory.
      /*!
        \param taskFactories Specifies the Tasks to execute.
      */
      ChainedTaskFactory(std::vector<TaskFactory> taskFactories);

      virtual std::shared_ptr<Task> Create() override final;

      virtual void PrepareContinuation(const Task& task) override final;

    private:
      std::vector<TaskFactory> m_taskFactories;
      int m_currentIndex;
  };

  inline ChainedTask::ChainedTask(std::vector<TaskFactory> taskFactories)
      : ChainedTask{std::move(taskFactories), 0, Guard{}} {}

  inline ChainedTask::ChainedTask(std::vector<TaskFactory> taskFactories,
      int currentIndex, Guard)
      : m_taskFactories(std::move(taskFactories)),
        m_currentIndex{currentIndex},
        m_canceled{false} {}

  inline void ChainedTask::OnExecute() {
    SetActive();
    StartNextTask();
  }

  inline void ChainedTask::OnCancel() {
    m_tasks.Push(
      [=] {
        m_canceled = true;
        SetTerminal(State::CANCELED);
      });
  }

  inline void ChainedTask::StartNextTask() {
    if(m_currentIndex == static_cast<int>(m_taskFactories.size())) {
      SetTerminal();
      return;
    }
    m_currentTask = m_taskFactories[m_currentIndex]->Create();
    Manage(m_currentTask);
    m_currentTask->GetPublisher().Monitor(m_tasks.GetSlot<StateEntry>(
      std::bind(&ChainedTask::OnUpdate, this, std::placeholders::_1)));
    m_currentTask->Execute();
  }

  inline void ChainedTask::OnUpdate(const StateEntry& update) {
    if(m_canceled) {
      return;
    }
    if(update.m_state == State::FAILED) {
      SetTerminal(update);
    } else if(IsTerminal(update.m_state)) {
      ++m_currentIndex;
      StartNextTask();
    }
  }

  inline ChainedTaskFactory::ChainedTaskFactory(
      std::vector<TaskFactory> taskFactories)
      : m_taskFactories(std::move(taskFactories)),
        m_currentIndex{0} {}

  inline std::shared_ptr<Task> ChainedTaskFactory::Create() {
    auto currentIndex = m_currentIndex;
    m_currentIndex = 0;
    return std::make_shared<ChainedTask>(m_taskFactories, currentIndex,
      ChainedTask::Guard{});
  }

  inline void ChainedTaskFactory::PrepareContinuation(const Task& task) {
    auto& chainedTask = static_cast<const ChainedTask&>(task);
    m_currentIndex = chainedTask.m_currentIndex;
    if(chainedTask.m_currentTask != nullptr) {
      m_taskFactories[m_currentIndex]->PrepareContinuation(
        *chainedTask.m_currentTask);
    }
  }
}
}

#endif
