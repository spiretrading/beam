#include "Beam/ReactorsTests/WeakReactorTester.hpp"
#include "Beam/Reactors/TriggeredReactor.hpp"
#include "Beam/Reactors/WeakReactor.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;
using namespace std;

void WeakReactorTester::TestUpdate() {
  auto trigger = MakeTriggeredReactor<int>();
  auto reactor = MakeWeakReactor(trigger);
  CPPUNIT_ASSERT(reactor->IsInitializing());
  trigger->SetValue(123);
  trigger->Trigger();
  trigger->Execute();
  reactor->Commit();
  CPPUNIT_ASSERT_EQUAL(123, reactor->Eval());
}
