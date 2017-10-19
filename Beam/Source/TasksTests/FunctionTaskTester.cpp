#include "Beam/TasksTests/FunctionTaskTester.hpp"
#include "Beam/Tasks/FunctionTask.hpp"

using namespace Beam;
using namespace Beam::Tasks;
using namespace Beam::Tasks::Tests;
using namespace std;

void FunctionTaskTester::TestFunctionZeroParameters() {
  int result = 0;
  auto factory = MakeFunctionTaskFactory(
    [&] {
      result = 123;
    }, {});
  auto task = factory->Create();
  task->Execute();
  Wait(*task);
  CPPUNIT_ASSERT(result == 123);
}

void FunctionTaskTester::TestFunctionOneParameter() {
  int result = 0;
  auto factory = MakeFunctionTaskFactory(
    [&] (int x) {
      result = x;
    }, {"x"});
  factory->Set("x", 543);
  auto task = factory->Create();
  task->Execute();
  Wait(*task);
  CPPUNIT_ASSERT(result == 543);
}
