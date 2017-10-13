#include "Beam/ReactorsTests/NoneReactorTester.hpp"
#include "Beam/Reactors/NoneReactor.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;
using namespace std;

void NoneReactorTester::TestInt() {
  auto reactor = MakeNoneReactor<int>();
  reactor->Commit(0);
  CPPUNIT_ASSERT(reactor->IsComplete());
  CPPUNIT_ASSERT_THROW(reactor->Eval(), ReactorUnavailableException);
}
