#include "Beam/TasksTests/WhenTaskTester.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Reactors/QueueReactor.hpp"
#include "Beam/Reactors/ReactorMonitor.hpp"
#include "Beam/Tasks/WhenTask.hpp"
#include "Beam/TasksTests/TestTaskEntry.hpp"

using namespace Beam;
using namespace Beam::Tasks;
using namespace Beam::Tasks::Tests;
using namespace Beam::Reactors;
using namespace std;

namespace {
  struct WhenTaskEntry {
    ReactorMonitor m_monitor;
    MockTaskEntry m_mockTaskEntry;
    std::shared_ptr<Queue<bool>> m_conditionQueue;
    std::shared_ptr<Reactor<bool>> m_condition;
    TestTaskEntry<WhenTaskFactory> m_taskEntry;

    WhenTaskEntry()
        : m_conditionQueue{std::make_shared<Queue<bool>>()},
          m_condition{MakeQueueReactor(m_conditionQueue)},
          m_taskEntry{Ref(m_monitor), m_condition, m_mockTaskEntry.m_factory} {
      m_monitor.Add(m_condition);
      m_monitor.Open();
    }
  };
}

void WhenTaskTester::TestNeverTrigger() {
  WhenTaskEntry entry;
  Execute(entry.m_taskEntry);
  ExpectState(entry.m_taskEntry.m_taskQueue.m_queue, Task::State::ACTIVE);
  entry.m_conditionQueue->Break();
  ExpectState(entry.m_taskEntry.m_taskQueue.m_queue, Task::State::COMPLETE);
}

void WhenTaskTester::TestExecuteThenTrigger() {
  WhenTaskEntry entry;
  Execute(entry.m_taskEntry);
  entry.m_conditionQueue->Push(true);
  ExpectState(entry.m_taskEntry.m_taskQueue.m_queue, Task::State::ACTIVE);
  auto executedTask = ExpectTask(entry.m_mockTaskEntry);
  executedTask.m_task->Execute();
  ExpectState(executedTask.m_queue, Task::State::INITIALIZING);
  executedTask.m_task->SetActive();
  ExpectState(executedTask.m_queue, Task::State::ACTIVE);
  executedTask.m_task->SetTerminal(Task::State::COMPLETE);
  ExpectState(executedTask.m_queue, Task::State::COMPLETE);
  ExpectState(entry.m_taskEntry.m_taskQueue.m_queue, Task::State::COMPLETE);
}

void WhenTaskTester::TestTriggerThenExecute() {
  WhenTaskEntry entry;
  entry.m_conditionQueue->Push(true);
  Execute(entry.m_taskEntry);
  ExpectState(entry.m_taskEntry.m_taskQueue.m_queue, Task::State::ACTIVE);
  auto executedTask = ExpectTask(entry.m_mockTaskEntry);
  executedTask.m_task->Execute();
  ExpectState(executedTask.m_queue, Task::State::INITIALIZING);
  executedTask.m_task->SetActive();
  ExpectState(executedTask.m_queue, Task::State::ACTIVE);
  executedTask.m_task->SetTerminal(Task::State::COMPLETE);
  ExpectState(executedTask.m_queue, Task::State::COMPLETE);
  ExpectState(entry.m_taskEntry.m_taskQueue.m_queue, Task::State::COMPLETE);
}
