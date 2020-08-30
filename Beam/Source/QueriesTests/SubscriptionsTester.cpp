#include <doctest/doctest.h>
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Queries/Subscriptions.hpp"
#include "Beam/QueriesTests/TestEntry.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Threading/TriggerTimer.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace Beam::Routines;
using namespace Beam::Serialization;
using namespace Beam::Services;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;

namespace {
  using TestServiceProtocolClient = ServiceProtocolClient<
    MessageProtocol<LocalClientChannel<SharedBuffer>,
    BinarySender<SharedBuffer>, NullEncoder>, TriggerTimer>;
}

TEST_SUITE("Subscriptions") {
  TEST_CASE("publish") {
    using TestSubscriptions =
      Subscriptions<TestEntry, TestServiceProtocolClient>;
    auto server = LocalServerConnection<SharedBuffer>();
    auto serverChannel = std::unique_ptr<LocalServerChannel<SharedBuffer>>();
    Spawn([&] {
      serverChannel = server.Accept();
    });
    auto client = TestServiceProtocolClient(Initialize("test", server),
      Initialize());
    auto subscriptions = TestSubscriptions();
    auto filter = Translate(ConstantExpression(true));
    auto queryId = subscriptions.Initialize(client, Range::Total(),
      std::move(filter));
    auto snapshot = QueryResult<SequencedTestEntry>();
    snapshot.m_queryId = queryId;
    subscriptions.Commit(snapshot,
      [&] (QueryResult<SequencedTestEntry> committedSnapshot) {
        REQUIRE(committedSnapshot.m_queryId == snapshot.m_queryId);
      });
    subscriptions.Publish(SequencedValue(
      TestEntry{321, second_clock::local_time()}, Beam::Queries::Sequence(5)),
      [&] (std::vector<TestServiceProtocolClient*>& receivingClients) {
        REQUIRE(receivingClients.size() == 1);
        REQUIRE(receivingClients.front() == &client);
      });
    subscriptions.End(queryId);
    subscriptions.Publish(SequencedValue(
      TestEntry{221, second_clock::local_time()}, Beam::Queries::Sequence(6)),
      [&] (std::vector<TestServiceProtocolClient*>& receivingClients) {
        REQUIRE(false);
      });
  }
}
