#include "Beam/TasksTests/SpawnTaskTester.hpp"
#include "Beam/Reactors/BasicReactor.hpp"
#include "Beam/Tasks/SpawnTask.hpp"
#include "Beam/TasksTests/TestTaskEntry.hpp"

using namespace Beam;
using namespace Beam::Tasks;
using namespace Beam::Tasks::Tests;
using namespace Beam::Reactors;
using namespace std;

namespace {
  struct SpawnTaskEntry {
    ReactorMonitor m_monitor;
    MockTaskEntry m_mockTaskEntry;
    std::shared_ptr<BasicReactor<int>> m_trigger;
    TestTaskEntry<SpawnTaskFactory> m_taskEntry;

    SpawnTaskEntry()
        : m_trigger{MakeBasicReactor<int>(Ref(m_monitor.GetTrigger()))},
          m_taskEntry{m_mockTaskEntry.m_factory, m_trigger, Ref(m_monitor)} {
      m_monitor.Add(m_trigger);
      m_monitor.Open();
    }
  };
}

void SpawnTaskTester::TestImmediateCompletion() {
  SpawnTaskEntry entry;
  entry.m_trigger->SetComplete();
  Execute(entry.m_taskEntry);
  ExpectState(entry.m_taskEntry.m_taskQueue.m_queue, Task::State::ACTIVE);
  ExpectState(entry.m_taskEntry.m_taskQueue.m_queue, Task::State::COMPLETE);
}
