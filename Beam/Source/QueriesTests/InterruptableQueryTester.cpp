#include "Beam/QueriesTests/InterruptableQueryTester.hpp"
#include "Beam/Queries/InterruptableQuery.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace std;

void InterruptableQueryTester::TestDefaultConstructor() {
  InterruptableQuery query;
  CPPUNIT_ASSERT(query.GetInterruptionPolicy() ==
    InterruptionPolicy::BREAK_QUERY);
}

void InterruptableQueryTester::TestInterruptionPolicyConstructor() {
  InterruptableQuery breakQuery(InterruptionPolicy::BREAK_QUERY);
  CPPUNIT_ASSERT(breakQuery.GetInterruptionPolicy() ==
    InterruptionPolicy::BREAK_QUERY);
  InterruptableQuery recoverQuery(InterruptionPolicy::RECOVER_DATA);
  CPPUNIT_ASSERT(recoverQuery.GetInterruptionPolicy() ==
    InterruptionPolicy::RECOVER_DATA);
  InterruptableQuery continueQuery(InterruptionPolicy::IGNORE_CONTINUE);
  CPPUNIT_ASSERT(continueQuery.GetInterruptionPolicy() ==
    InterruptionPolicy::IGNORE_CONTINUE);
}

void InterruptableQueryTester::TestSetInterruptionPolicy() {
  InterruptableQuery query;
  CPPUNIT_ASSERT(query.GetInterruptionPolicy() !=
    InterruptionPolicy::IGNORE_CONTINUE);
  query.SetInterruptionPolicy(InterruptionPolicy::IGNORE_CONTINUE);
  CPPUNIT_ASSERT(query.GetInterruptionPolicy() ==
    InterruptionPolicy::IGNORE_CONTINUE);
  query.SetInterruptionPolicy(InterruptionPolicy::RECOVER_DATA);
  CPPUNIT_ASSERT(query.GetInterruptionPolicy() ==
    InterruptionPolicy::RECOVER_DATA);
  query.SetInterruptionPolicy(InterruptionPolicy::BREAK_QUERY);
  CPPUNIT_ASSERT(query.GetInterruptionPolicy() ==
    InterruptionPolicy::BREAK_QUERY);
}
