#include <doctest/doctest.h>
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Routines/RoutineHandler.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"
#include "Beam/Services/RecordMessage.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/TimeService/TriggerTimer.hpp"

using namespace Beam;
using namespace Beam::Tests;

namespace {
  using ServerServiceProtocolClient = ServiceProtocolClient<
    MessageProtocol<std::unique_ptr<LocalServerConnection::Channel>,
      BinarySender<SharedBuffer>, NullEncoder>, TriggerTimer>;
  using ClientServiceProtocolClient = ServiceProtocolClient<
    MessageProtocol<LocalClientChannel, BinarySender<SharedBuffer>,
      NullEncoder>, TriggerTimer>;

  BEAM_DEFINE_MESSAGES(test_messages,
    (SimpleMessage, "Test.SimpleMessage", (int, value)),
    (MultiFieldMessage, "Test.MultiFieldMessage", (int, id),
      (std::string, name), (double, amount)));

  BEAM_DEFINE_RECORD(ComplexRecord, (int, id), (std::string, data));
}

TEST_SUITE("RecordMessage") {
  TEST_CASE("constructor") {
    auto message =
      RecordMessage<SimpleMessage, ClientServiceProtocolClient>(123);
    REQUIRE(message.get_record().value == 123);
  }

  TEST_CASE("multi_field_construction") {
    auto message =
      RecordMessage<MultiFieldMessage, ClientServiceProtocolClient>(
        42, "test", 3.14);
    REQUIRE(message.get_record().id == 42);
    REQUIRE(message.get_record().name == "test");
    REQUIRE(message.get_record().amount == 3.14);
  }

  TEST_CASE("send_record_message") {
    auto server = LocalServerConnection();
    auto received_count = 0;
    auto receive_token = Async<void>();
    auto server_task = RoutineHandler(spawn([&] {
      auto client = ServerServiceProtocolClient(server.accept(), init());
      register_test_messages(out(client.get_slots()));
      add_message_slot<SimpleMessage>(out(client.get_slots()),
        [&] (auto& protocol, auto value) {
          ++received_count;
          REQUIRE(value == 777);
          receive_token.get_eval().set();
        });
      try {
        while(true) {
          auto message = client.read_message();
          if(auto slot = client.get_slots().find(*message)) {
            message->emit(slot, Ref(client));
          }
        }
      } catch(const EndOfFileException&) {
      }
    }));
    auto client = ClientServiceProtocolClient(init("client", server), init());
    register_test_messages(out(client.get_slots()));
    send_record_message<SimpleMessage>(client, 777);
    receive_token.get();
    client.close();
    server_task.wait();
    REQUIRE(received_count == 1);
  }

  TEST_CASE("add_message_slot") {
    auto server = LocalServerConnection();
    auto receive_token = Async<void>();
    auto server_task = RoutineHandler(spawn([&] {
      auto client = ServerServiceProtocolClient(server.accept(), init());
      register_test_messages(out(client.get_slots()));
      add_message_slot<MultiFieldMessage>(out(client.get_slots()),
        [&] (auto& protocol, auto id, auto name, auto amount) {
          REQUIRE(id == 321);
          REQUIRE(name == "slot_test");
          REQUIRE(amount == 1.618);
          receive_token.get_eval().set();
        });
      try {
        while(true) {
          auto message = client.read_message();
          if(auto slot = client.get_slots().find(*message)) {
            message->emit(slot, Ref(client));
          }
        }
      } catch(const EndOfFileException&) {
      }
    }));
    auto client = ClientServiceProtocolClient(init("client", server), init());
    register_test_messages(out(client.get_slots()));
    send_record_message<MultiFieldMessage>(client, 321, "slot_test", 1.618);
    client.close();
    server_task.wait();
  }

  TEST_CASE("broadcast_record_message_empty_list") {
    auto clients = std::vector<ServerServiceProtocolClient*>();
    REQUIRE_NOTHROW(broadcast_record_message<SimpleMessage>(clients, 100));
  }

  TEST_CASE("broadcast_record_message_single_client") {
    auto server = LocalServerConnection();
    auto received_count = 0;
    auto receive_token = Async<void>();
    auto server_task = RoutineHandler(spawn([&] {
      auto client = ServerServiceProtocolClient(server.accept(), init());
      register_test_messages(out(client.get_slots()));
      add_message_slot<SimpleMessage>(out(client.get_slots()),
        [&] (auto& protocol, auto value) {
          ++received_count;
          REQUIRE(value == 555);
          receive_token.get_eval().set();
        });
      try {
        while(true) {
          auto message = client.read_message();
          if(auto slot = client.get_slots().find(*message)) {
            message->emit(slot, Ref(client));
          }
        }
      } catch(const EndOfFileException&) {
      }
    }));
    auto client = ClientServiceProtocolClient(init("client", server), init());
    register_test_messages(out(client.get_slots()));
    auto clients = std::vector<ClientServiceProtocolClient*>();
    clients.push_back(&client);
    broadcast_record_message<SimpleMessage>(clients, 555);
    receive_token.get();
    client.close();
    server_task.wait();
    REQUIRE(received_count == 1);
  }

  TEST_CASE("broadcast_record_message_multiple_clients") {
    auto server1 = LocalServerConnection();
    auto server2 = LocalServerConnection();
    auto received_count1 = 0;
    auto receive_token1 = Async<void>();
    auto received_count2 = 0;
    auto receive_token2 = Async<void>();
    auto server_task1 = RoutineHandler(spawn([&] {
      auto client = ServerServiceProtocolClient(server1.accept(), init());
      register_test_messages(out(client.get_slots()));
      add_message_slot<SimpleMessage>(out(client.get_slots()),
        [&] (auto& protocol, auto value) {
          ++received_count1;
          REQUIRE(value == 888);
          receive_token1.get_eval().set();
        });
      try {
        while(true) {
          auto message = client.read_message();
          if(auto slot = client.get_slots().find(*message)) {
            message->emit(slot, Ref(client));
          }
        }
      } catch(const EndOfFileException&) {
      }
    }));
    auto server_task2 = RoutineHandler(spawn([&] {
      auto client = ServerServiceProtocolClient(server2.accept(), init());
      register_test_messages(out(client.get_slots()));
      add_message_slot<SimpleMessage>(out(client.get_slots()),
        [&] (auto& protocol, auto value) {
          ++received_count2;
          REQUIRE(value == 888);
          receive_token2.get_eval().set();
        });
      try {
        while(true) {
          auto message = client.read_message();
          if(auto slot = client.get_slots().find(*message)) {
            message->emit(slot, Ref(client));
          }
        }
      } catch(const EndOfFileException&) {
      }
    }));
    auto client1 =
      ClientServiceProtocolClient(init("client1", server1), init());
    register_test_messages(out(client1.get_slots()));
    auto client2 =
      ClientServiceProtocolClient(init("client2", server2), init());
    register_test_messages(out(client2.get_slots()));
    auto clients = std::vector<ClientServiceProtocolClient*>();
    clients.push_back(&client1);
    clients.push_back(&client2);
    broadcast_record_message<SimpleMessage>(clients, 888);
    receive_token1.get();
    receive_token2.get();
    client1.close();
    client2.close();
    server_task1.wait();
    server_task2.wait();
    REQUIRE(received_count1 == 1);
    REQUIRE(received_count2 == 1);
  }

  TEST_CASE("stream") {
    test_round_trip_shuttle(
      RecordMessage<MultiFieldMessage, ClientServiceProtocolClient>(
        100, "serialized", 2.718), [&] (auto&& received) {
      REQUIRE(received.get_record().id == 100);
      REQUIRE(received.get_record().name == "serialized");
      REQUIRE(received.get_record().amount == 2.718);
    });
  }
}
