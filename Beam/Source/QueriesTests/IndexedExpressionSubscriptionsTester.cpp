#include <doctest/doctest.h>
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Queries/IndexedExpressionSubscriptions.hpp"
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
  using TestSubscriptions = IndexedExpressionSubscriptions<TestEntry, int,
    std::string, TestServiceProtocolClient>;

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

TEST_SUITE("IndexedExpressionSubscriptions") {
  TEST_CASE("publish_to_specific_index") {
    auto fixture = SingleClientFixture();
    auto& client = fixture.m_client;
    auto subscriptions = TestSubscriptions();
    auto filter = translate<TestTranslator>(ConstantExpression(true));
    auto expression = translate<TestTranslator>(MemberAccessExpression(
      "value", typeid(int), ParameterExpression(0, typeid(TestEntry))));
    auto query_id = 1;
    auto index_a = std::string("IndexA");
    subscriptions.init(index_a, client, query_id, Range::TOTAL,
      std::move(filter), ExpressionQuery::UpdatePolicy::ALL,
      std::move(expression));
    auto snapshot = std::vector<SequencedValue<TestEntry>>();
    auto result = QueryResult<SequencedValue<int>>();
    result.m_id = query_id;
    subscriptions.commit(index_a, client, SnapshotLimit::UNLIMITED, result,
      snapshot, [&] (QueryResult<SequencedValue<int>> committed_snapshot) {
        REQUIRE(committed_snapshot.m_id == result.m_id);
      });
    auto received_a = false;
    subscriptions.publish(SequencedValue(IndexedValue(
      TestEntry(100, time_from_string("2024-01-15 10:30:00")), index_a),
      Beam::Sequence(1)),
      [&] (auto& sender_client, int id, const SequencedValue<int>& value) {
        REQUIRE(&sender_client == &client);
        REQUIRE(id == query_id);
        REQUIRE(value.get_value() == 100);
        received_a = true;
      });
    REQUIRE(received_a);
    auto index_b = std::string("IndexB");
    subscriptions.publish(SequencedValue(IndexedValue(
      TestEntry(200, time_from_string("2024-01-15 10:30:01")), index_b),
      Beam::Sequence(2)),
      [&] (auto& sender_client, int id, const SequencedValue<int>& value) {
        REQUIRE(false);
      });
  }

  TEST_CASE("multiple_indexes_same_client") {
    auto fixture = SingleClientFixture();
    auto& client = fixture.m_client;
    auto subscriptions = TestSubscriptions();
    auto index_a = std::string("IndexA");
    auto filter_a = translate<TestTranslator>(ConstantExpression(true));
    auto expression_a = translate<TestTranslator>(ConstantExpression(100));
    auto query_id_a = 1;
    subscriptions.init(index_a, client, query_id_a, Range::TOTAL,
      std::move(filter_a), ExpressionQuery::UpdatePolicy::ALL,
      std::move(expression_a));
    auto index_b = std::string("IndexB");
    auto filter_b = translate<TestTranslator>(ConstantExpression(true));
    auto expression_b = translate<TestTranslator>(ConstantExpression(200));
    auto query_id_b = 2;
    subscriptions.init(index_b, client, query_id_b, Range::TOTAL,
      std::move(filter_b), ExpressionQuery::UpdatePolicy::ALL,
      std::move(expression_b));
    auto snapshot_a = std::vector<SequencedValue<TestEntry>>();
    auto result_a = QueryResult<SequencedValue<int>>();
    result_a.m_id = query_id_a;
    subscriptions.commit(index_a, client, SnapshotLimit::UNLIMITED, result_a,
      snapshot_a, [&] (QueryResult<SequencedValue<int>> committed_snapshot) {
        REQUIRE(committed_snapshot.m_id == result_a.m_id);
      });
    auto snapshot_b = std::vector<SequencedValue<TestEntry>>();
    auto result_b = QueryResult<SequencedValue<int>>();
    result_b.m_id = query_id_b;
    subscriptions.commit(index_b, client, SnapshotLimit::UNLIMITED, result_b,
      snapshot_b, [&] (QueryResult<SequencedValue<int>> committed_snapshot) {
        REQUIRE(committed_snapshot.m_id == result_b.m_id);
      });
    auto received_a = false;
    subscriptions.publish(SequencedValue(IndexedValue(
      TestEntry(50, time_from_string("2024-01-15 10:30:00")), index_a),
      Beam::Sequence(1)),
      [&] (auto& sender_client, int id, const SequencedValue<int>& value) {
        REQUIRE(&sender_client == &client);
        REQUIRE(id == query_id_a);
        REQUIRE(value.get_value() == 100);
        received_a = true;
      });
    REQUIRE(received_a);
    auto received_b = false;
    subscriptions.publish(SequencedValue(IndexedValue(
      TestEntry(75, time_from_string("2024-01-15 10:30:01")), index_b),
      Beam::Sequence(2)),
      [&] (auto& sender_client, int id, const SequencedValue<int>& value) {
        REQUIRE(&sender_client == &client);
        REQUIRE(id == query_id_b);
        REQUIRE(value.get_value() == 200);
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
    auto filter1 = translate<TestTranslator>(ConstantExpression(true));
    auto expression1 = translate<TestTranslator>(ConstantExpression(100));
    auto query_id1 = 1;
    subscriptions.init(index, client1, query_id1, Range::TOTAL,
      std::move(filter1), ExpressionQuery::UpdatePolicy::ALL,
      std::move(expression1));
    auto filter2 = translate<TestTranslator>(ConstantExpression(true));
    auto expression2 = translate<TestTranslator>(ConstantExpression(200));
    auto query_id2 = 2;
    subscriptions.init(index, client2, query_id2, Range::TOTAL,
      std::move(filter2), ExpressionQuery::UpdatePolicy::ALL,
      std::move(expression2));
    auto snapshot1 = std::vector<SequencedValue<TestEntry>>();
    auto result1 = QueryResult<SequencedValue<int>>();
    result1.m_id = query_id1;
    subscriptions.commit(index, client1, SnapshotLimit::UNLIMITED, result1,
      snapshot1, [&] (QueryResult<SequencedValue<int>> committed_snapshot) {
        REQUIRE(committed_snapshot.m_id == result1.m_id);
      });
    auto snapshot2 = std::vector<SequencedValue<TestEntry>>();
    auto result2 = QueryResult<SequencedValue<int>>();
    result2.m_id = query_id2;
    subscriptions.commit(index, client2, SnapshotLimit::UNLIMITED, result2,
      snapshot2, [&] (QueryResult<SequencedValue<int>> committed_snapshot) {
        REQUIRE(committed_snapshot.m_id == result2.m_id);
      });
    auto client1_received = false;
    auto client2_received = false;
    subscriptions.publish(SequencedValue(IndexedValue(
      TestEntry(50, time_from_string("2024-01-15 10:30:00")), index),
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
    auto filter1 = translate<TestTranslator>(ConstantExpression(true));
    auto expression1 = translate<TestTranslator>(ConstantExpression(100));
    auto query_id1 = 1;
    subscriptions.init(index_a, client1, query_id1, Range::TOTAL,
      std::move(filter1), ExpressionQuery::UpdatePolicy::ALL,
      std::move(expression1));
    auto filter2 = translate<TestTranslator>(ConstantExpression(true));
    auto expression2 = translate<TestTranslator>(ConstantExpression(200));
    auto query_id2 = 2;
    subscriptions.init(index_a, client2, query_id2, Range::TOTAL,
      std::move(filter2), ExpressionQuery::UpdatePolicy::ALL,
      std::move(expression2));
    auto index_b = std::string("IndexB");
    auto filter3 = translate<TestTranslator>(ConstantExpression(true));
    auto expression3 = translate<TestTranslator>(ConstantExpression(300));
    auto query_id3 = 3;
    subscriptions.init(index_b, client1, query_id3, Range::TOTAL,
      std::move(filter3), ExpressionQuery::UpdatePolicy::ALL,
      std::move(expression3));
    auto snapshot1 = std::vector<SequencedValue<TestEntry>>();
    auto result1 = QueryResult<SequencedValue<int>>();
    result1.m_id = query_id1;
    subscriptions.commit(index_a, client1, SnapshotLimit::UNLIMITED, result1,
      snapshot1, [&] (QueryResult<SequencedValue<int>> committed_snapshot) {
        REQUIRE(committed_snapshot.m_id == result1.m_id);
      });
    auto snapshot2 = std::vector<SequencedValue<TestEntry>>();
    auto result2 = QueryResult<SequencedValue<int>>();
    result2.m_id = query_id2;
    subscriptions.commit(index_a, client2, SnapshotLimit::UNLIMITED, result2,
      snapshot2, [&] (QueryResult<SequencedValue<int>> committed_snapshot) {
        REQUIRE(committed_snapshot.m_id == result2.m_id);
      });
    auto snapshot3 = std::vector<SequencedValue<TestEntry>>();
    auto result3 = QueryResult<SequencedValue<int>>();
    result3.m_id = query_id3;
    subscriptions.commit(index_b, client1, SnapshotLimit::UNLIMITED, result3,
      snapshot3, [&] (QueryResult<SequencedValue<int>> committed_snapshot) {
        REQUIRE(committed_snapshot.m_id == result3.m_id);
      });
    auto client1_count = 0;
    auto client2_count = 0;
    subscriptions.publish(SequencedValue(IndexedValue(
      TestEntry(50, time_from_string("2024-01-15 10:30:00")), index_a),
      Beam::Sequence(1)),
      [&] (auto& sender_client, int id, const SequencedValue<int>& value) {
        if(&sender_client == &client1) {
          REQUIRE(id == query_id1);
          REQUIRE(value.get_value() == 100);
          ++client1_count;
        } else if(&sender_client == &client2) {
          REQUIRE(id == query_id2);
          REQUIRE(value.get_value() == 200);
          ++client2_count;
        }
      });
    REQUIRE(client1_count == 1);
    REQUIRE(client2_count == 1);
    client1_count = 0;
    client2_count = 0;
    subscriptions.publish(SequencedValue(IndexedValue(
      TestEntry(75, time_from_string("2024-01-15 10:30:01")), index_b),
      Beam::Sequence(2)),
      [&] (auto& sender_client, int id, const SequencedValue<int>& value) {
        REQUIRE(&sender_client == &client1);
        REQUIRE(id == query_id3);
        REQUIRE(value.get_value() == 300);
        ++client1_count;
      });
    REQUIRE(client1_count == 1);
    REQUIRE(client2_count == 0);
  }

  TEST_CASE("end_subscription_preserves_other_indexes") {
    auto fixture = SingleClientFixture();
    auto& client = fixture.m_client;
    auto subscriptions = TestSubscriptions();
    auto index_a = std::string("IndexA");
    auto filter_a = translate<TestTranslator>(ConstantExpression(true));
    auto expression_a = translate<TestTranslator>(ConstantExpression(100));
    auto query_id_a = 1;
    subscriptions.init(index_a, client, query_id_a, Range::TOTAL,
      std::move(filter_a), ExpressionQuery::UpdatePolicy::ALL,
      std::move(expression_a));
    auto index_b = std::string("IndexB");
    auto filter_b = translate<TestTranslator>(ConstantExpression(true));
    auto expression_b = translate<TestTranslator>(ConstantExpression(200));
    auto query_id_b = 2;
    subscriptions.init(index_b, client, query_id_b, Range::TOTAL,
      std::move(filter_b), ExpressionQuery::UpdatePolicy::ALL,
      std::move(expression_b));
    auto snapshot_a = std::vector<SequencedValue<TestEntry>>();
    auto result_a = QueryResult<SequencedValue<int>>();
    result_a.m_id = query_id_a;
    subscriptions.commit(index_a, client, SnapshotLimit::UNLIMITED, result_a,
      snapshot_a, [&] (QueryResult<SequencedValue<int>> committed_snapshot) {
        REQUIRE(committed_snapshot.m_id == result_a.m_id);
      });
    auto snapshot_b = std::vector<SequencedValue<TestEntry>>();
    auto result_b = QueryResult<SequencedValue<int>>();
    result_b.m_id = query_id_b;
    subscriptions.commit(index_b, client, SnapshotLimit::UNLIMITED, result_b,
      snapshot_b, [&] (QueryResult<SequencedValue<int>> committed_snapshot) {
        REQUIRE(committed_snapshot.m_id == result_b.m_id);
      });
    subscriptions.end(client, query_id_a);
    subscriptions.publish(SequencedValue(IndexedValue(
      TestEntry(50, time_from_string("2024-01-15 10:30:00")), index_a),
      Beam::Sequence(1)),
      [&] (auto& sender_client, int id, const SequencedValue<int>& value) {
        REQUIRE(false);
      });
    auto received_b = false;
    subscriptions.publish(SequencedValue(IndexedValue(
      TestEntry(75, time_from_string("2024-01-15 10:30:01")), index_b),
      Beam::Sequence(2)),
      [&] (auto& sender_client, int id, const SequencedValue<int>& value) {
        REQUIRE(&sender_client == &client);
        REQUIRE(id == query_id_b);
        REQUIRE(value.get_value() == 200);
        received_b = true;
      });
    REQUIRE(received_b);
  }
}
