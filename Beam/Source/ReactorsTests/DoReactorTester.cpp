#include "Beam/ReactorsTests/DoReactorTester.hpp"
#include "Beam/Reactors/ConstantReactor.hpp"
#include "Beam/Reactors/DoReactor.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;
using namespace std;

void DoReactorTester::TestPassThrough() {
  auto constant = MakeConstantReactor(123);
  int capture;
  auto doReactor = Do(
    [&] (const auto& value) {
      capture = value;
    }, constant);
  AssertValue(*doReactor, 0, BaseReactor::Update::COMPLETE_WITH_EVAL, 123);
  CPPUNIT_ASSERT_EQUAL(capture, 123);
}
