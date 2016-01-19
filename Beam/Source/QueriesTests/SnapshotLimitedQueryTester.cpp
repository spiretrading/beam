#include "Beam/QueriesTests/SnapshotLimitedQueryTester.hpp"
#include "Beam/Queries/SnapshotLimitedQuery.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace std;

void SnapshotLimitedQueryTester::TestDefaultConstructor() {
  SnapshotLimitedQuery query;
  CPPUNIT_ASSERT(query.GetSnapshotLimit() == SnapshotLimit::None());
}

void SnapshotLimitedQueryTester::TestSnapshotLimitConstructor() {
  SnapshotLimitedQuery unlimitedQuery(SnapshotLimit::Unlimited());
  CPPUNIT_ASSERT(unlimitedQuery.GetSnapshotLimit() ==
    SnapshotLimit::Unlimited());
  SnapshotLimitedQuery headQuery(SnapshotLimit(SnapshotLimit::Type::HEAD, 100));
  CPPUNIT_ASSERT(headQuery.GetSnapshotLimit() ==
    SnapshotLimit(SnapshotLimit::Type::HEAD, 100));
  SnapshotLimitedQuery tailQuery(SnapshotLimit(SnapshotLimit::Type::TAIL, 200));
  CPPUNIT_ASSERT(tailQuery.GetSnapshotLimit() ==
    SnapshotLimit(SnapshotLimit::Type::TAIL, 200));
}

void SnapshotLimitedQueryTester::TestSetSnapshotLimit() {
  SnapshotLimitedQuery query;
  CPPUNIT_ASSERT(query.GetSnapshotLimit() != SnapshotLimit::Unlimited());
  query.SetSnapshotLimit(SnapshotLimit::Unlimited());
  CPPUNIT_ASSERT(query.GetSnapshotLimit() == SnapshotLimit::Unlimited());
  query.SetSnapshotLimit(SnapshotLimit(SnapshotLimit::Type::HEAD, 100));
  CPPUNIT_ASSERT(query.GetSnapshotLimit() ==
    SnapshotLimit(SnapshotLimit::Type::HEAD, 100));
  query.SetSnapshotLimit(SnapshotLimit(SnapshotLimit::Type::TAIL, 200));
  CPPUNIT_ASSERT(query.GetSnapshotLimit() ==
    SnapshotLimit(SnapshotLimit::Type::TAIL, 200));
}
