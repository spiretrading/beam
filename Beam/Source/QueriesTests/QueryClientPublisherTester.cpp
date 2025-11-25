#include <doctest/doctest.h>
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queries/QueryClientPublisher.hpp"
#include "Beam/Queries/QueryResult.hpp"
#include "Beam/Queries/ShuttleQueryTypes.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/QueriesTests/TestEntry.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/Services/ServiceProtocolClientBuilder.hpp"
#include "Beam/Services/ServiceProtocolClientHandler.hpp"
#include "Beam/TimeService/TriggerTimer.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;

namespace {
  using TestQuery = BasicQuery<std::string>;
  using ClientServiceProtocolClient = ServiceProtocolClient<MessageProtocol<
    std::unique_ptr<LocalClientChannel>, BinarySender<SharedBuffer>,
      NullEncoder>, TriggerTimer>;
  using ServerServiceProtocolClient = ServiceProtocolClient<MessageProtocol<
    std::unique_ptr<LocalServerChannel>, BinarySender<SharedBuffer>,
      NullEncoder>, TriggerTimer>;
  using TestServiceProtocolClientBuilder = ServiceProtocolClientBuilder<
    ClientServiceProtocolClient::MessageProtocol, TriggerTimer>;

  BEAM_DEFINE_SERVICES(test_query_services,
    (QueryService, "query_service", QueryResult<SequencedTestEntry>,
      (TestQuery, query)));

  BEAM_DEFINE_MESSAGES(test_query_messages,
    (EndQueryMessage, "end_query_message", (std::string, index), (int, id)));

  using TestQueryClientPublisher = QueryClientPublisher<
    TestEntry, TestQuery, TestTranslator, ServiceProtocolClientHandler<
      TestServiceProtocolClientBuilder>, QueryService, EndQueryMessage>;

  struct Fixture {
    LocalServerConnection m_server;
    std::atomic_int m_channel_id;
    optional<ServerServiceProtocolClient> m_server_client;
    RoutineHandler m_accept_routine;
    ServiceProtocolClientHandler<TestServiceProtocolClientBuilder>
      m_client_handler;
    TestQueryClientPublisher m_publisher;

    Fixture()
      : m_channel_id(1),
        m_accept_routine(spawn([&] {
          m_server_client.emplace(m_server.accept(), init());
          register_query_types(
            out(m_server_client->get_slots().get_registry()));
          register_test_query_services(out(m_server_client->get_slots()));
          register_test_query_messages(out(m_server_client->get_slots()));
        })),
        m_client_handler(init(
          [&] {
            auto id = m_channel_id++;
            return std::make_unique<LocalClientChannel>(
              "test" + std::to_string(id), m_server);
          },
          [] {
            return std::make_unique<TriggerTimer>();
          })),
        m_publisher(Ref(m_client_handler)) {
      register_query_types(
        out(m_client_handler.get_slots().get_registry()));
      register_test_query_services(out(m_client_handler.get_slots()));
      register_test_query_messages(out(m_client_handler.get_slots()));
      m_accept_routine.wait();
    }
  };
}

