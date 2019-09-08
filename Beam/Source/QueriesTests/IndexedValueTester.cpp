#include "Beam/QueriesTests/IndexedValueTester.hpp"
#include <string>
#include "Beam/Queries/IndexedValue.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace std;

void IndexedValueTester::TestDefaultConstructor() {
  IndexedValue<int, string> intValue;
  CPPUNIT_ASSERT(intValue.GetIndex().empty());
  CPPUNIT_ASSERT(intValue.GetValue() == 0);
  IndexedValue<string, string> stringValue;
  CPPUNIT_ASSERT(stringValue.GetIndex().empty());
  CPPUNIT_ASSERT(stringValue.GetValue().empty());
}

void IndexedValueTester::TestValueAndSequenceConstructor() {
  IndexedValue<string, string> value("hello world", "goodbye sky");
  CPPUNIT_ASSERT(value.GetValue() == "hello world");
  CPPUNIT_ASSERT(value.GetIndex() == "goodbye sky");
}

void IndexedValueTester::TestDereference() {
  IndexedValue<int, string> intValue(123, "index");
  CPPUNIT_ASSERT(*intValue == 123);
  IndexedValue<string, string> stringValue("hello world", "index");
  CPPUNIT_ASSERT(*stringValue == "hello world");
}

void IndexedValueTester::TestEqualsOperator() {
  typedef IndexedValue<int, string> TestIndexedValue;
  CPPUNIT_ASSERT(TestIndexedValue(123, "hello world") ==
    TestIndexedValue(123, "hello world"));
  CPPUNIT_ASSERT(!(TestIndexedValue(123, "hello world") ==
    TestIndexedValue(321, "hello world")));
  CPPUNIT_ASSERT(!(TestIndexedValue(123, "goodbye sky") ==
    TestIndexedValue(123, "hello world")));
  CPPUNIT_ASSERT(!(TestIndexedValue(123, "goodbye sky") ==
    TestIndexedValue(321, "hello world")));
}

void IndexedValueTester::TestNotEqualsOperator() {
  typedef IndexedValue<int, string> TestIndexedValue;
  CPPUNIT_ASSERT(!(TestIndexedValue(123, "hello world") !=
    TestIndexedValue(123, "hello world")));
  CPPUNIT_ASSERT(TestIndexedValue(123, "hello world") !=
    TestIndexedValue(321, "hello world"));
  CPPUNIT_ASSERT(TestIndexedValue(123, "goodbye sky") !=
    TestIndexedValue(123, "hello world"));
  CPPUNIT_ASSERT(TestIndexedValue(123, "goodbye sky") !=
    TestIndexedValue(321, "hello world"));
}

void IndexedValueTester::TestMakeIndexedValue() {
  auto value = IndexedValue(321, string("hello world"));
  CPPUNIT_ASSERT(value.GetValue() == 321);
  CPPUNIT_ASSERT(value.GetIndex() == "hello world");
}
