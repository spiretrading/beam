#include "Beam/ReactorsTests/ConstantReactorTester.hpp"
#include <string>
#include "Beam/Reactors/ConstantReactor.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;
using namespace std;

void ConstantReactorTester::TestInt() {
  auto constant = MakeConstantReactor(123);
  CPPUNIT_ASSERT(constant->IsInitialized());
  CPPUNIT_ASSERT(constant->IsComplete());
  CPPUNIT_ASSERT(constant->Commit(0) == BaseReactor::Update::NONE);
  CPPUNIT_ASSERT(constant->IsInitialized());
  CPPUNIT_ASSERT(constant->IsComplete());
  CPPUNIT_ASSERT_EQUAL(constant->Eval(), 123);
}

void ConstantReactorTester::TestDecimal() {
  auto constant = MakeConstantReactor(3.14);
  CPPUNIT_ASSERT(constant->IsInitialized());
  CPPUNIT_ASSERT(constant->IsComplete());
  CPPUNIT_ASSERT(constant->Commit(0) == BaseReactor::Update::NONE);
  CPPUNIT_ASSERT(constant->IsInitialized());
  CPPUNIT_ASSERT(constant->IsComplete());
  CPPUNIT_ASSERT_EQUAL(constant->Eval(), 3.14);
}

void ConstantReactorTester::TestString() {
  auto constant = MakeConstantReactor<string>("hello world");
  CPPUNIT_ASSERT(constant->IsInitialized());
  CPPUNIT_ASSERT(constant->IsComplete());
  CPPUNIT_ASSERT(constant->Commit(0) == BaseReactor::Update::NONE);
  CPPUNIT_ASSERT(constant->IsInitialized());
  CPPUNIT_ASSERT(constant->IsComplete());
  CPPUNIT_ASSERT_EQUAL(constant->Eval(), string("hello world"));
}
