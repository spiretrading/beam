#include "Beam/ReactorsTests/NoneReactorTester.hpp"
#include "Beam/Reactors/NoneReactor.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;
using namespace std;

void NoneReactorTester::TestInt() {
  auto reactor = MakeNoneReactor<int>();
  CPPUNIT_ASSERT(reactor->Commit(0) == BaseReactor::Update::COMPLETE);
  CPPUNIT_ASSERT_THROW(reactor->Eval(), ReactorUnavailableException);
  CPPUNIT_ASSERT(reactor->Commit(1) == BaseReactor::Update::NONE);
  CPPUNIT_ASSERT_THROW(reactor->Eval(), ReactorUnavailableException);
}
