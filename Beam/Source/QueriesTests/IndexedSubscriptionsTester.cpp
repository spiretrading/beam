#include <doctest/doctest.h>
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Queries/IndexedSubscriptions.hpp"
#include "Beam/QueriesTests/TestEntry.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/TimeService/TriggerTimer.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;

namespace {
  using TestServiceProtocolClient = ServiceProtocolClient<
    MessageProtocol<LocalClientChannel, BinarySender<SharedBuffer>,
      NullEncoder>, TriggerTimer>;
  using TestSubscriptions =
    IndexedSubscriptions<TestEntry, std::string, TestServiceProtocolClient>;

  struct SingleClientFixture {
    LocalServerConnection m_server;
    std::unique_ptr<LocalServerChannel> m_server_channel;
    RoutineHandler m_accept_routine;
    TestServiceProtocolClient m_client;
    TestSubscriptions m_subscriptions;

    SingleClientFixture()
      : m_accept_routine(spawn([&] {
          m_server_channel = m_server.accept();
        })),
        m_client(init("test", m_server), init()) {}
  };
}

TEST_SUITE("IndexedSubscriptions") {
  TEST_CASE("publish_to_specific_index") {
    auto fixture = SingleClientFixture();
    auto [client, subscriptions] =
      std::tie(fixture.m_client, fixture.m_subscriptions);
    auto index_a = std::string("IndexA");
    auto filter = translate(ConstantExpression(true));
    auto query_id =
      subscriptions.init(index_a, client, Range::TOTAL, std::move(filter));
    auto snapshot = QueryResult<SequencedTestEntry>();
    snapshot.m_id = query_id;
    subscriptions.commit(index_a, snapshot,
      [&] (QueryResult<SequencedTestEntry> committed_snapshot) {
        REQUIRE(committed_snapshot.m_id == snapshot.m_id);
      });
    auto received_a = false;
    subscriptions.publish(SequencedValue(IndexedValue(
      TestEntry(100, time_from_string("2024-01-15 10:30:00")), index_a),
      Beam::Sequence(1)),
      [&] (auto& receiving_clients) {
        REQUIRE(receiving_clients.size() == 1);
        REQUIRE(receiving_clients.front() == &client);
        received_a = true;
      });
    REQUIRE(received_a);
    auto index_b = std::string("IndexB");
    subscriptions.publish(SequencedValue(IndexedValue(
      TestEntry(200, time_from_string("2024-01-15 10:30:01")), index_b),
      Beam::Sequence(2)),
      [&] (auto& receiving_clients) {
        REQUIRE(false);
      });
  }

  TEST_CASE("multiple_indexes_same_client") {
    auto fixture = SingleClientFixture();
    auto [client, subscriptions] =
      std::tie(fixture.m_client, fixture.m_subscriptions);
    auto index_a = std::string("IndexA");
    auto filter_a = translate(ConstantExpression(true));
    auto query_id_a =
      subscriptions.init(index_a, client, Range::TOTAL, std::move(filter_a));
    auto index_b = std::string("IndexB");
    auto filter_b = translate(ConstantExpression(true));
    auto query_id_b =
      subscriptions.init(index_b, client, Range::TOTAL, std::move(filter_b));
    auto snapshot_a = QueryResult<SequencedTestEntry>();
    snapshot_a.m_id = query_id_a;
    subscriptions.commit(index_a, snapshot_a,
      [&] (QueryResult<SequencedTestEntry> committed_snapshot) {
        REQUIRE(committed_snapshot.m_id == snapshot_a.m_id);
      });
    auto snapshot_b = QueryResult<SequencedTestEntry>();
    snapshot_b.m_id = query_id_b;
    subscriptions.commit(index_b, snapshot_b,
      [&] (QueryResult<SequencedTestEntry> committed_snapshot) {
        REQUIRE(committed_snapshot.m_id == snapshot_b.m_id);
      });
    auto received_a = false;
    subscriptions.publish(SequencedValue(IndexedValue(
      TestEntry(50, time_from_string("2024-01-15 10:30:00")), index_a),
      Beam::Sequence(1)),
      [&] (auto& receiving_clients) {
        REQUIRE(receiving_clients.size() == 1);
        REQUIRE(receiving_clients.front() == &client);
        received_a = true;
      });
    REQUIRE(received_a);
    auto received_b = false;
    subscriptions.publish(SequencedValue(IndexedValue(
      TestEntry(75, time_from_string("2024-01-15 10:30:01")), index_b),
      Beam::Sequence(2)),
      [&] (auto& receiving_clients) {
        REQUIRE(receiving_clients.size() == 1);
        REQUIRE(receiving_clients.front() == &client);
        received_b = true;
      });
    REQUIRE(received_b);
  }

  TEST_CASE("multiple_clients_same_index") {
    auto server1 = LocalServerConnection();
    auto server_channel1 = std::unique_ptr<LocalServerChannel>();
    spawn([&] {
      server_channel1 = server1.accept();
    });
    auto client1 = TestServiceProtocolClient(init("test1", server1), init());
    auto server2 = LocalServerConnection();
    auto server_channel2 = std::unique_ptr<LocalServerChannel>();
    spawn([&] {
      server_channel2 = server2.accept();
    });
    auto client2 = TestServiceProtocolClient(init("test2", server2), init());
    auto subscriptions = TestSubscriptions();
    auto index = std::string("SharedIndex");
    auto filter1 = translate(ConstantExpression(true));
    auto query_id1 =
      subscriptions.init(index, client1, Range::TOTAL, std::move(filter1));
    auto filter2 = translate(ConstantExpression(true));
    auto query_id2 =
      subscriptions.init(index, client2, Range::TOTAL, std::move(filter2));
    auto snapshot1 = QueryResult<SequencedTestEntry>();
    snapshot1.m_id = query_id1;
    subscriptions.commit(index, snapshot1,
      [&] (QueryResult<SequencedTestEntry> committed_snapshot) {
        REQUIRE(committed_snapshot.m_id == snapshot1.m_id);
      });
    auto snapshot2 = QueryResult<SequencedTestEntry>();
    snapshot2.m_id = query_id2;
    subscriptions.commit(index, snapshot2,
      [&] (QueryResult<SequencedTestEntry> committed_snapshot) {
        REQUIRE(committed_snapshot.m_id == snapshot2.m_id);
      });
    subscriptions.publish(SequencedValue(IndexedValue(
      TestEntry(50, time_from_string("2024-01-15 10:30:00")), index),
      Beam::Sequence(1)),
      [&] (auto& receiving_clients) {
        REQUIRE(receiving_clients.size() == 2);
        REQUIRE(std::find(receiving_clients.begin(), receiving_clients.end(),
          &client1) != receiving_clients.end());
        REQUIRE(std::find(receiving_clients.begin(), receiving_clients.end(),
          &client2) != receiving_clients.end());
      });
  }

  TEST_CASE("mixed_indexes_and_clients") {
    auto server1 = LocalServerConnection();
    auto server_channel1 = std::unique_ptr<LocalServerChannel>();
    spawn([&] {
      server_channel1 = server1.accept();
    });
    auto client1 = TestServiceProtocolClient(init("test1", server1), init());
    auto server2 = LocalServerConnection();
    auto server_channel2 = std::unique_ptr<LocalServerChannel>();
    spawn([&] {
      server_channel2 = server2.accept();
    });
    auto client2 = TestServiceProtocolClient(init("test2", server2), init());
    auto subscriptions = TestSubscriptions();
    auto index_a = std::string("IndexA");
    auto filter1 = translate(ConstantExpression(true));
    auto query_id1 =
      subscriptions.init(index_a, client1, Range::TOTAL, std::move(filter1));
    auto filter2 = translate(ConstantExpression(true));
    auto query_id2 =
      subscriptions.init(index_a, client2, Range::TOTAL, std::move(filter2));
    auto index_b = std::string("IndexB");
    auto filter3 = translate(ConstantExpression(true));
    auto query_id3 =
      subscriptions.init(index_b, client1, Range::TOTAL, std::move(filter3));
    auto snapshot1 = QueryResult<SequencedTestEntry>();
    snapshot1.m_id = query_id1;
    subscriptions.commit(index_a, snapshot1,
      [&] (QueryResult<SequencedTestEntry> committed_snapshot) {
        REQUIRE(committed_snapshot.m_id == snapshot1.m_id);
      });
    auto snapshot2 = QueryResult<SequencedTestEntry>();
    snapshot2.m_id = query_id2;
    subscriptions.commit(index_a, snapshot2,
      [&] (QueryResult<SequencedTestEntry> committed_snapshot) {
        REQUIRE(committed_snapshot.m_id == snapshot2.m_id);
      });
    auto snapshot3 = QueryResult<SequencedTestEntry>();
    snapshot3.m_id = query_id3;
    subscriptions.commit(index_b, snapshot3,
      [&] (QueryResult<SequencedTestEntry> committed_snapshot) {
        REQUIRE(committed_snapshot.m_id == snapshot3.m_id);
      });
    auto publish_count_a = 0;
    subscriptions.publish(SequencedValue(IndexedValue(
      TestEntry(50, time_from_string("2024-01-15 10:30:00")), index_a),
      Beam::Sequence(1)),
      [&] (auto& receiving_clients) {
        REQUIRE(receiving_clients.size() == 2);
        REQUIRE(std::find(receiving_clients.begin(), receiving_clients.end(),
          &client1) != receiving_clients.end());
        REQUIRE(std::find(receiving_clients.begin(), receiving_clients.end(),
          &client2) != receiving_clients.end());
        ++publish_count_a;
      });
    REQUIRE(publish_count_a == 1);
    auto publish_count_b = 0;
    subscriptions.publish(SequencedValue(IndexedValue(
      TestEntry(75, time_from_string("2024-01-15 10:30:01")), index_b),
      Beam::Sequence(2)),
      [&] (auto& receiving_clients) {
        REQUIRE(receiving_clients.size() == 1);
        REQUIRE(receiving_clients.front() == &client1);
        ++publish_count_b;
      });
    REQUIRE(publish_count_b == 1);
  }

  TEST_CASE("end_subscription_by_index") {
    auto fixture = SingleClientFixture();
    auto [client, subscriptions] =
      std::tie(fixture.m_client, fixture.m_subscriptions);
    auto index_a = std::string("IndexA");
    auto filter_a = translate(ConstantExpression(true));
    auto query_id_a =
      subscriptions.init(index_a, client, Range::TOTAL, std::move(filter_a));
    auto index_b = std::string("IndexB");
    auto filter_b = translate(ConstantExpression(true));
    auto query_id_b =
      subscriptions.init(index_b, client, Range::TOTAL, std::move(filter_b));
    auto snapshot_a = QueryResult<SequencedTestEntry>();
    snapshot_a.m_id = query_id_a;
    subscriptions.commit(index_a, snapshot_a,
      [&] (QueryResult<SequencedTestEntry> committed_snapshot) {
        REQUIRE(committed_snapshot.m_id == snapshot_a.m_id);
      });
    auto snapshot_b = QueryResult<SequencedTestEntry>();
    snapshot_b.m_id = query_id_b;
    subscriptions.commit(index_b, snapshot_b,
      [&] (QueryResult<SequencedTestEntry> committed_snapshot) {
        REQUIRE(committed_snapshot.m_id == snapshot_b.m_id);
      });
    subscriptions.end(index_a, query_id_a);
    subscriptions.publish(SequencedValue(IndexedValue(
      TestEntry(50, time_from_string("2024-01-15 10:30:00")), index_a),
      Beam::Sequence(1)),
      [&] (auto& receiving_clients) {
        REQUIRE(false);
      });
    auto received_b = false;
    subscriptions.publish(SequencedValue(IndexedValue(
      TestEntry(75, time_from_string("2024-01-15 10:30:01")), index_b),
      Beam::Sequence(2)),
      [&] (auto& receiving_clients) {
        REQUIRE(receiving_clients.size() == 1);
        REQUIRE(receiving_clients.front() == &client);
        received_b = true;
      });
    REQUIRE(received_b);
  }
}
