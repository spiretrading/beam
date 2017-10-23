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
  auto source = MakeBasicReactor<int>(Ref(trigger));
  auto reactor = MakeStaticReactor(source);
  source->Update(123);
  AssertValue(*reactor, 0, BaseReactor::Update::EVAL, 123, true);
}
