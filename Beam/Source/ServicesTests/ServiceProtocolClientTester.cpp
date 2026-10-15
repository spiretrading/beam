#include <sstream>
#include <doctest/doctest.h>
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/IO/BasicChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/NullChannel.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Network/SocketIdentifier.hpp"
#include "Beam/Routines/RoutineHandler.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/ServicesTests/TestServices.hpp"
#include "Beam/SignalHandling/NullSlot.hpp"
#include "Beam/TimeService/TriggerTimer.hpp"

using namespace Beam;
using namespace Beam::Tests;

namespace {
  template<typename F>
  std::string capture_stderr(F function) {
    auto stream = std::ostringstream();
    auto buffer = std::cerr.rdbuf(stream.rdbuf());
    try {
      function();
      std::cerr.rdbuf(buffer);
      return stream.str();
    } catch(...) {
      std::cerr.rdbuf(buffer);
      throw;
    }
  }

  void throw_handler_exception() {
    try {
      boost::throw_with_location(std::runtime_error("inner failure"));
    } catch(...) {
      throw_nested_with_location(std::runtime_error("handler failure"));
    }
  }

  BEAM_DEFINE_RECORD(LoggingRecord, (int, value));

  template<typename C>
  class FailingMessage : public ServiceMessage<C> {
    public:
      class Slot : public ServiceSlot<FailingMessage> {
        public:
          void add_pre_hook(
            const typename ServiceSlot<FailingMessage>::PreHook&) override {}
      };

      int get_id() const override {
        return 17;
      }

      bool is_response() const override {
        return false;
      }

      void emit(BaseServiceSlot<C>*, Ref<C>) const override {
        throw_handler_exception();
      }

    private:
      friend struct Beam::DataShuttle;

      template<IsShuttle S>
      void shuttle(S&, unsigned int) {}
  };

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
  TEST_CASE_TEMPLATE(
      "request_diagnostics", T, std::false_type, std::true_type) {
    using Server =
      ServiceProtocolServer<LocalServerConnection*, BinarySender<SharedBuffer>,
      NullEncoder, std::unique_ptr<TriggerTimer>, NullSession, T::value>;
    auto operation = std::string("handle_request");
    auto output = capture_stderr([&] {
      auto connection = LocalServerConnection();
      auto server = Server(&connection, [] {
        return std::make_unique<TriggerTimer>();
      }, NullSlot(), NullSlot());
      auto& slots = server.get_slots();
      register_test_services(out(slots));
      SUBCASE("ordinary_handler") {
        IdentityService::add_slot(out(slots), [] (auto&, auto) -> int {
          throw_handler_exception();
          return 0;
        });
      }
      SUBCASE("request_handler") {
        IdentityService::add_request_slot(out(slots), [] (auto&, auto) {
          throw_handler_exception();
        });
      }
      SUBCASE("pre_hook") {
        operation = "pre_hook";
        IdentityService::add_slot(out(slots), [] (auto&, auto n) { return n; });
        slots.apply([] (const auto&, auto& slot) {
          slot.add_pre_hook([] (auto&) { throw_handler_exception(); });
        });
      }
      auto client =
        ClientServiceProtocolClient(init("peer", connection), init());
      register_test_services(out(client.get_slots()));
      REQUIRE_THROWS_WITH_AS(client.send_request<IdentityService>(7),
        "handler failure", ServiceRequestException);
      client.close();
      server.close();
    });
    REQUIRE(output.contains("client=peer operation=" + operation +
      " message=Beam.Services.Tests.IdentityService.Request request=2\n"));
    REQUIRE(output.contains("handler failure"));
    REQUIRE(output.contains("inner failure"));
    REQUIRE(output.contains("thrown at: "));
    auto position = output.find("Uncaught exception thrown:");
    REQUIRE(position != std::string::npos);
    REQUIRE(output.find("Uncaught exception thrown:", position + 1) ==
      std::string::npos);
  }

  TEST_CASE("record_message_diagnostics") {
    using Client = ServiceProtocolClient<MessageProtocol<
      NullChannel, BinarySender<SharedBuffer>, NullEncoder>, TriggerTimer>;
    using Message = RecordMessage<LoggingRecord, Client>;
    auto operation = std::string("handle_message");
    auto output = capture_stderr([&] {
      auto client = Client(init(NamedChannelIdentifier("peer")), init());
      auto& slots = client.get_slots();
      slots.get_registry().add<Message>("Test.LoggingRecord");
      SUBCASE("handler") {
        add_message_slot<LoggingRecord>(out(slots), [] (auto&, auto) {
          throw_handler_exception();
        });
      }
      SUBCASE("pre_hook") {
        operation = "pre_hook";
        add_message_slot<LoggingRecord>(out(slots), [] (auto&, auto) {});
        slots.apply([] (const auto&, auto& slot) {
          slot.add_pre_hook([] (auto&) { throw_handler_exception(); });
        });
      }
      auto message = Message(7);
      REQUIRE_NOTHROW(message.emit(slots.find(message), Ref(client)));
    });
    REQUIRE(output.contains("client=peer operation=" + operation +
      " message=Test.LoggingRecord\n"));
    REQUIRE(!output.contains("request="));
    REQUIRE(output.contains("handler failure"));
    REQUIRE(output.contains("inner failure"));
    auto position = output.find("Uncaught exception thrown:");
    REQUIRE(position != std::string::npos);
    REQUIRE(output.find("Uncaught exception thrown:", position + 1) ==
      std::string::npos);
  }