TEST_SUITE("QueryClientPublisher") {
  TEST_CASE("submit_realtime_query") {
    auto fixture = Fixture();
    auto query = TestQuery();
    query.set_index("IndexA");
    query.set_range(Range::TOTAL);
    auto& client = *fixture.m_server_client;
    QueryService::add_slot(out(client.get_slots()),
      [&] (auto& protocol_client, const TestQuery& received_query) {
        REQUIRE(received_query.get_index() == query.get_index());
        REQUIRE(received_query.get_range() == query.get_range());
        auto result = QueryResult<SequencedTestEntry>();
        result.m_id = 1;
        result.m_snapshot.push_back(SequencedValue(
          TestEntry(100, time_from_string("2024-01-15 10:30:00")),
          Beam::Sequence(1)));
        return result;
      });
    client.spawn_message_handler();
    auto queue = std::make_shared<Queue<SequencedTestEntry>>();
    fixture.m_publisher.submit(query, queue);
    auto value = queue->pop();
    REQUIRE(value.get_value().m_value == 100);
    REQUIRE(value.get_sequence() == Beam::Sequence(1));
  }

  TEST_CASE("submit_historical_query") {
    auto fixture = Fixture();
    auto query = TestQuery();
    query.set_index("IndexA");
    query.set_range(Range(Beam::Sequence(1), Beam::Sequence(10)));
    auto& client = *fixture.m_server_client;
    QueryService::add_slot(out(client.get_slots()),
      [&] (auto& protocol_client, const TestQuery& received_query) {
        REQUIRE(received_query.get_index() == query.get_index());
        REQUIRE(received_query.get_range() == query.get_range());
        auto result = QueryResult<SequencedTestEntry>();
        result.m_id = -1;
        result.m_snapshot.push_back(SequencedValue(
          TestEntry(200, time_from_string("2024-01-15 10:30:00")),
          Beam::Sequence(5)));
        return result;
      });
    client.spawn_message_handler();
    auto queue = std::make_shared<Queue<SequencedTestEntry>>();
    auto queue_closed = false;
    fixture.m_publisher.submit(query, queue);
    auto value = queue->pop();
    REQUIRE(value.get_value().m_value == 200);
    REQUIRE(value.get_sequence() == Beam::Sequence(5));
    REQUIRE_THROWS_AS(queue->pop(), PipeBrokenException);
  }

  TEST_CASE("submit_with_value_queue") {
    auto fixture = Fixture();
    auto& client = *fixture.m_server_client;
    QueryService::add_slot(out(client.get_slots()),
      [] (auto& protocol_client, const TestQuery& received_query) {
        auto result = QueryResult<SequencedTestEntry>();
        result.m_id = 7;
        result.m_snapshot.push_back(SequencedValue(
          TestEntry(400, time_from_string("2024-01-15 10:30:00")),
          Beam::Sequence(3)));
        return result;
      });
    client.spawn_message_handler();
    auto query = TestQuery();
    query.set_index("IndexC");
    query.set_range(Range::TOTAL);
    auto queue = std::make_shared<Queue<TestEntry>>();
    fixture.m_publisher.submit(query, queue);
    auto value = queue->pop();
    REQUIRE(value.m_value == 400);
  }

  TEST_CASE("publish_to_matching_index") {
    auto fixture = Fixture();
    auto& client = *fixture.m_server_client;
    QueryService::add_slot(out(client.get_slots()),
      [] (auto& protocol_client, const TestQuery& received_query) {
        auto result = QueryResult<SequencedTestEntry>();
        result.m_id = 10;
        result.m_snapshot.push_back(SequencedValue(
          TestEntry(500, time_from_string("2024-01-15 10:30:00")),
          Beam::Sequence(1)));
        return result;
      });
    client.spawn_message_handler();
    auto query = TestQuery();
    query.set_index("IndexD");
    query.set_range(Range::TOTAL);
    auto queue = std::make_shared<Queue<SequencedTestEntry>>();
    fixture.m_publisher.submit(query, queue);
    auto snapshot_value = queue->pop();
    REQUIRE(snapshot_value.get_value().m_value == 500);
    fixture.m_publisher.publish(SequencedValue(IndexedValue(
      TestEntry(600, time_from_string("2024-01-15 10:30:01")),
      std::string("IndexD")), Beam::Sequence(2)));
    auto published_value = queue->pop();
    REQUIRE(published_value.get_value().m_value == 600);
    REQUIRE(published_value.get_sequence() == Beam::Sequence(2));
    fixture.m_publisher.publish(SequencedValue(IndexedValue(
      TestEntry(700, time_from_string("2024-01-15 10:30:02")),
      std::string("IndexE")), Beam::Sequence(3)));
    REQUIRE(!queue->try_pop());
  }

  TEST_CASE("publish_removes_broken_publisher") {
    auto fixture = Fixture();
    auto end_query_received = false;
    auto& client = *fixture.m_server_client;
    QueryService::add_slot(out(client.get_slots()),
      [] (auto& protocol_client, const TestQuery& received_query) {
        auto result = QueryResult<SequencedTestEntry>();
        result.m_id = 15;
        result.m_snapshot.push_back(SequencedValue(
          TestEntry(800, time_from_string("2024-01-15 10:30:00")),
          Beam::Sequence(1)));
        return result;
      });
    add_message_slot<EndQueryMessage>(out(client.get_slots()),
      [&] (auto& sender, const std::string& index, int id) {
        REQUIRE(index == "IndexF");
        REQUIRE(id == 15);
        end_query_received = true;
      });
    client.spawn_message_handler();
    auto query = TestQuery();
    query.set_index("IndexF");
    query.set_range(Range::TOTAL);
    auto queue = std::make_shared<Queue<SequencedTestEntry>>();
    fixture.m_publisher.submit(query, queue);
    auto snapshot_value = queue->pop();
    REQUIRE(snapshot_value.get_value().m_value == 800);
    queue->close();
    fixture.m_publisher.publish(SequencedValue(IndexedValue(
      TestEntry(900, time_from_string("2024-01-15 10:30:01")),
      std::string("IndexF")), Beam::Sequence(2)));
    flush_pending_routines();
    REQUIRE(end_query_received);
    fixture.m_publisher.publish(SequencedValue(IndexedValue(
      TestEntry(1000, time_from_string("2024-01-15 10:30:02")),
      std::string("IndexF")), Beam::Sequence(3)));
  }

  TEST_CASE("close_breaks_all_queues") {
    auto fixture = Fixture();
    auto& client = *fixture.m_server_client;
    QueryService::add_slot(out(client.get_slots()),
      [] (auto& protocol_client, const TestQuery& received_query) {
        auto result = QueryResult<SequencedTestEntry>();
        if(received_query.get_index() == "IndexH") {
          result.m_id = 25;
        } else {
          result.m_id = 26;
        }
        result.m_snapshot.push_back(SequencedValue(
          TestEntry(1400, time_from_string("2024-01-15 10:30:00")),
          Beam::Sequence(1)));
        return result;
      });
    client.spawn_message_handler();
    auto query1 = TestQuery();
    query1.set_index("IndexH");
    query1.set_range(Range::TOTAL);
    auto queue1 = std::make_shared<Queue<SequencedTestEntry>>();
    auto query2 = TestQuery();
    query2.set_index("IndexI");
    query2.set_range(Range::TOTAL);
    auto queue2 = std::make_shared<Queue<SequencedTestEntry>>();
    fixture.m_publisher.submit(query1, queue1);
    fixture.m_publisher.submit(query2, queue2);
    auto value1 = queue1->pop();
    REQUIRE(value1.get_value().m_value == 1400);
    auto value2 = queue2->pop();
    REQUIRE(value2.get_value().m_value == 1400);
    fixture.m_publisher.close();
    REQUIRE_THROWS_AS(queue1->pop(), PipeBrokenException);
    REQUIRE_THROWS_AS(queue2->pop(), PipeBrokenException);
    auto queue3 = std::make_shared<Queue<SequencedTestEntry>>();
    auto query3 = TestQuery();
    query3.set_index("IndexJ");
    query3.set_range(Range::TOTAL);
    fixture.m_publisher.submit(query3, queue3);
    REQUIRE_THROWS_AS(queue3->pop(), PipeBrokenException);
  }
}
