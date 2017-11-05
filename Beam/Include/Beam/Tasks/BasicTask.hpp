#ifndef BEAM_BASIC_TASK_HPP
#define BEAM_BASIC_TASK_HPP
#include <cassert>
#include <unordered_map>
#include "Beam/Queues/RoutineTaskQueue.hpp"
#include "Beam/Queues/SequencePublisher.hpp"
#include "Beam/Tasks/Task.hpp"
#include "Beam/Tasks/TaskPropertyNotFoundException.hpp"
#include "Beam/Tasks/Tasks.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"

namespace Beam {
namespace Tasks {

  /*! \class BasicTask
      \brief Provides common functionality used by many Tasks.
   */
  class BasicTask : public Task {
    public:
      virtual void Execute() override final;

      virtual void Cancel() override final;

      virtual const Publisher<StateEntry>& GetPublisher() const override final;

    protected:

      //! Constructs a BasicTask.
      BasicTask();

      //! Implements the execute action handler.
      virtual void OnExecute() = 0;

      //! Implements the cancel action handler.
      virtual void OnCancel() = 0;

      //! Sets the State of this Task to ACTIVE.
      void SetActive();

      //! Sets the State of this Task to ACTIVE.
      /*!
        \param message The message describing the change in State.
      */
      void SetActive(const std::string& message);

      //! Sets the State of this Task to COMPLETE.
      void SetTerminal();

      //! Sets the State of this Task to some terminal state.
      /*!
        \param state The State of the Task.
      */
      void SetTerminal(const StateEntry& state);

      //! Sets the State of this Task to some terminal state.
      /*!
        \param state The State of the Task.
        \param message The message describing the change in State.
      */
      void SetTerminal(State state, const std::string& message);

      //! Manages a sub-Task.
      /*!
        \param task The sub-Task to manage.
      */
      void Manage(std::shared_ptr<Task> task);

    private:
      struct TaskEntry {
        std::shared_ptr<Task> m_task;
        bool m_isTerminal;

        TaskEntry(std::shared_ptr<Task> task);
      };
      mutable Threading::RecursiveMutex m_mutex;
      State m_state;
      SequencePublisher<StateEntry> m_publisher;
      std::vector<TaskEntry> m_managedTasks;
      int m_terminalCount;
      StateEntry m_terminalStateEntry;
      RoutineTaskQueue m_taskQueue;

      void SetState(State state, const std::string& message);
      void OnTaskUpdate(std::size_t entryIndex, const StateEntry& update);
  };

  /*! \class BasicTaskFactory
      \brief Helper base class for implementing a BasicTaskFactory.
      \tparam The TaskFactory inhering this helper class.
   */
  template<typename T>
  class BasicTaskFactory : public VirtualTaskFactory,
      public CloneableMixin<T> {
    public:
      virtual boost::any& FindProperty(const std::string& name) override;

    protected:

      //! Constructs a BasicTaskFactory.
      BasicTaskFactory() = default;

      //! Copies a BasicTaskFactory.
      BasicTaskFactory(const BasicTaskFactory& factory) = default;

      //! Adds a property to this factory.
      /*!
        \param name The name of the property to add.
        \param value The property's default value.
      */
      template<typename U>
      void DefineProperty(const std::string& name, const U& value = U());

    private:
      std::unordered_map<std::string, boost::any> m_properties;
  };

  inline BasicTask::TaskEntry::TaskEntry(std::shared_ptr<Task> task)
      : m_task{std::move(task)},
        m_isTerminal{false} {}

  inline void BasicTask::Execute() {
    boost::lock_guard<Threading::RecursiveMutex> lock{m_mutex};
    if(m_state != Task::State::NONE) {
      return;
    }
    SetState(Task::State::INITIALIZING, "");
    OnExecute();
  }

  inline void BasicTask::Cancel() {
    boost::lock_guard<Threading::RecursiveMutex> lock{m_mutex};
    m_taskQueue.Push(
      [=] {
        if(m_state == Task::State::PENDING_CANCEL || IsTerminal(m_state)) {
          return;
        }
        if(m_state == Task::State::NONE) {
          SetState(Task::State::PENDING_CANCEL, "");
          SetTerminal(Task::State::CANCELED);
        } else {
          SetState(Task::State::PENDING_CANCEL, "");
          OnCancel();
        }
      });
  }

  inline const Publisher<Task::StateEntry>& BasicTask::GetPublisher() const {
    return m_publisher;
  }

  inline BasicTask::BasicTask()
      : m_state{State::NONE},
        m_terminalCount{0},
        m_terminalStateEntry{State::NONE} {}

  inline void BasicTask::SetActive() {
    SetActive("");
  }

  inline void BasicTask::SetActive(const std::string& message) {
    m_taskQueue.Push(
      [=] {
        if(m_state == State::PENDING_CANCEL) {
          SetTerminal(Task::State::CANCELED);
          return;
        }
        assert(m_state == State::INITIALIZING);
        SetState(State::ACTIVE, message);
      });
  }

  inline void BasicTask::SetTerminal() {
    SetTerminal(State::COMPLETE);
  }

  inline void BasicTask::SetTerminal(const StateEntry& state) {
    m_taskQueue.Push(
      [=] {
        if(IsTerminal(m_state) ||
            m_terminalStateEntry.m_state != Task::State::NONE) {
          return;
        }
        assert(IsTerminal(state.m_state));
        if(m_terminalCount == m_managedTasks.size()) {
          m_state = state.m_state;
          m_publisher.Push(state);
        } else {
          m_terminalStateEntry = state;
          for(auto& managedTask : m_managedTasks) {
            if(!managedTask.m_isTerminal) {
              managedTask.m_task->Cancel();
            }
          }
        }
      });
  }

  inline void BasicTask::SetTerminal(State state, const std::string& message) {
    SetTerminal(StateEntry{state, message});
  }

  inline void BasicTask::Manage(std::shared_ptr<Task> task) {
    m_taskQueue.Push(
      [=, task = std::move(task)] {
        TaskEntry entry{std::move(task)};
        entry.m_task->GetPublisher().Monitor(m_taskQueue.GetSlot<StateEntry>(
          std::bind(&BasicTask::OnTaskUpdate, this, m_managedTasks.size(),
          std::placeholders::_1)));
        m_managedTasks.push_back(std::move(entry));
      });
  }

  inline void BasicTask::SetState(State state, const std::string& message) {
    assert(!IsTerminal(m_state));
    assert(!IsTerminal(state));
    m_state = state;
    m_publisher.Push(StateEntry{m_state, message});
  }

  inline void BasicTask::OnTaskUpdate(std::size_t entryIndex,
      const StateEntry& update) {
    if(!IsTerminal(update.m_state)) {
      return;
    }
    m_managedTasks[entryIndex].m_isTerminal = true;
    ++m_terminalCount;
    if(m_terminalStateEntry.m_state != State::NONE &&
        m_terminalCount == m_managedTasks.size()) {
      m_state = m_terminalStateEntry.m_state;
      m_publisher.Push(m_terminalStateEntry);
    }
  }

  template<typename T>
  boost::any& BasicTaskFactory<T>::FindProperty(const std::string& name) {
    auto propertyIterator = m_properties.find(name);
    if(propertyIterator == m_properties.end()) {
      BOOST_THROW_EXCEPTION(TaskPropertyNotFoundException{name});
    }
    return propertyIterator->second;
  }

  template<typename T>
  template<typename U>
  void BasicTaskFactory<T>::DefineProperty(const std::string& name,
      const U& value) {
    m_properties.insert(std::make_pair(name, value));
  }
}
}

#endif
