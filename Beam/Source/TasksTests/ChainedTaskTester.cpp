#include "Beam/TasksTests/ChainedTaskTester.hpp"
#include "Beam/Tasks/ChainedTask.hpp"
#include "Beam/TasksTests/TestTaskEntry.hpp"

using namespace Beam;
using namespace Beam::Tasks;
using namespace Beam::Tasks::Tests;
using namespace std;

void ChainedTaskTester::TestEmpty() {
  std::vector<TaskFactory> factories;
  TestTaskEntry<ChainedTaskFactory> taskEntry{factories};
  Execute(taskEntry);
  ExpectState(taskEntry.m_taskQueue.m_queue, Task::State::ACTIVE);
  ExpectState(taskEntry.m_taskQueue.m_queue, Task::State::COMPLETE);
}

void ChainedTaskTester::TestSingleTask() {
  MockTaskEntry mockTaskEntry;
  std::vector<TaskFactory> factories;
  factories.push_back(mockTaskEntry.m_factory);
  TestTaskEntry<ChainedTaskFactory> taskEntry{factories};
  Execute(taskEntry);
  ExpectState(taskEntry.m_taskQueue.m_queue, Task::State::ACTIVE);
  auto executedTask = ExpectTask(mockTaskEntry);
  executedTask.m_task->Execute();
  ExpectState(executedTask.m_queue, Task::State::INITIALIZING);
  executedTask.m_task->SetActive();
  ExpectState(executedTask.m_queue, Task::State::ACTIVE);
  executedTask.m_task->SetTerminal(Task::State::COMPLETE);
  ExpectState(executedTask.m_queue, Task::State::COMPLETE);
  ExpectState(taskEntry.m_taskQueue.m_queue, Task::State::COMPLETE);
}

void ChainedTaskTester::TestDoubleTask() {
  MockTaskEntry mockTaskEntryA;
  MockTaskEntry mockTaskEntryB;
  std::vector<TaskFactory> factories;
  factories.push_back(mockTaskEntryA.m_factory);
  factories.push_back(mockTaskEntryB.m_factory);
  TestTaskEntry<ChainedTaskFactory> taskEntry{factories};
  Execute(taskEntry);
  ExpectState(taskEntry.m_taskQueue.m_queue, Task::State::ACTIVE);
  auto executedTaskA = ExpectTask(mockTaskEntryA);
  executedTaskA.m_task->Execute();
  ExpectState(executedTaskA.m_queue, Task::State::INITIALIZING);
  executedTaskA.m_task->SetActive();
  ExpectState(executedTaskA.m_queue, Task::State::ACTIVE);
  executedTaskA.m_task->SetTerminal(Task::State::COMPLETE);
  ExpectState(executedTaskA.m_queue, Task::State::COMPLETE);
  auto executedTaskB = ExpectTask(mockTaskEntryB);
  executedTaskB.m_task->Execute();
  ExpectState(executedTaskB.m_queue, Task::State::INITIALIZING);
  executedTaskB.m_task->SetActive();
  ExpectState(executedTaskB.m_queue, Task::State::ACTIVE);
  executedTaskB.m_task->SetTerminal(Task::State::COMPLETE);
  ExpectState(executedTaskB.m_queue, Task::State::COMPLETE);
  ExpectState(taskEntry.m_taskQueue.m_queue, Task::State::COMPLETE);
}

void ChainedTaskTester::TestDoubleTaskWithFailure() {
  MockTaskEntry mockTaskEntryA;
  MockTaskEntry mockTaskEntryB;
  std::vector<TaskFactory> factories;
  factories.push_back(mockTaskEntryA.m_factory);
  factories.push_back(mockTaskEntryB.m_factory);
  TestTaskEntry<ChainedTaskFactory> taskEntry{factories};
  Execute(taskEntry);
  ExpectState(taskEntry.m_taskQueue.m_queue, Task::State::ACTIVE);
  auto executedTaskA = ExpectTask(mockTaskEntryA);
  executedTaskA.m_task->Execute();
  ExpectState(executedTaskA.m_queue, Task::State::INITIALIZING);
  executedTaskA.m_task->SetTerminal(Task::State::FAILED);
  ExpectState(executedTaskA.m_queue, Task::State::FAILED);
  ExpectState(taskEntry.m_taskQueue.m_queue, Task::State::FAILED);
}
