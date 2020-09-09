#include <doctest/doctest.h>
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/Routines/RoutineHandler.hpp"
#include "Beam/Services/ServiceSlots.hpp"
#include "Beam/ServicesTests/TestServices.hpp"
#include "Beam/Utilities/Capture.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;
using namespace Beam::Routines;
using namespace Beam::Serialization;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::Threading;

namespace {
  using TestServerConnection = LocalServerConnection<SharedBuffer>;
  using ClientChannel = LocalClientChannel<SharedBuffer>;
  using ServerServiceProtocolClient = ServiceProtocolClient<
    MessageProtocol<std::unique_ptr<TestServerConnection::Channel>,
    BinarySender<SharedBuffer>, NullEncoder>, TriggerTimer>;
  using ClientServiceProtocolClient = ServiceProtocolClient<
    MessageProtocol<ClientChannel, BinarySender<SharedBuffer>, NullEncoder>,
    TriggerTimer>;

  void OnVoidRequest(
      RequestToken<ServerServiceProtocolClient, VoidService>& request, int n,
      int* callbackCount) {
    ++*callbackCount;
    request.SetResult();
  }

  void OnExceptionVoidRequest(
      RequestToken<ServerServiceProtocolClient, VoidService>& request, int n,
      int* callbackCount) {
    ++*callbackCount;
    request.SetException(ServiceRequestException());
  }
}

TEST_SUITE("ServiceProtocolClient") {
  TEST_CASE("void_return_type") {
    auto server = TestServerConnection();
    auto callbackCount = 0;
    auto serverTask = RoutineHandler(Spawn(
      [&] {
        auto clientChannel = server.Accept();
        auto client = ServerServiceProtocolClient(std::move(clientChannel),
          Initialize());
        RegisterTestServices(Store(client.GetSlots()));
        VoidService::AddRequestSlot(Store(client.GetSlots()),
          std::bind(OnVoidRequest, std::placeholders::_1, std::placeholders::_2,
          &callbackCount));
        try {
          while(true) {
            auto message = client.ReadMessage();
            auto slot = client.GetSlots().Find(*message);
            if(slot != nullptr) {
              message->EmitSignal(slot, Ref(client));
            }
          }
        } catch(const ServiceRequestException&) {
        } catch(const EndOfFileException&) {
        }
      }));
    auto clientTask = RoutineHandler(Spawn(
      [&] {
        auto client = ClientServiceProtocolClient(Initialize("client", server),
          Initialize());
        RegisterTestServices(Store(client.GetSlots()));
        client.SendRequest<VoidService>(123);
        client.SendRequest<VoidService>(321);
        client.Close();
      }));
    clientTask.Wait();
    serverTask.Wait();
    REQUIRE(callbackCount == 2);
  }

  TEST_CASE("exception") {
    auto server = TestServerConnection();
    auto callbackCount = 0;
    auto serverTask = RoutineHandler(Spawn(
      [&] {
        auto clientChannel = server.Accept();
        auto client = ServerServiceProtocolClient(std::move(clientChannel),
          Initialize());
        RegisterTestServices(Store(client.GetSlots()));
        VoidService::AddRequestSlot(Store(client.GetSlots()), std::bind(
          OnExceptionVoidRequest, std::placeholders::_1, std::placeholders::_2,
          &callbackCount));
        try {
          while(true) {
            auto message = client.ReadMessage();
            auto slot = client.GetSlots().Find(*message);
            if(slot != nullptr) {
              message->EmitSignal(slot, Ref(client));
            }
          }
        } catch(const ServiceRequestException&) {
        } catch(const EndOfFileException&) {
        }
      }));
    auto clientTask = RoutineHandler(Spawn(
      [&] {
        auto client = ClientServiceProtocolClient(Initialize("client", server),
          Initialize());
        RegisterTestServices(Store(client.GetSlots()));
        REQUIRE_THROWS_AS(client.SendRequest<VoidService>(123),
          ServiceRequestException);
        client.Close();
      }));
    clientTask.Wait();
    serverTask.Wait();
    REQUIRE(callbackCount == 1);
  }

  TEST_CASE("request_before_connection_closed") {
    auto server = TestServerConnection();
    auto callbackCount = 0;
    auto serverTask = RoutineHandler(Spawn(
      [&] {
        auto clientChannel = server.Accept();
        auto client = ServerServiceProtocolClient(std::move(clientChannel),
          Initialize());
        RegisterTestServices(Store(client.GetSlots()));
        VoidService::AddRequestSlot(Store(client.GetSlots()), std::bind(
          OnExceptionVoidRequest, std::placeholders::_1, std::placeholders::_2,
          &callbackCount));
        try {
          while(true) {
            auto message = client.ReadMessage();
            auto slot = client.GetSlots().Find(*message);
            if(slot != nullptr) {
              client.Close();
            }
          }
        } catch(const ServiceRequestException&) {
        } catch(const EndOfFileException&) {
        }
      }));
    auto clientTask = RoutineHandler(Spawn(
      [&] {
        auto clientChannel = ClientChannel("client", server);
        auto client = ServiceProtocolClient<MessageProtocol<ClientChannel*,
          BinarySender<SharedBuffer>, NullEncoder>, TriggerTimer>(
          &clientChannel, Initialize());
        RegisterTestServices(Store(client.GetSlots()));
        REQUIRE_THROWS_AS(client.SendRequest<VoidService>(123),
          ServiceRequestException);
        client.Close();
      }));
    clientTask.Wait();
    serverTask.Wait();
  }

  TEST_CASE("request_after_connection_closed") {
    auto server = TestServerConnection();
    auto serverTask = RoutineHandler(Spawn(
      [&] {
        auto clientChannel = server.Accept();
        clientChannel->GetConnection().Close();
      }));
    auto clientTask = RoutineHandler(Spawn(
      [&] {
        auto clientChannel = ClientChannel("client", server);
        auto client = ServiceProtocolClient<MessageProtocol<ClientChannel*,
          BinarySender<SharedBuffer>, NullEncoder>, TriggerTimer>(
          &clientChannel, Initialize());
        RegisterTestServices(Store(client.GetSlots()));
        clientChannel.GetConnection().Close();
        REQUIRE_THROWS_AS(client.SendRequest<VoidService>(123),
          ServiceRequestException);
        client.Close();
      }));
    clientTask.Wait();
    serverTask.Wait();
  }
}
