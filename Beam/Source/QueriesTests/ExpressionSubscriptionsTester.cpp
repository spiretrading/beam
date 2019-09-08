#include "Beam/QueriesTests/ExpressionSubscriptionsTester.hpp"
#include "Beam/Queries/ExpressionSubscriptions.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace Beam::Serialization;
using namespace boost;
using namespace boost::posix_time;
using namespace std;

namespace {
  struct Entry {
    int m_value;
    ptime m_timestamp;
  };
}

void ExpressionSubscriptionsTester::TestPublish() {
  using TestSubscriptions = ExpressionSubscriptions<Entry, int,
    ServiceProtocolClient>;
  ServiceProtocolClient client(Initialize(), Initialize());
  TestSubscriptions subscriptions;
  auto filter = Translate(MakeConstantExpression(true));
  auto expression = Translate(MakeConstantExpression(123));
  auto queryId = 123;
  subscriptions.Initialize(client, queryId, Range::Total(), std::move(filter),
    ExpressionQuery::UpdatePolicy::ALL, std::move(expression));
  vector<SequencedValue<Entry>> snapshot;
  QueryResult<SequencedValue<int>> result;
  result.m_queryId = queryId;
  subscriptions.Commit(client, SnapshotLimit::Unlimited(), result, snapshot,
    [&] (QueryResult<SequencedValue<int>> committedSnapshot) {
      CPPUNIT_ASSERT(committedSnapshot.m_queryId == result.m_queryId);
    });
  subscriptions.Publish(SequencedValue(Entry{321, second_clock::local_time()},
    Sequence(5)),
    [&] (ServiceProtocolClient& senderClient, int id,
        const SequencedValue<int>& value) {
      CPPUNIT_ASSERT(&senderClient == &client);
    });
  subscriptions.End(client, queryId);
  subscriptions.Publish(SequencedValue(Entry{221, second_clock::local_time()},
    Sequence(6)),
    [&] (ServiceProtocolClient& senderClient, int id,
        const SequencedValue<int>& value) {
      CPPUNIT_ASSERT(false);
    });
}
