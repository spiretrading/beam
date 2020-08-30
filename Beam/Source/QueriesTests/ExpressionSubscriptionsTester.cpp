#include <doctest/doctest.h>
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Queries/ExpressionSubscriptions.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/Threading/TriggerTimer.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;
using namespace Beam::Queries;
using namespace Beam::Routines;
using namespace Beam::Serialization;
using namespace Beam::Services;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;

namespace {
  struct Entry {
    int m_value;
    ptime m_timestamp;
  };

  using TestServiceProtocolClient = ServiceProtocolClient<
    MessageProtocol<LocalClientChannel<SharedBuffer>,
    BinarySender<SharedBuffer>, NullEncoder>, TriggerTimer>;
}

TEST_SUITE("ExpressionSubscriptions") {
  TEST_CASE("publish") {
    using TestSubscriptions = ExpressionSubscriptions<Entry, int,
      TestServiceProtocolClient>;
    auto server = LocalServerConnection<SharedBuffer>();
    auto serverChannel = std::unique_ptr<LocalServerChannel<SharedBuffer>>();
    Spawn([&] {
      serverChannel = server.Accept();
    });
    auto client = TestServiceProtocolClient(Initialize("test", server),
      Initialize());
    auto subscriptions = TestSubscriptions();
    auto filter = Translate(ConstantExpression(true));
    auto expression = Translate(ConstantExpression(123));
    auto queryId = 123;
    subscriptions.Initialize(client, queryId, Range::Total(), std::move(filter),
      ExpressionQuery::UpdatePolicy::ALL, std::move(expression));
    auto snapshot = std::vector<SequencedValue<Entry>>();
    auto result = QueryResult<SequencedValue<int>>();
    result.m_queryId = queryId;
    subscriptions.Commit(client, SnapshotLimit::Unlimited(), result, snapshot,
      [&] (QueryResult<SequencedValue<int>> committedSnapshot) {
        REQUIRE(committedSnapshot.m_queryId == result.m_queryId);
      });
    subscriptions.Publish(SequencedValue(Entry{321, second_clock::local_time()},
      Beam::Queries::Sequence(5)),
      [&] (TestServiceProtocolClient& senderClient, int id,
          const SequencedValue<int>& value) {
        REQUIRE(&senderClient == &client);
      });
    subscriptions.End(client, queryId);
    subscriptions.Publish(SequencedValue(Entry{221, second_clock::local_time()},
      Beam::Queries::Sequence(6)),
      [&] (TestServiceProtocolClient& senderClient, int id,
          const SequencedValue<int>& value) {
        REQUIRE(false);
      });
  }
}
