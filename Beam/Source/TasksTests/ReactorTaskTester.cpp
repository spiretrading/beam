#include "Beam/TasksTests/ReactorTaskTester.hpp"
#include "Beam/Reactors/BasicReactor.hpp"
#include "Beam/Tasks/ReactorTask.hpp"
#include "Beam/TasksTests/TestTaskEntry.hpp"

using namespace Beam;
using namespace Beam::Tasks;
using namespace Beam::Tasks::Tests;
using namespace Beam::Reactors;
using namespace std;

void ReactorTaskTester::TestUpdates() {
  ReactorMonitor monitor;
  auto propertyReactor = MakeBasicReactor<int>();
  auto property = MakeReactorProperty("xyz",
    std::static_pointer_cast<Reactor<int>>(propertyReactor));
  MockTaskEntry mockTaskEntry;
  mockTaskEntry.m_factory.DefineProperty<int>("xyz");
  TestTaskEntry<ReactorTaskFactory> taskEntry{Ref(monitor),
    std::vector<ReactorProperty>{property}, mockTaskEntry.m_factory};
  monitor.Add(propertyReactor);
  monitor.Open();
  Execute(taskEntry);
  propertyReactor->Update(123);
  ExpectState(taskEntry.m_taskQueue.m_queue, Task::State::ACTIVE);
  auto executedTaskA = ExpectTask(mockTaskEntry);
  ExpectState(executedTaskA.m_queue, Task::State::INITIALIZING);
  CPPUNIT_ASSERT(executedTaskA.m_task->GetFactory().Get<int>("xyz") == 123);
  executedTaskA.m_task->SetActive();
  ExpectState(executedTaskA.m_queue, Task::State::ACTIVE);
  propertyReactor->Update(321);
  ExpectState(executedTaskA.m_queue, Task::State::PENDING_CANCEL);
  executedTaskA.m_task->SetTerminal(Task::State::CANCELED);
  auto executedTaskB = ExpectTask(mockTaskEntry);
  ExpectState(executedTaskB.m_queue, Task::State::INITIALIZING);
  CPPUNIT_ASSERT(executedTaskB.m_task->GetFactory().Get<int>("xyz") == 321);
}
