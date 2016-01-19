#include "Beam/TasksTests/IdleTaskTester.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Tasks/IdleTask.hpp"

using namespace Beam;
using namespace Beam::Tasks;
using namespace Beam::Tasks::Tests;
using namespace std;

void IdleTaskTester::TestExecuteAndCancel() {
  IdleTaskFactory taskFactory;
  std::shared_ptr<Queue<Task::StateEntry>> taskStateQueue =
    std::make_shared<Queue<Task::StateEntry>>();
  std::shared_ptr<Task> task = taskFactory.Create();
  task->GetPublisher().Monitor(taskStateQueue);
  task->Execute();
  Task::StateEntry initialState = taskStateQueue->Top();
  CPPUNIT_ASSERT(initialState.m_state == Task::State::INITIALIZING);
  taskStateQueue->Pop();
  Task::StateEntry activeState = taskStateQueue->Top();
  CPPUNIT_ASSERT(activeState.m_state == Task::State::ACTIVE);
  taskStateQueue->Pop();
  task->Cancel();
  Task::StateEntry pendingCancelState = taskStateQueue->Top();
  CPPUNIT_ASSERT(pendingCancelState.m_state == Task::State::PENDING_CANCEL);
  taskStateQueue->Pop();
  Task::StateEntry canceledState = taskStateQueue->Top();
  CPPUNIT_ASSERT(canceledState.m_state == Task::State::CANCELED);
  taskStateQueue->Pop();
}
