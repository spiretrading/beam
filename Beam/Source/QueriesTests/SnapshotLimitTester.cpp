#include "Beam/QueriesTests/SnapshotLimitTester.hpp"
#include "Beam/Queries/SnapshotLimit.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace std;

void SnapshotLimitTester::TestDefaultConstructor() {
  SnapshotLimit limit;
  CPPUNIT_ASSERT(limit.GetSize() == 0);
  CPPUNIT_ASSERT(limit.GetType() == SnapshotLimit::Type::HEAD);
}

void SnapshotLimitTester::TestConstructor() {
  SnapshotLimit headLimit(SnapshotLimit::Type::HEAD, 123);
  CPPUNIT_ASSERT(headLimit.GetType() == SnapshotLimit::Type::HEAD);
  CPPUNIT_ASSERT(headLimit.GetSize() == 123);
  SnapshotLimit negativeHeadLimit(SnapshotLimit::Type::HEAD, -123);
  CPPUNIT_ASSERT(negativeHeadLimit.GetType() == SnapshotLimit::Type::HEAD);
  CPPUNIT_ASSERT(negativeHeadLimit.GetSize() == 0);
  SnapshotLimit emptyHeadLimit(SnapshotLimit::Type::HEAD, 0);
  CPPUNIT_ASSERT(emptyHeadLimit.GetType() == SnapshotLimit::Type::HEAD);
  CPPUNIT_ASSERT(emptyHeadLimit.GetSize() == 0);
  SnapshotLimit tailLimit(SnapshotLimit::Type::TAIL, 123);
  CPPUNIT_ASSERT(tailLimit.GetType() == SnapshotLimit::Type::TAIL);
  CPPUNIT_ASSERT(tailLimit.GetSize() == 123);
  SnapshotLimit negativeTailLimit(SnapshotLimit::Type::TAIL, -123);
  CPPUNIT_ASSERT(negativeTailLimit.GetType() == SnapshotLimit::Type::HEAD);
  CPPUNIT_ASSERT(negativeTailLimit.GetSize() == 0);
  SnapshotLimit emptyTailLimit(SnapshotLimit::Type::TAIL, 0);
  CPPUNIT_ASSERT(emptyTailLimit.GetType() == SnapshotLimit::Type::HEAD);
  CPPUNIT_ASSERT(emptyTailLimit.GetSize() == 0);
}

void SnapshotLimitTester::TestNoneSnapshotLimit() {
  CPPUNIT_ASSERT(SnapshotLimit::None().GetSize() == 0);
}

void SnapshotLimitTester::TestUnlimitedSnapshotLimit() {
  CPPUNIT_ASSERT(SnapshotLimit::Unlimited().GetSize() ==
    numeric_limits<int>::max());
}

