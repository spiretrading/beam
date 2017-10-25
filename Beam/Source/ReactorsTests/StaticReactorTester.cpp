#include "Beam/ReactorsTests/StaticReactorTester.hpp"
#include "Beam/Reactors/BasicReactor.hpp"
#include "Beam/Reactors/StaticReactor.hpp"
#include "Beam/Reactors/Trigger.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;
using namespace std;

void StaticReactorTester::TestSingleValue() {
  Trigger trigger;
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  auto source = MakeBasicReactor<int>(Ref(trigger));
  auto reactor = MakeStaticReactor(source);
  AssertException<ReactorUnavailableException>(*reactor, 0,
    BaseReactor::Update::NONE, false);
  source->Update(123);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 1);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 1, BaseReactor::Update::EVAL, 123, true);
}
