#include "Beam/ReactorsTests/ConstantReactorTester.hpp"
#include <string>
#include "Beam/Reactors/ConstantReactor.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;
using namespace std;

void ConstantReactorTester::TestInt() {
  auto constant = MakeConstantReactor(123);
  CPPUNIT_ASSERT(constant->Commit(0) ==
    BaseReactor::Update::COMPLETE_WITH_EVAL);
  CPPUNIT_ASSERT_EQUAL(constant->Eval(), 123);
  CPPUNIT_ASSERT(constant->Commit(1) == BaseReactor::Update::NONE);
}

void ConstantReactorTester::TestDecimal() {
  auto constant = MakeConstantReactor(3.14);
  CPPUNIT_ASSERT(constant->Commit(0) ==
    BaseReactor::Update::COMPLETE_WITH_EVAL);
  CPPUNIT_ASSERT_EQUAL(constant->Eval(), 3.14);
  CPPUNIT_ASSERT(constant->Commit(1) == BaseReactor::Update::NONE);
}

void ConstantReactorTester::TestString() {
  auto constant = MakeConstantReactor<string>("hello world");
  CPPUNIT_ASSERT(constant->Commit(0) ==
    BaseReactor::Update::COMPLETE_WITH_EVAL);
  CPPUNIT_ASSERT_EQUAL(constant->Eval(), string("hello world"));
  CPPUNIT_ASSERT(constant->Commit(1) == BaseReactor::Update::NONE);
}

void ConstantReactorTester::TestLift() {
  auto literal = Lift(5);
  CPPUNIT_ASSERT(literal->Commit(0) == BaseReactor::Update::COMPLETE_WITH_EVAL);
  CPPUNIT_ASSERT_EQUAL(literal->Eval(), 5);
  CPPUNIT_ASSERT(literal->Commit(1) == BaseReactor::Update::NONE);
  auto reactor = Lift(MakeConstantReactor(10));
  CPPUNIT_ASSERT(reactor->Commit(0) == BaseReactor::Update::COMPLETE_WITH_EVAL);
  CPPUNIT_ASSERT_EQUAL(reactor->Eval(), 10);
  CPPUNIT_ASSERT(reactor->Commit(1) == BaseReactor::Update::NONE);
}