  TEST_CASE_TEMPLATE(
      "dispatch_diagnostics", T, std::false_type, std::true_type) {
    using Server = ServiceProtocolServer<LocalServerConnection*,
      BinarySender<SharedBuffer>, NullEncoder, std::unique_ptr<TriggerTimer>,
      NullSession, T::value>;
    using ServerMessage =
      FailingMessage<typename Server::ServiceProtocolClient>;
    using ClientMessage = FailingMessage<ClientServiceProtocolClient>;
    auto output = capture_stderr([&] {
      auto connection = LocalServerConnection();
      auto closed = Async<void>();
      auto server = Server(&connection, [] {
        return std::make_unique<TriggerTimer>();
      }, NullSlot(), [&] (auto&) { closed.get_eval().set(); });
      auto& slots = server.get_slots();
      slots.get_registry().template add<ServerMessage>("Test.FailingMessage");
      slots.add(std::make_unique<typename ServerMessage::Slot>());
      auto client =
        ClientServiceProtocolClient(init("peer", connection), init());
      client.get_slots().get_registry().add<ClientMessage>(
        "Test.FailingMessage");
      client.send(ClientMessage());
      closed.get();
      client.close();
      server.close();
    });
    REQUIRE(output.contains("client=peer operation=dispatch_message "
      "message=Test.FailingMessage request=17\n"));
    REQUIRE(output.contains("handler failure"));
    REQUIRE(output.contains("inner failure"));
    auto position = output.find("Uncaught exception thrown:");
    REQUIRE(position != std::string::npos);
    REQUIRE(output.find("Uncaught exception thrown:", position + 1) ==
      std::string::npos);
  }

  TEST_CASE("expected_failures_are_quiet") {
    using Client = ServiceProtocolClient<MessageProtocol<
      NullChannel, BinarySender<SharedBuffer>, NullEncoder>, TriggerTimer>;
    auto output = capture_stderr([] {
      auto client = Client(init(NamedChannelIdentifier("peer")), init());
      auto& slots = client.get_slots();
      register_test_services(out(slots));
      auto request = RequestToken<Client, IdentityService>(Ref(client), 17);
      request.set_exception(ServiceRequestException("rejected"));
      request.set_exception(std::make_exception_ptr(
        ServiceRequestException("rejected")));
      IdentityService::add_slot(out(slots), [] (auto&, auto) -> int {
        throw EndOfFileException();
      });
      auto message = IdentityService::Request<Client>(
        17, IdentityService::Parameters(7));
      REQUIRE_THROWS_AS(
        message.emit(slots.find(message), Ref(client)), EndOfFileException);
      REQUIRE_THROWS_AS(client.read_message(), EndOfFileException);
      client.close();
    });
    REQUIRE(output.empty());
  }

  TEST_CASE("read_diagnostics") {
    using Channel =
      BasicChannel<SocketIdentifier, NullConnection, PipedReader*, NullWriter>;
    using Client = ServiceProtocolClient<MessageProtocol<
      Channel*, BinarySender<SharedBuffer>, NullEncoder>, TriggerTimer>;
    auto output = capture_stderr([] {
      auto reader = PipedReader();
      auto writer = PipedWriter(Ref(reader));
      auto channel = Channel(SocketIdentifier(
        IpAddress("192.0.2.14", 53122)), init(), &reader, init());
      auto client = Client(&channel, init());
      try {
        throw_handler_exception();
      } catch(...) {
        writer.close(std::current_exception());
      }
      REQUIRE_THROWS_AS(client.read_message(), EndOfFileException);
      client.close();
    });
    REQUIRE(
      output.contains("client=192.0.2.14:53122 operation=read_message\n"));
    REQUIRE(output.contains("handler failure"));
    REQUIRE(output.contains("inner failure"));
    REQUIRE(!output.contains("message="));
    REQUIRE(!output.contains("request="));
  }

  TEST_CASE("client_lifecycle_diagnostics") {
    using Server = ServiceProtocolServer<LocalServerConnection*,
      BinarySender<SharedBuffer>, NullEncoder, std::unique_ptr<TriggerTimer>>;
    auto operation = std::string();
    SUBCASE("accept") {
      operation = "accept_client";
    }
    SUBCASE("close") {
      operation = "close_client";
    }
    auto output = capture_stderr([&] {
      auto connection = LocalServerConnection();
      auto closed = Async<void>();
      auto server = Server(&connection, [] {
        return std::make_unique<TriggerTimer>();
      }, [&] (auto&) {
        if(operation == "accept_client") {
          throw_handler_exception();
        }
      }, [&] (auto&) {
        closed.get_eval().set();
        if(operation == "close_client") {
          throw_handler_exception();
        }
      });
      auto client =
        ClientServiceProtocolClient(init("peer", connection), init());
      client.close();
      closed.get();
      server.close();
    });
    REQUIRE(output.contains("client=peer operation=" + operation + "\n"));
    REQUIRE(output.contains("handler failure"));
    REQUIRE(output.contains("inner failure"));
    auto position = output.find("Uncaught exception thrown:");
    REQUIRE(position != std::string::npos);
    REQUIRE(output.find("Uncaught exception thrown:", position + 1) ==
      std::string::npos);
  }

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
