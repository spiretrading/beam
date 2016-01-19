#ifndef BEAM_TESTTASKENTRY_HPP
#define BEAM_TESTTASKENTRY_HPP
#include <memory>
#include <utility>
#include "Beam/Queues/Queue.hpp"
#include "Beam/Tasks/Task.hpp"
#include "Beam/TasksTests/MockTask.hpp"
#include "Beam/TasksTests/TasksTests.hpp"

namespace Beam {
namespace Tasks {
namespace Tests {

  /*! \struct TaskQueueEntry
      \brief Stores a single Task and a Queue monitoring its States.
   */
  struct TaskQueueEntry {

    //! The Task to store.
    std::shared_ptr<Task> m_task;

    //! The Queue monitoring the Task's States.
    std::shared_ptr<Queue<Task::StateEntry>> m_queue;
  };

  /*! \struct TestTaskEntry
      \brief Stores a TaskFactory and a TaskQueueEntry representing the Task
             executed by the TaskFactory.
      \tparam TaskFactory The type of TaskFactory represented.
   */
  template<typename TaskFactory>
  struct TestTaskEntry {

    //! The TaskFactory to store.
    TaskFactory m_factory;

    //! The TaskQueueEntry representing the Task created by the TaskFactory.
    TaskQueueEntry m_taskQueue;

    //! Constructs a TestTaskEntry.
    /*!
      \param args The parameters forwarded into the TaskFactory's constructor.
    */
    template<typename... Args>
    TestTaskEntry(Args&&... args);
  };

  /*! \struct MockTaskEntry
      \brief Stores a MockTaskFactory and a Queue monitoring the MockTasks
             executed by the MockTaskFactory.
   */
  struct MockTaskEntry {

    //! The MockTaskFactory represented.
    MockTaskFactory m_factory;

    //! The Queue monitoring the MockTasks executed by the MockTaskFactory.
    std::shared_ptr<Queue<std::weak_ptr<MockTask>>> m_taskQueue;

    //! Constructs a MockTaskEntry.
    MockTaskEntry();
  };

  /*! \struct MockTaskQueueEntry
      \brief Stores a MockTask and a Queue monitoring the MockTask's States.
   */
  struct MockTaskQueueEntry {

    //! The MockTask to store.
    std::shared_ptr<MockTask> m_task;

    //! The Queue monitoring the MockTask's States.
    std::shared_ptr<Queue<Task::StateEntry>> m_queue;
  };

  //! Tests that a Task has transitioned to a specified State.
  /*!
    \param stateQueue The Queue monitoring the Task's State transitions.
    \param state The State to expect.
  */
  inline void ExpectState(
      const std::shared_ptr<Queue<Task::StateEntry>>& stateQueue,
      Task::State state) {
    CPPUNIT_ASSERT(stateQueue->Top().m_state == state);
    stateQueue->Pop();
  }

  //! Executes the Task defined by a TestTaskEntry.
  /*!
    \param entry The TestTaskEntry defining the Task to execute.
  */
  template<typename TaskFactory>
  void Execute(TestTaskEntry<TaskFactory>& entry) {
    entry.m_taskQueue.m_task = entry.m_factory.Create();
    entry.m_taskQueue.m_queue = std::make_shared<Queue<Task::StateEntry>>();
    entry.m_taskQueue.m_task->GetPublisher().Monitor(entry.m_taskQueue.m_queue);
    entry.m_taskQueue.m_task->Execute();
    ExpectState(entry.m_taskQueue.m_queue, Task::State::INITIALIZING);
  }

  //! Tests that a MockTask is executed.
  /*!
    \param entry The MockTaskEntry defining the MockTask that is expected.
  */
  inline MockTaskQueueEntry ExpectTask(MockTaskEntry& entry) {
    MockTaskQueueEntry taskQueueEntry;
    taskQueueEntry.m_task = entry.m_taskQueue->Top().lock();
    entry.m_taskQueue->Pop();
    taskQueueEntry.m_queue = std::make_shared<Queue<Task::StateEntry>>();
    taskQueueEntry.m_task->GetPublisher().Monitor(taskQueueEntry.m_queue);
    return taskQueueEntry;
  }

  //! Tests that a MockTask received a cancel request.
  /*!
    \param entry The MockTaskQueueEntry monitoring the MockTask that is
           expecting the cancel request.
  */
  inline void ExpectCancel(MockTaskQueueEntry& entry) {
    ExpectState(entry.m_queue, Task::State::PENDING_CANCEL);
    entry.m_task->SetTerminal(Task::State::CANCELED);
    ExpectState(entry.m_queue, Task::State::CANCELED);
  }

  template<typename TaskFactory>
  template<typename... Args>
  TestTaskEntry<TaskFactory>::TestTaskEntry(Args&&... args)
      : m_factory(std::forward<Args>(args)...) {}

  inline MockTaskEntry::MockTaskEntry()
      : m_taskQueue(std::make_shared<Queue<std::weak_ptr<MockTask>>>()) {
        m_factory.GetPublisher().Monitor(m_taskQueue);
  }
}
}
}

#endif
