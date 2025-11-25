#include <doctest/doctest.h>
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Queries/Subscriptions.hpp"
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
  using TestSubscriptions = Subscriptions<TestEntry, TestServiceProtocolClient>;

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

TEST_SUITE("Subscriptions") {
  TEST_CASE("publish") {
    auto fixture = SingleClientFixture();
    auto [client, subscriptions] =
      std::tie(fixture.m_client, fixture.m_subscriptions);
    auto filter = translate(ConstantExpression(true));
    auto query_id = subscriptions.init(client, Range::TOTAL, std::move(filter));
    auto snapshot = QueryResult<SequencedTestEntry>();
    snapshot.m_id = query_id;
    subscriptions.commit(snapshot,
      [&] (QueryResult<SequencedTestEntry> committed_snapshot) {
        REQUIRE(committed_snapshot.m_id == snapshot.m_id);
      });
    subscriptions.publish(
      SequencedValue(TestEntry(321, time_from_string("2024-01-15 10:30:00")),
        Beam::Sequence(5)),
      [&] (auto& receiving_clients) {
        REQUIRE(receiving_clients.size() == 1);
        REQUIRE(receiving_clients.front() == &client);
      });
    subscriptions.end(query_id);
    subscriptions.publish(
      SequencedValue(TestEntry(221, time_from_string("2024-01-15 10:30:01")),
        Beam::Sequence(6)),
      [&] (auto& receiving_clients) {
        REQUIRE(false);
      });
  }

  TEST_CASE("multiple_subscriptions_same_client") {
    auto fixture = SingleClientFixture();
    auto [client, subscriptions] =
      std::tie(fixture.m_client, fixture.m_subscriptions);
    auto filter1 = translate(ConstantExpression(true));
    auto query_id1 =
      subscriptions.init(client, Range::TOTAL, std::move(filter1));
    auto filter2 = translate(ConstantExpression(true));
    auto query_id2 =
      subscriptions.init(client, Range::TOTAL, std::move(filter2));
    auto snapshot1 = QueryResult<SequencedTestEntry>();
    snapshot1.m_id = query_id1;
    subscriptions.commit(snapshot1,
      [&] (QueryResult<SequencedTestEntry> committed_snapshot) {
        REQUIRE(committed_snapshot.m_id == snapshot1.m_id);
      });
    auto snapshot2 = QueryResult<SequencedTestEntry>();
    snapshot2.m_id = query_id2;
    subscriptions.commit(snapshot2,
      [&] (QueryResult<SequencedTestEntry> committed_snapshot) {
        REQUIRE(committed_snapshot.m_id == snapshot2.m_id);
      });
    auto publish_count = 0;
    subscriptions.publish(
      SequencedValue(TestEntry(100, time_from_string("2024-01-15 10:30:00")),
        Beam::Sequence(1)),
      [&] (auto& receiving_clients) {
        REQUIRE(receiving_clients.size() == 1);
        REQUIRE(receiving_clients.front() == &client);
        ++publish_count;
      });
    REQUIRE(publish_count == 1);
  }

  TEST_CASE("multiple_clients") {
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
    auto filter1 = translate(ConstantExpression(true));
    auto query_id1 =
      subscriptions.init(client1, Range::TOTAL, std::move(filter1));
    auto filter2 = translate(ConstantExpression(true));
    auto query_id2 =
      subscriptions.init(client2, Range::TOTAL, std::move(filter2));
    auto snapshot1 = QueryResult<SequencedTestEntry>();
    snapshot1.m_id = query_id1;
    subscriptions.commit(snapshot1,
      [&] (QueryResult<SequencedTestEntry> committed_snapshot) {
        REQUIRE(committed_snapshot.m_id == snapshot1.m_id);
      });
    auto snapshot2 = QueryResult<SequencedTestEntry>();
    snapshot2.m_id = query_id2;
    subscriptions.commit(snapshot2,
      [&] (QueryResult<SequencedTestEntry> committed_snapshot) {
        REQUIRE(committed_snapshot.m_id == snapshot2.m_id);
      });
    subscriptions.publish(
      SequencedValue(TestEntry(200, time_from_string("2024-01-15 10:30:00")),
        Beam::Sequence(1)),
      [&] (auto& receiving_clients) {
        REQUIRE(receiving_clients.size() == 2);
        REQUIRE(std::ranges::find(receiving_clients, &client1) !=
          receiving_clients.end());
        REQUIRE(std::ranges::find(receiving_clients, &client2) !=
          receiving_clients.end());
      });
  }

  TEST_CASE("filter") {
    auto fixture = SingleClientFixture();
    auto [client, subscriptions] =
      std::tie(fixture.m_client, fixture.m_subscriptions);
    auto value_expression = MemberAccessExpression(
      "value", typeid(int), ParameterExpression(0, typeid(TestEntry)));
    auto filter =
      translate<TestTranslator>(value_expression >= ConstantExpression(100));
    auto query_id = subscriptions.init(client, Range::TOTAL, std::move(filter));
    auto snapshot = QueryResult<SequencedTestEntry>();
    snapshot.m_id = query_id;
    subscriptions.commit(snapshot,
      [&] (QueryResult<SequencedTestEntry> committed_snapshot) {
        REQUIRE(committed_snapshot.m_id == snapshot.m_id);
      });
    subscriptions.publish(
      SequencedValue(TestEntry(50, time_from_string("2024-01-15 10:30:00")),
        Beam::Sequence(1)),
      [&] (auto& receiving_clients) {
        REQUIRE(false);
      });
    subscriptions.publish(
      SequencedValue(TestEntry(75, time_from_string("2024-01-15 10:30:01")),
        Beam::Sequence(2)),
      [&] (auto& receiving_clients) {
        REQUIRE(false);
      });
    auto received = false;
    subscriptions.publish(
      SequencedValue(TestEntry(150, time_from_string("2024-01-15 10:30:02")),
        Beam::Sequence(3)),
      [&] (auto& receiving_clients) {
        REQUIRE(receiving_clients.size() == 1);
        REQUIRE(receiving_clients.front() == &client);
        received = true;
      });
    REQUIRE(received);
  }

  TEST_CASE("range_filtering") {
    auto fixture = SingleClientFixture();
    auto [client, subscriptions] =
      std::tie(fixture.m_client, fixture.m_subscriptions);
    auto filter = translate(ConstantExpression(true));
    auto range = Range(Beam::Sequence(5), Beam::Sequence::LAST);
    auto query_id = subscriptions.init(client, range, std::move(filter));
    auto snapshot = QueryResult<SequencedTestEntry>();
    snapshot.m_id = query_id;
    subscriptions.commit(snapshot,
      [&] (QueryResult<SequencedTestEntry> committed_snapshot) {
        REQUIRE(committed_snapshot.m_id == snapshot.m_id);
      });
    subscriptions.publish(
      SequencedValue(TestEntry(100, time_from_string("2024-01-15 10:30:00")),
        Beam::Sequence(3)),
      [&] (auto& receiving_clients) {
        REQUIRE(false);
      });
    auto received = false;
    subscriptions.publish(
      SequencedValue(TestEntry(200, time_from_string("2024-01-15 10:30:01")),
        Beam::Sequence(7)),
      [&] (auto& receiving_clients) {
        REQUIRE(receiving_clients.size() == 1);
        REQUIRE(receiving_clients.front() == &client);
        received = true;
      });
    REQUIRE(received);
  }

  TEST_CASE("add_method") {
    auto fixture = SingleClientFixture();
    auto [client, subscriptions] =
      std::tie(fixture.m_client, fixture.m_subscriptions);
    auto filter = translate(ConstantExpression(true));
    auto query_id = subscriptions.add(client, Range::TOTAL, std::move(filter));
    REQUIRE(query_id > 0);
    auto received = false;
    subscriptions.publish(
      SequencedValue(TestEntry(300, time_from_string("2024-01-15 10:30:00")),
        Beam::Sequence(1)),
      [&] (auto& receiving_clients) {
        REQUIRE(receiving_clients.size() == 1);
        REQUIRE(receiving_clients.front() == &client);
        received = true;
      });
    REQUIRE(received);
  }

  TEST_CASE("write_log_merge") {
    auto fixture = SingleClientFixture();
    auto [client, subscriptions] =
      std::tie(fixture.m_client, fixture.m_subscriptions);
    auto filter = translate(ConstantExpression(true));
    auto query_id = subscriptions.init(client, Range::TOTAL, std::move(filter));
    subscriptions.publish(
      SequencedValue(TestEntry(100, time_from_string("2024-01-15 10:30:00")),
        Beam::Sequence(1)),
      [&] (auto& receiving_clients) {
        REQUIRE(false);
      });
    subscriptions.publish(
      SequencedValue(TestEntry(200, time_from_string("2024-01-15 10:30:01")),
        Beam::Sequence(2)),
      [&] (auto& receiving_clients) {
        REQUIRE(false);
      });
    auto snapshot = QueryResult<SequencedTestEntry>();
    snapshot.m_id = query_id;
    subscriptions.commit(snapshot,
      [&] (QueryResult<SequencedTestEntry> committed_snapshot) {
        REQUIRE(committed_snapshot.m_id == query_id);
        REQUIRE(committed_snapshot.m_snapshot.size() == 2);
        REQUIRE(committed_snapshot.m_snapshot[0].get_value().m_value == 100);
        REQUIRE(committed_snapshot.m_snapshot[0].get_sequence() ==
          Beam::Sequence(1));
        REQUIRE(committed_snapshot.m_snapshot[1].get_value().m_value == 200);
        REQUIRE(committed_snapshot.m_snapshot[1].get_sequence() ==
          Beam::Sequence(2));
      });
    auto received = false;
    subscriptions.publish(
      SequencedValue(TestEntry(300, time_from_string("2024-01-15 10:30:02")),
        Beam::Sequence(3)),
      [&] (auto& receiving_clients) {
        REQUIRE(receiving_clients.size() == 1);
        REQUIRE(receiving_clients.front() == &client);
        received = true;
      });
    REQUIRE(received);
  }
}
