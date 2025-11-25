#include <doctest/doctest.h>
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Routines/RoutineHandler.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/ServicesTests/TestServices.hpp"
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

  void on_void_request(
      RequestToken<ServerServiceProtocolClient, VoidService>& request, int n,
      int* callback_count) {
    ++*callback_count;
    request.set();
  }

  void on_exception_void_request(
      RequestToken<ServerServiceProtocolClient, VoidService>& request, int n,
      int* callback_count) {
    ++*callback_count;
    request.set_exception(ServiceRequestException());
  }
}

TEST_SUITE("ServiceProtocolClient") {
  TEST_CASE("void_return_type_request_slot") {
    auto server = LocalServerConnection();
    auto callback_count = 0;
    auto server_task = RoutineHandler(spawn([&] {
      auto client = ServerServiceProtocolClient(server.accept(), init());
      register_test_services(out(client.get_slots()));
      VoidService::add_request_slot(out(client.get_slots()),
        [&] (auto& request, auto n) {
          on_void_request(request, n, &callback_count);
        });
      try {
        while(true) {
          auto message = client.read_message();
          if(auto slot = client.get_slots().find(*message)) {
            message->emit(slot, Ref(client));
          }
        }
      } catch(const ServiceRequestException&) {
      } catch(const EndOfFileException&) {
      }
    }));
    auto client = ClientServiceProtocolClient(init("client", server), init());
    register_test_services(out(client.get_slots()));
    client.send_request<VoidService>(123);
    client.send_request<VoidService>(321);
    client.close();
    server_task.wait();
    REQUIRE(callback_count == 2);
  }

  TEST_CASE("void_return_type_slot") {
    auto server = LocalServerConnection();
    auto callback_count = 0;
    auto server_task = RoutineHandler(spawn([&] {
      auto client = ServerServiceProtocolClient(server.accept(), init());
      register_test_services(out(client.get_slots()));
      VoidService::add_slot(out(client.get_slots()),
        [&] (auto& client, auto n) {
          ++callback_count;
        });
      try {
        while(true) {
          auto message = client.read_message();
          if(auto slot = client.get_slots().find(*message)) {
            message->emit(slot, Ref(client));
          }
        }
      } catch(const ServiceRequestException&) {
      } catch(const EndOfFileException&) {
      }
    }));
    auto client = ClientServiceProtocolClient(init("client", server), init());
    register_test_services(out(client.get_slots()));
    client.send_request<VoidService>(123);
    client.send_request<VoidService>(321);
    client.close();
    server_task.wait();
    REQUIRE(callback_count == 2);
  }

  TEST_CASE("addition_service") {
    auto server = LocalServerConnection();
    auto server_task = RoutineHandler(spawn([&] {
      auto client = ServerServiceProtocolClient(server.accept(), init());
      register_test_services(out(client.get_slots()));
      AdditionService::add_slot(out(client.get_slots()),
        [&] (auto& client, auto a, auto b) {
          return a + b;
        });
      try {
        while(true) {
          auto message = client.read_message();
          if(auto slot = client.get_slots().find(*message)) {
            message->emit(slot, Ref(client));
          }
        }
      } catch(const ServiceRequestException&) {
      } catch(const EndOfFileException&) {
      }
    }));
    auto client = ClientServiceProtocolClient(init("client", server), init());
    register_test_services(out(client.get_slots()));
    auto sum = client.send_request<AdditionService>(123, 456);
    REQUIRE(sum == 579);
    client.close();
    server_task.wait();
  }

  TEST_CASE("exception") {
    auto server = LocalServerConnection();
    auto callback_count = 0;
    auto server_task = RoutineHandler(spawn([&] {
      auto client = ServerServiceProtocolClient(server.accept(), init());
      register_test_services(out(client.get_slots()));
      VoidService::add_request_slot(out(client.get_slots()),
        [&] (auto& request, auto n) {
          on_exception_void_request(request, n, &callback_count);
        });
      try {
        while(true) {
          auto message = client.read_message();
          if(auto slot = client.get_slots().find(*message)) {
            message->emit(slot, Ref(client));
          }
        }
      } catch(const ServiceRequestException&) {
      } catch(const EndOfFileException&) {
      }
    }));
    auto client = ClientServiceProtocolClient(init("client", server), init());
    register_test_services(out(client.get_slots()));
    REQUIRE_THROWS_AS(
      client.send_request<VoidService>(123), ServiceRequestException);
    client.close();
    server_task.wait();
    REQUIRE(callback_count == 1);
  }

  TEST_CASE("request_before_connection_closed") {
    auto server = LocalServerConnection();
    auto callback_count = 0;
    auto server_task = RoutineHandler(spawn([&] {
      auto client = ServerServiceProtocolClient(server.accept(), init());
      register_test_services(out(client.get_slots()));
      VoidService::add_request_slot(out(client.get_slots()),
        [&] (auto& request, auto n) {
          on_exception_void_request(request, n, &callback_count);
        });
      try {
        while(true) {
          auto message = client.read_message();
          if(auto slot = client.get_slots().find(*message)) {
            client.close();
          }
        }
      } catch(const ServiceRequestException&) {
      } catch(const EndOfFileException&) {
      }
    }));
    auto client = ClientServiceProtocolClient(init("client", server), init());
    register_test_services(out(client.get_slots()));
    REQUIRE_THROWS_AS(
      client.send_request<VoidService>(123), ServiceRequestException);
    client.close();
    server_task.wait();
  }

  TEST_CASE("request_after_connection_closed") {
    auto server = LocalServerConnection();
    auto server_task = RoutineHandler(spawn([&] {
      auto client_channel = server.accept();
      client_channel->get_connection().close();
    }));
    auto client = ClientServiceProtocolClient(init("client", server), init());
    register_test_services(out(client.get_slots()));
    client.close();
    REQUIRE_THROWS_AS(
      client.send_request<VoidService>(123), EndOfFileException);
    client.close();
    server_task.wait();
  }
}