void SnapshotLimitTester::TestEqualsOperator() {
  CPPUNIT_ASSERT(SnapshotLimit(SnapshotLimit::Type::HEAD, 0) ==
    SnapshotLimit(SnapshotLimit::Type::HEAD, 0));
  CPPUNIT_ASSERT(SnapshotLimit(SnapshotLimit::Type::HEAD, 0) ==
    SnapshotLimit(SnapshotLimit::Type::TAIL, 0));
  CPPUNIT_ASSERT(SnapshotLimit(SnapshotLimit::Type::HEAD, 123) ==
    SnapshotLimit(SnapshotLimit::Type::HEAD, 123));
  CPPUNIT_ASSERT(
    SnapshotLimit(SnapshotLimit::Type::HEAD, numeric_limits<int>::max()) ==
    SnapshotLimit(SnapshotLimit::Type::TAIL, numeric_limits<int>::max()));
  CPPUNIT_ASSERT(SnapshotLimit(SnapshotLimit::Type::TAIL, 0) ==
    SnapshotLimit(SnapshotLimit::Type::TAIL, 0));
  CPPUNIT_ASSERT(SnapshotLimit(SnapshotLimit::Type::TAIL, 0) ==
    SnapshotLimit(SnapshotLimit::Type::HEAD, 0));
  CPPUNIT_ASSERT(SnapshotLimit(SnapshotLimit::Type::TAIL, 123) ==
    SnapshotLimit(SnapshotLimit::Type::TAIL, 123));
  CPPUNIT_ASSERT(
    SnapshotLimit(SnapshotLimit::Type::TAIL, numeric_limits<int>::max()) ==
    SnapshotLimit(SnapshotLimit::Type::HEAD, numeric_limits<int>::max()));
  CPPUNIT_ASSERT(!(SnapshotLimit(SnapshotLimit::Type::HEAD, 1) ==
    SnapshotLimit(SnapshotLimit::Type::TAIL, 1)));
  CPPUNIT_ASSERT(!(SnapshotLimit(SnapshotLimit::Type::HEAD, 123) ==
    SnapshotLimit(SnapshotLimit::Type::HEAD, 456)));
  CPPUNIT_ASSERT(!(SnapshotLimit(SnapshotLimit::Type::TAIL, 1) ==
    SnapshotLimit(SnapshotLimit::Type::HEAD, 1)));
  CPPUNIT_ASSERT(!(SnapshotLimit(SnapshotLimit::Type::TAIL, 123) ==
    SnapshotLimit(SnapshotLimit::Type::TAIL, 456)));
}

void SnapshotLimitTester::TestNotEqualsOperator() {
  CPPUNIT_ASSERT(!(SnapshotLimit(SnapshotLimit::Type::HEAD, 0) !=
    SnapshotLimit(SnapshotLimit::Type::HEAD, 0)));
  CPPUNIT_ASSERT(!(SnapshotLimit(SnapshotLimit::Type::HEAD, 0) !=
    SnapshotLimit(SnapshotLimit::Type::TAIL, 0)));
  CPPUNIT_ASSERT(!(SnapshotLimit(SnapshotLimit::Type::HEAD, 123) !=
    SnapshotLimit(SnapshotLimit::Type::HEAD, 123)));
  CPPUNIT_ASSERT(
    !(SnapshotLimit(SnapshotLimit::Type::HEAD, numeric_limits<int>::max()) !=
    SnapshotLimit(SnapshotLimit::Type::TAIL, numeric_limits<int>::max())));
  CPPUNIT_ASSERT(!(SnapshotLimit(SnapshotLimit::Type::TAIL, 0) !=
    SnapshotLimit(SnapshotLimit::Type::TAIL, 0)));
  CPPUNIT_ASSERT(!(SnapshotLimit(SnapshotLimit::Type::TAIL, 0) !=
    SnapshotLimit(SnapshotLimit::Type::HEAD, 0)));
  CPPUNIT_ASSERT(!(SnapshotLimit(SnapshotLimit::Type::TAIL, 123) !=
    SnapshotLimit(SnapshotLimit::Type::TAIL, 123)));
  CPPUNIT_ASSERT(
    !(SnapshotLimit(SnapshotLimit::Type::TAIL, numeric_limits<int>::max()) !=
    SnapshotLimit(SnapshotLimit::Type::HEAD, numeric_limits<int>::max())));
  CPPUNIT_ASSERT(SnapshotLimit(SnapshotLimit::Type::HEAD, 1) !=
    SnapshotLimit(SnapshotLimit::Type::TAIL, 1));
  CPPUNIT_ASSERT(SnapshotLimit(SnapshotLimit::Type::HEAD, 123) !=
    SnapshotLimit(SnapshotLimit::Type::HEAD, 456));
  CPPUNIT_ASSERT(SnapshotLimit(SnapshotLimit::Type::TAIL, 1) !=
    SnapshotLimit(SnapshotLimit::Type::HEAD, 1));
  CPPUNIT_ASSERT(SnapshotLimit(SnapshotLimit::Type::TAIL, 123) !=
    SnapshotLimit(SnapshotLimit::Type::TAIL, 456));
}
