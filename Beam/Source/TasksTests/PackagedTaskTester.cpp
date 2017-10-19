#include "Beam/TasksTests/PackagedTaskTester.hpp"
#include "Beam/Tasks/PackagedTask.hpp"

using namespace Beam;
using namespace Beam::Tasks;
using namespace Beam::Tasks::Tests;
using namespace std;

namespace {
  struct FunctionPackage {
    std::shared_ptr<int> m_result;

    FunctionPackage()
        : m_result{std::make_shared<int>()} {}

    void Execute(int a, int b) {
      *m_result = a + b;
    }

    void Cancel() {}
  };
}

void PackagedTaskTester::TestExecuteFunction() {
  FunctionPackage package;
  PackagedTaskFactory<FunctionPackage> factory{package, {"a", "b"}};
  factory.Set("a", 200);
  factory.Set("b", 367);
  auto task = factory.Create();
  task->Execute();
  Wait(*task);
  CPPUNIT_ASSERT(*package.m_result == 567);
}
