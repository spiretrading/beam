#include "Beam/QueriesTests/SubscriptionsTester.hpp"

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

void SubscriptionsTester::TestPublish() {
  using TestSubscriptions = Subscriptions<Entry, ServiceProtocolClient>;
  ServiceProtocolClient client(Initialize(), Initialize());
  TestSubscriptions subscriptions;
  auto filter = Translate(MakeConstantExpression(true));
  auto queryId = subscriptions.Initialize(client, Range::Total(),
    std::move(filter));
  QueryResult<SequencedValue<Entry>> snapshot;
  snapshot.m_queryId = queryId;
  subscriptions.Commit(snapshot,
    [&] (QueryResult<SequencedValue<Entry>> committedSnapshot) {
      CPPUNIT_ASSERT(committedSnapshot.m_queryId == snapshot.m_queryId);
    });
  subscriptions.Publish(
    SequencedValue(Entry{321, second_clock::local_time() }, Sequence(5)),
    [&] (std::vector<ServiceProtocolClient*>& receivingClients) {
      CPPUNIT_ASSERT(receivingClients.size() == 1);
      CPPUNIT_ASSERT(receivingClients.front() == &client);
    });
  subscriptions.End(queryId);
  subscriptions.Publish(
    SequencedValue(Entry{221, second_clock::local_time()}, Sequence(6)),
    [&] (std::vector<ServiceProtocolClient*>& receivingClients) {
      CPPUNIT_ASSERT(false);
    });
}
