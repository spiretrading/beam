#include "Beam/TasksTests/UntilTaskTester.hpp"
#include "Beam/Queues/StatePublisher.hpp"
#include "Beam/Reactors/PublisherReactor.hpp"
#include "Beam/Reactors/ReactorMonitor.hpp"
#include "Beam/Tasks/UntilTask.hpp"
#include "Beam/TasksTests/TestTaskEntry.hpp"

using namespace Beam;
using namespace Beam::Tasks;
using namespace Beam::Tasks::Tests;
using namespace Beam::Reactors;
using namespace std;

namespace {
  struct UntilTaskEntry {
    ReactorMonitor m_monitor;
    MockTaskEntry m_mockTaskEntry;
    StatePublisher<bool> m_conditionPublisher;
    std::shared_ptr<Reactor<bool>> m_condition;
    TestTaskEntry<UntilTaskFactory> m_taskEntry;

    UntilTaskEntry()
        : m_condition(MakePublisherReactor(&m_conditionPublisher)),
          m_taskEntry(m_mockTaskEntry.m_factory, m_condition, Ref(m_monitor)) {
      m_monitor.AddEvent(dynamic_pointer_cast<Event>(m_condition));
      m_monitor.Open();
    }
  };
}

void UntilTaskTester::TestExecuteThenTrigger() {
  UntilTaskEntry entry;
  entry.m_conditionPublisher.Push(false);
  Execute(entry.m_taskEntry);
  ExpectState(entry.m_taskEntry.m_taskQueue.m_queue, Task::State::ACTIVE);
  auto executedTask = ExpectTask(entry.m_mockTaskEntry);
  executedTask.m_task->Execute();
  ExpectState(executedTask.m_queue, Task::State::INITIALIZING);
  executedTask.m_task->SetActive();
  ExpectState(executedTask.m_queue, Task::State::ACTIVE);
  entry.m_conditionPublisher.Push(true);
  ExpectCancel(executedTask);
  ExpectState(entry.m_taskEntry.m_taskQueue.m_queue, Task::State::COMPLETE);
}

void UntilTaskTester::TestTriggerThenExecute() {
  UntilTaskEntry entry;
  entry.m_conditionPublisher.Push(true);
  Execute(entry.m_taskEntry);
  ExpectState(entry.m_taskEntry.m_taskQueue.m_queue, Task::State::ACTIVE);
  ExpectState(entry.m_taskEntry.m_taskQueue.m_queue, Task::State::COMPLETE);
}
