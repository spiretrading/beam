#include "Beam/TasksTests/ReactorTaskTester.hpp"
#include "Beam/Reactors/TriggeredReactor.hpp"
#include "Beam/Tasks/ReactorTask.hpp"
#include "Beam/TasksTests/TestTaskEntry.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Tasks;
using namespace Beam::Tasks::Tests;
using namespace std;

namespace {
  struct ReactorTaskEntry {
    ReactorMonitor m_monitor;
    MockTaskEntry m_mockTaskEntry;
    TestTaskEntry<ReactorTaskFactory> m_taskEntry;

    ReactorTaskEntry(const std::vector<ReactorProperty>& properties)
        : m_taskEntry(m_mockTaskEntry.m_factory, properties, Ref(m_monitor)) {
      m_monitor.Open();
    }
  };
}

void ReactorTaskTester::TestOneProperty() {
  auto xTrigger = MakeTriggeredReactor<int>();
  auto xProperty = MakeReactorProperty("x",
    static_pointer_cast<Reactor<int>>(xTrigger));
  vector<ReactorProperty> properties;
  properties.push_back(xProperty);
  ReactorTaskEntry entry(properties);
  entry.m_monitor.AddEvent(xTrigger);
  entry.m_mockTaskEntry.m_factory.DefineProperty<int>("x");
  Execute(entry.m_taskEntry);
  xTrigger->SetValue(123);
  xTrigger->Trigger();
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
