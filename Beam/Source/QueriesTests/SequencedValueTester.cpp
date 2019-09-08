#include "Beam/QueriesTests/SequencedValueTester.hpp"
#include "Beam/Queries/SequencedValue.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace std;

void SequencedValueTester::TestDefaultConstructor() {
  SequencedValue<int> intValue;
  CPPUNIT_ASSERT(intValue.GetSequence() == Sequence());
  CPPUNIT_ASSERT(intValue.GetValue() == 0);
  SequencedValue<string> stringValue;
  CPPUNIT_ASSERT(stringValue.GetSequence() == Sequence());
  CPPUNIT_ASSERT(stringValue.GetValue() == "");
}

void SequencedValueTester::TestValueAndSequenceConstructor() {
  SequencedValue<string> value("hello world", Sequence(1));
  CPPUNIT_ASSERT(value.GetValue() == "hello world");
  CPPUNIT_ASSERT(value.GetSequence() == Sequence(1));
}

void SequencedValueTester::TestDereference() {
  SequencedValue<int> intValue(123, Sequence(1));
  CPPUNIT_ASSERT(*intValue == 123);
  SequencedValue<string> stringValue("hello world", Sequence(1));
  CPPUNIT_ASSERT(*stringValue == "hello world");
}

void SequencedValueTester::TestEqualsOperator() {
  CPPUNIT_ASSERT(SequencedValue<string>("hello world", Sequence(1)) ==
    SequencedValue<string>("hello world", Sequence(1)));
  CPPUNIT_ASSERT(!(SequencedValue<string>("hello world", Sequence(1)) ==
    SequencedValue<string>("hello world", Sequence(2))));
  CPPUNIT_ASSERT(!(SequencedValue<string>("goodbye sky", Sequence(1)) ==
    SequencedValue<string>("hello world", Sequence(1))));
  CPPUNIT_ASSERT(!(SequencedValue<string>("goodbye sky", Sequence(1)) ==
    SequencedValue<string>("hello world", Sequence(2))));
}

void SequencedValueTester::TestNotEqualsOperator() {
  CPPUNIT_ASSERT(!(SequencedValue<string>("hello world", Sequence(1)) !=
    SequencedValue<string>("hello world", Sequence(1))));
  CPPUNIT_ASSERT(SequencedValue<string>("hello world", Sequence(2)) !=
    SequencedValue<string>("hello world", Sequence(1)));
  CPPUNIT_ASSERT(SequencedValue<string>("goodbye sky", Sequence(2)) !=
    SequencedValue<string>("hello world", Sequence(2)));
  CPPUNIT_ASSERT(SequencedValue<string>("goodbye sky", Sequence(1)) !=
    SequencedValue<string>("hello world", Sequence(2)));
}

void SequencedValueTester::TestMakeSequencedValue() {
  auto value = SequencedValue(321, Sequence(1));
  CPPUNIT_ASSERT(value.GetValue() == 321);
  CPPUNIT_ASSERT(value.GetSequence() == Sequence(1));
}
