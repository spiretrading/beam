#include <doctest/doctest.h>
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Queries/ExpressionSubscriptions.hpp"
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
    ExpressionSubscriptions<TestEntry, int, TestServiceProtocolClient>;

  struct SingleClientFixture {
    LocalServerConnection m_server;
    std::unique_ptr<LocalServerChannel> m_server_channel;
    Routine::Id m_accept_routine;
    TestServiceProtocolClient m_client;
    TestSubscriptions m_subscriptions;

    SingleClientFixture()
      : m_accept_routine(spawn([&] {
          m_server_channel = m_server.accept();
        })),
        m_client(init("test", m_server), init()) {}
  };
}

TEST_SUITE("ExpressionSubscriptions") {
  TEST_CASE("publish") {
    auto fixture = SingleClientFixture();
    auto& client = fixture.m_client;
    auto subscriptions = TestSubscriptions();
    auto filter = translate<TestTranslator>(ConstantExpression(true));
    auto expression = translate<TestTranslator>(ConstantExpression(123));
    auto query_id = 123;
    subscriptions.init(client, query_id, Range::TOTAL, std::move(filter),
      ExpressionQuery::UpdatePolicy::ALL, std::move(expression));
    auto snapshot = std::vector<SequencedValue<TestEntry>>();
    auto result = QueryResult<SequencedValue<int>>();
    result.m_id = query_id;
    subscriptions.commit(client, SnapshotLimit::UNLIMITED, result, snapshot,
      [&] (QueryResult<SequencedValue<int>> committed_snapshot) {
        REQUIRE(committed_snapshot.m_id == result.m_id);
      });
    subscriptions.publish(
      SequencedValue(TestEntry(321, time_from_string("2024-01-15 10:30:00")),
        Beam::Sequence(5)),
      [&] (auto& sender_client, int id, const SequencedValue<int>& value) {
        REQUIRE(&sender_client == &client);
      });
    subscriptions.end(client, query_id);
    subscriptions.publish(
      SequencedValue(TestEntry(221, time_from_string("2024-01-15 10:30:01")),
        Beam::Sequence(6)),
      [&] (auto& sender_client, int id, const SequencedValue<int>& value) {
        REQUIRE(false);
      });
  }

  TEST_CASE("multiple_subscriptions_same_client") {
    auto fixture = SingleClientFixture();
    auto& client = fixture.m_client;
    auto subscriptions = TestSubscriptions();
    auto filter1 = translate<TestTranslator>(ConstantExpression(true));
    auto expression1 = translate<TestTranslator>(
      MemberAccessExpression("value", typeid(int),
        ParameterExpression(0, typeid(TestEntry))));
    auto query_id1 = 100;
    subscriptions.init(client, query_id1, Range::TOTAL, std::move(filter1),
      ExpressionQuery::UpdatePolicy::ALL, std::move(expression1));
    auto filter2 = translate<TestTranslator>(ConstantExpression(true));
    auto expression2 = translate<TestTranslator>(ConstantExpression(999));
    auto query_id2 = 200;
    subscriptions.init(client, query_id2, Range::TOTAL, std::move(filter2),
      ExpressionQuery::UpdatePolicy::ALL, std::move(expression2));
    auto snapshot1 = std::vector<SequencedValue<TestEntry>>();
    auto result1 = QueryResult<SequencedValue<int>>();
    result1.m_id = query_id1;
    subscriptions.commit(client, SnapshotLimit::UNLIMITED, result1, snapshot1,
      [&] (QueryResult<SequencedValue<int>> committed_snapshot) {
        REQUIRE(committed_snapshot.m_id == result1.m_id);
      });
    auto snapshot2 = std::vector<SequencedValue<TestEntry>>();
    auto result2 = QueryResult<SequencedValue<int>>();
    result2.m_id = query_id2;
    subscriptions.commit(client, SnapshotLimit::UNLIMITED, result2, snapshot2,
      [&] (QueryResult<SequencedValue<int>> committed_snapshot) {
        REQUIRE(committed_snapshot.m_id == result2.m_id);
      });
    auto received_count1 = 0;
    auto received_count2 = 0;
    subscriptions.publish(
      SequencedValue(TestEntry(42, time_from_string("2024-01-15 10:30:00")),
        Beam::Sequence(1)),
      [&] (auto& sender_client, int id, const SequencedValue<int>& value) {
        REQUIRE(&sender_client == &client);
        if(id == query_id1) {
          REQUIRE(value.get_value() == 42);
          ++received_count1;
        } else if(id == query_id2) {
          REQUIRE(value.get_value() == 999);
          ++received_count2;
        }
      });
    REQUIRE(received_count1 == 1);
    REQUIRE(received_count2 == 1);
  }

  TEST_CASE("duplicate_query_id") {
    auto fixture = SingleClientFixture();
    auto& client = fixture.m_client;
    auto subscriptions = TestSubscriptions();
    auto filter = translate<TestTranslator>(ConstantExpression(true));
    auto expression = translate<TestTranslator>(ConstantExpression(123));
    auto query_id = 456;
    subscriptions.init(client, query_id, Range::TOTAL, std::move(filter),
      ExpressionQuery::UpdatePolicy::ALL, std::move(expression));
    auto filter2 = translate<TestTranslator>(ConstantExpression(true));
    auto expression2 = translate<TestTranslator>(ConstantExpression(789));
    REQUIRE_THROWS_AS(subscriptions.init(client, query_id, Range::TOTAL,
      std::move(filter2), ExpressionQuery::UpdatePolicy::ALL,
      std::move(expression2)), std::runtime_error);
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
    auto filter1 = translate<TestTranslator>(ConstantExpression(true));
    auto expression1 = translate<TestTranslator>(ConstantExpression(100));
    auto query_id1 = 1;
    subscriptions.init(client1, query_id1, Range::TOTAL, std::move(filter1),
      ExpressionQuery::UpdatePolicy::ALL, std::move(expression1));
    auto filter2 = translate<TestTranslator>(ConstantExpression(true));
    auto expression2 = translate<TestTranslator>(ConstantExpression(200));
    auto query_id2 = 1;
    subscriptions.init(client2, query_id2, Range::TOTAL, std::move(filter2),
      ExpressionQuery::UpdatePolicy::ALL, std::move(expression2));
    auto snapshot1 = std::vector<SequencedValue<TestEntry>>();
    auto result1 = QueryResult<SequencedValue<int>>();
    result1.m_id = query_id1;
    subscriptions.commit(client1, SnapshotLimit::UNLIMITED, result1, snapshot1,
      [&] (QueryResult<SequencedValue<int>> committed_snapshot) {
        REQUIRE(committed_snapshot.m_id == result1.m_id);
      });
    auto snapshot2 = std::vector<SequencedValue<TestEntry>>();
    auto result2 = QueryResult<SequencedValue<int>>();
    result2.m_id = query_id2;
    subscriptions.commit(client2, SnapshotLimit::UNLIMITED, result2, snapshot2,
      [&] (QueryResult<SequencedValue<int>> committed_snapshot) {
        REQUIRE(committed_snapshot.m_id == result2.m_id);
      });
    auto client1_received = false;
    auto client2_received = false;
    subscriptions.publish(
      SequencedValue(TestEntry(50, time_from_string("2024-01-15 10:30:00")),
        Beam::Sequence(1)),
      [&] (auto& sender_client, int id, const SequencedValue<int>& value) {
        if(&sender_client == &client1) {
          REQUIRE(id == query_id1);
          REQUIRE(value.get_value() == 100);
          client1_received = true;
        } else if(&sender_client == &client2) {
          REQUIRE(id == query_id2);
          REQUIRE(value.get_value() == 200);
          client2_received = true;
        }
      });
    REQUIRE(client1_received);
    REQUIRE(client2_received);
  }

  TEST_CASE("filter") {
    auto fixture = SingleClientFixture();
    auto& client = fixture.m_client;
    auto subscriptions = TestSubscriptions();
    auto value_expression = MemberAccessExpression(
      "value", typeid(int), ParameterExpression(0, typeid(TestEntry)));
    auto filter = translate<TestTranslator>(
      value_expression >= ConstantExpression(100));
    auto expression = translate<TestTranslator>(MemberAccessExpression(
      "value", typeid(int), ParameterExpression(0, typeid(TestEntry))));
    auto query_id = 1;
    subscriptions.init(client, query_id, Range::TOTAL, std::move(filter),
      ExpressionQuery::UpdatePolicy::ALL, std::move(expression));
    auto snapshot = std::vector<SequencedValue<TestEntry>>();
    auto result = QueryResult<SequencedValue<int>>();
    result.m_id = query_id;
    subscriptions.commit(client, SnapshotLimit::UNLIMITED, result, snapshot,
      [&] (QueryResult<SequencedValue<int>> committed_snapshot) {
        REQUIRE(committed_snapshot.m_id == result.m_id);
      });
    subscriptions.publish(
      SequencedValue(TestEntry(50, time_from_string("2024-01-15 10:30:00")),
        Beam::Sequence(1)),
      [&] (auto& sender_client, int id, const SequencedValue<int>& value) {
        REQUIRE(false);
      });
    subscriptions.publish(
      SequencedValue(TestEntry(75, time_from_string("2024-01-15 10:30:01")),
        Beam::Sequence(2)),
      [&] (auto& sender_client, int id, const SequencedValue<int>& value) {
        REQUIRE(false);
      });
    auto received = false;
    subscriptions.publish(
      SequencedValue(TestEntry(150, time_from_string("2024-01-15 10:30:02")),
        Beam::Sequence(3)),
      [&] (auto& sender_client, int id, const SequencedValue<int>& value) {
        REQUIRE(&sender_client == &client);
        REQUIRE(id == query_id);
        REQUIRE(value.get_value() == 150);
        received = true;
      });
    REQUIRE(received);
  }

  TEST_CASE("range_filtering") {
    auto fixture = SingleClientFixture();
    auto& client = fixture.m_client;
    auto subscriptions = TestSubscriptions();
    auto filter = translate<TestTranslator>(ConstantExpression(true));
    auto expression = translate<TestTranslator>(MemberAccessExpression(
      "value", typeid(int), ParameterExpression(0, typeid(TestEntry))));
    auto query_id = 1;
    auto range = Range(Beam::Sequence(5), Beam::Sequence(10));
    subscriptions.init(client, query_id, range, std::move(filter),
      ExpressionQuery::UpdatePolicy::ALL, std::move(expression));
    auto snapshot = std::vector<SequencedValue<TestEntry>>();
    auto result = QueryResult<SequencedValue<int>>();
    result.m_id = query_id;
    subscriptions.commit(client, SnapshotLimit::UNLIMITED, result, snapshot,
      [&] (QueryResult<SequencedValue<int>> committed_snapshot) {
        REQUIRE(committed_snapshot.m_id == result.m_id);
      });
    subscriptions.publish(
      SequencedValue(TestEntry(100, time_from_string("2024-01-15 10:30:00")),
        Beam::Sequence(3)),
      [&] (auto& sender_client, int id, const SequencedValue<int>& value) {
        REQUIRE(false);
      });
    auto received_count = 0;
    subscriptions.publish(
      SequencedValue(TestEntry(200, time_from_string("2024-01-15 10:30:01")),
        Beam::Sequence(7)),
      [&] (auto& sender_client, int id, const SequencedValue<int>& value) {
        REQUIRE(&sender_client == &client);
        REQUIRE(id == query_id);
        REQUIRE(value.get_value() == 200);
        ++received_count;
      });
    subscriptions.publish(
      SequencedValue(TestEntry(300, time_from_string("2024-01-15 10:30:02")),
        Beam::Sequence(12)),
      [&] (auto& sender_client, int id, const SequencedValue<int>& value) {
        REQUIRE(false);
      });
    REQUIRE(received_count == 1);
  }
}
