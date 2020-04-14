#include <doctest/doctest.h>
#include "Beam/QueriesTests/SubscriptionsTester.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace Beam::Serialization;
using namespace boost;
using namespace boost::posix_time;

namespace {
  struct Entry {
    int m_value;
    ptime m_timestamp;
  };
}

TEST_SUITE("Subscriptions") {
  TEST_CASE("publish") {
    using TestSubscriptions = Subscriptions<Entry, ServiceProtocolClient>;
    ServiceProtocolClient client(Initialize(), Initialize());
    TestSubscriptions subscriptions;
    auto filter = Translate(ConstantExpression(true));
    auto queryId = subscriptions.Initialize(client, Range::Total(),
      std::move(filter));
    QueryResult<SequencedValue<Entry>> snapshot;
    snapshot.m_queryId = queryId;
    subscriptions.Commit(snapshot,
      [&] (QueryResult<SequencedValue<Entry>> committedSnapshot) {
        REQUIRE(committedSnapshot.m_queryId == snapshot.m_queryId);
      });
    subscriptions.Publish(
      SequencedValue(Entry{321, second_clock::local_time() }, Sequence(5)),
      [&] (std::vector<ServiceProtocolClient*>& receivingClients) {
        REQUIRE(receivingClients.size() == 1);
        REQUIRE(receivingClients.front() == &client);
      });
    subscriptions.End(queryId);
    subscriptions.Publish(
      SequencedValue(Entry{221, second_clock::local_time()}, Sequence(6)),
      [&] (std::vector<ServiceProtocolClient*>& receivingClients) {
        REQUIRE(false);
      });
  }
}
