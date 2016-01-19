#include "Beam/ReactorsTests/TriggeredReactorTester.hpp"
#include "Beam/Reactors/TriggeredReactor.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;
using namespace std;

void TriggeredReactorTester::TestUpdate() {
  auto reactor = MakeTriggeredReactor<int>();
  CPPUNIT_ASSERT(reactor->IsInitializing());
  reactor->SetValue(5);
  reactor->Trigger();
  reactor->Execute();
  reactor->Commit();
  CPPUNIT_ASSERT(reactor->IsInitialized());
  CPPUNIT_ASSERT(reactor->Eval());
}
