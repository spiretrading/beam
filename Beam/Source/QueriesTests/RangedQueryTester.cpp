#include "Beam/QueriesTests/RangedQueryTester.hpp"
#include "Beam/Queries/RangedQuery.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace std;

void RangedQueryTester::TestDefaultConstructor() {
  RangedQuery query;
  CPPUNIT_ASSERT(query.GetRange() == Range::Empty());
}

void RangedQueryTester::TestRangeConstructor() {
  RangedQuery totalRangeQuery(Range::Total());
  CPPUNIT_ASSERT(totalRangeQuery.GetRange() == Range::Total());
  RangedQuery realTimeRangeQuery(Range::RealTime());
  CPPUNIT_ASSERT(realTimeRangeQuery.GetRange() == Range::RealTime());
  RangedQuery sequenceRangeQuery(Range(Sequence(1), Sequence(2)));
  CPPUNIT_ASSERT(sequenceRangeQuery.GetRange() ==
    Range(Sequence(1), Sequence(2)));
}

void RangedQueryTester::TestSetRange() {
  RangedQuery query;
  CPPUNIT_ASSERT(query.GetRange() != Range::Total());
  query.SetRange(Range::Total());
  CPPUNIT_ASSERT(query.GetRange() == Range::Total());
  query.SetRange(Range::RealTime());
  CPPUNIT_ASSERT(query.GetRange() == Range::RealTime());
  query.SetRange(Range(Sequence(1), Sequence(2)));
  CPPUNIT_ASSERT(query.GetRange() == Range(Sequence(1), Sequence(2)));
}
