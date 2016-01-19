#include "Beam/TasksTests/SpawnTaskTester.hpp"
#include "Beam/Queues/StatePublisher.hpp"
#include "Beam/Reactors/PublisherReactor.hpp"
#include "Beam/Reactors/TriggeredReactor.hpp"
#include "Beam/Tasks/SpawnTask.hpp"
#include "Beam/TasksTests/TestTaskEntry.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Tasks;
using namespace Beam::Tasks::Tests;
using namespace std;

namespace {
  struct SpawnTaskEntry {
    ReactorMonitor m_monitor;
    MockTaskEntry m_mockTaskEntry;
    StatePublisher<int> m_triggerPublisher;
    std::shared_ptr<Reactor<int>> m_trigger;
    TestTaskEntry<SpawnTaskFactory> m_taskEntry;

    SpawnTaskEntry()
        : m_trigger(MakePublisherReactor(&m_triggerPublisher)),
          m_taskEntry(m_mockTaskEntry.m_factory, m_trigger, Ref(m_monitor)) {
      m_monitor.AddEvent(dynamic_pointer_cast<Event>(m_trigger));
      m_monitor.Open();
    }
  };
}

void SpawnTaskTester::TestTrigger() {
  SpawnTaskEntry entry;
  Execute(entry.m_taskEntry);
  entry.m_triggerPublisher.Push(1);
  ExpectState(entry.m_taskEntry.m_taskQueue.m_queue, Task::State::ACTIVE);
  auto executedTask = ExpectTask(entry.m_mockTaskEntry);
  executedTask.m_task->Execute();
  ExpectState(executedTask.m_queue, Task::State::INITIALIZING);
  executedTask.m_task->SetActive();
  ExpectState(executedTask.m_queue, Task::State::ACTIVE);
  executedTask.m_task->SetTerminal(Task::State::COMPLETE);
  ExpectState(executedTask.m_queue, Task::State::COMPLETE);
  entry.m_taskEntry.m_taskQueue.m_task->Cancel();
  ExpectState(entry.m_taskEntry.m_taskQueue.m_queue,
    Task::State::PENDING_CANCEL);
  ExpectState(entry.m_taskEntry.m_taskQueue.m_queue, Task::State::CANCELED);
}
