#include "Beam/QueriesTests/IndexedQueryTester.hpp"
#include <string>
#include "Beam/Queries/IndexedQuery.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace std;

void IndexedQueryTester::TestDefaultConstructor() {
  IndexedQuery<int> query;
  CPPUNIT_ASSERT(query.GetIndex() == 0);
}

void IndexedQueryTester::TestIndexConstructor() {
  IndexedQuery<int> intIndex(123);
  CPPUNIT_ASSERT(intIndex.GetIndex() == 123);
  IndexedQuery<string> stringIndex("hello world");
  CPPUNIT_ASSERT(stringIndex.GetIndex() == "hello world");
}

void IndexedQueryTester::TestSetIndex() {
  IndexedQuery<int> intIndex;
  CPPUNIT_ASSERT(intIndex.GetIndex() != 123);
  intIndex.SetIndex(123);
  CPPUNIT_ASSERT(intIndex.GetIndex() == 123);
  IndexedQuery<string> stringIndex;
  CPPUNIT_ASSERT(stringIndex.GetIndex() != "hello world");
  stringIndex.SetIndex("hello world");
  CPPUNIT_ASSERT(stringIndex.GetIndex() == "hello world");
}
