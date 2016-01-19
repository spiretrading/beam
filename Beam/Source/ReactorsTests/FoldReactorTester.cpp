#include "Beam/ReactorsTests/FoldReactorTester.hpp"
#include "Beam/Reactors/FoldReactor.hpp"
#include "Beam/Reactors/FunctionReactor.hpp"
#include "Beam/Reactors/TriggeredReactor.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;
using namespace std;

void FoldReactorTester::TestSum() {
  auto producer = MakeTriggeredReactor<int>();
  auto leftTrigger = MakeTriggeredReactor<int>();
  auto rightTrigger = MakeTriggeredReactor<int>();
  auto sum = MakeFunctionReactor(
    [] (int lhs, int rhs) {
      return lhs + rhs;
    }, leftTrigger, rightTrigger);
  auto fold = MakeFoldReactor(producer, sum, leftTrigger, rightTrigger);
  producer->SetValue(100);
  producer->Trigger();
  producer->Execute();
  fold->Commit();
  CPPUNIT_ASSERT(fold->IsInitializing());
  producer->SetValue(200);
  producer->Trigger();
  producer->Execute();
  fold->Commit();
  CPPUNIT_ASSERT(fold->GetSequenceNumber() == 1);
  CPPUNIT_ASSERT(fold->Eval() == 300);
  producer->SetValue(300);
  producer->Trigger();
  producer->Execute();
  fold->Commit();
  CPPUNIT_ASSERT(fold->GetSequenceNumber() == 2);
  CPPUNIT_ASSERT(fold->Eval() == 600);
}
