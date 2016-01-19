#include "Beam/ReactorsTests/ConstantReactorTester.hpp"
#include <string>
#include "Beam/Reactors/ConstantReactor.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;
using namespace std;

void ConstantReactorTester::TestInt() {
  auto constant = MakeConstantReactor(123);
  constant->Commit();
  CPPUNIT_ASSERT(constant->IsComplete());
  CPPUNIT_ASSERT_EQUAL(constant->Eval(), 123);
  auto sequence = constant->GetSequenceNumber();
  constant->Commit();
  CPPUNIT_ASSERT_EQUAL(sequence, constant->GetSequenceNumber());
  CPPUNIT_ASSERT(!constant->IsInitializing());
  CPPUNIT_ASSERT(constant->IsComplete());
}

void ConstantReactorTester::TestDecimal() {
  auto constant = MakeConstantReactor(3.14);
  constant->Commit();
  CPPUNIT_ASSERT(constant->IsComplete());
  CPPUNIT_ASSERT_EQUAL(constant->Eval(), 3.14);
  auto sequence = constant->GetSequenceNumber();
  constant->Commit();
  CPPUNIT_ASSERT_EQUAL(sequence, constant->GetSequenceNumber());
  CPPUNIT_ASSERT(!constant->IsInitializing());
  CPPUNIT_ASSERT(constant->IsComplete());
}

void ConstantReactorTester::TestString() {
  auto constant = MakeConstantReactor<string>("hello world");
  constant->Commit();
  CPPUNIT_ASSERT(constant->IsComplete());
  CPPUNIT_ASSERT_EQUAL(constant->Eval(), string("hello world"));
  auto sequence = constant->GetSequenceNumber();
  constant->Commit();
  CPPUNIT_ASSERT_EQUAL(sequence, constant->GetSequenceNumber());
  CPPUNIT_ASSERT(!constant->IsInitializing());
  CPPUNIT_ASSERT(constant->IsComplete());
}
