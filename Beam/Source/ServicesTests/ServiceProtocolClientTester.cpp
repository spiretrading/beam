#include "Beam/ServicesTests/ServiceProtocolClientTester.hpp"
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
using namespace boost;
using namespace std;

namespace {
  void OnVoidRequest(RequestToken<ServiceProtocolClientTester::
      ServerServiceProtocolClient, VoidService>& request, int n,
      int* callbackCount) {
    ++*callbackCount;
    request.SetResult();
  }

  void OnExceptionVoidRequest(RequestToken<ServiceProtocolClientTester::
      ServerServiceProtocolClient, VoidService>& request, int n,
      int* callbackCount) {
    ++*callbackCount;
    request.SetException(ServiceRequestException());
  }
}

void ServiceProtocolClientTester::TestVoidReturnType() {
  ServerConnection server;
  auto callbackCount = 0;
  RoutineHandler serverTask = Spawn(
    [&] {
      server.Open();
      auto clientChannel = server.Accept();
      ServiceProtocolClientTester::ServerServiceProtocolClient client(
        std::move(clientChannel), Initialize());
      RegisterTestServices(Store(client.GetSlots()));
      VoidService::AddRequestSlot(Store(client.GetSlots()),
        std::bind(OnVoidRequest, std::placeholders::_1, std::placeholders::_2,
        &callbackCount));
      client.Open();
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
    });
  RoutineHandler clientTask = Spawn(
    [&] {
      ClientServiceProtocolClient client(Initialize(string("client"),
        Ref(server)), Initialize());
      RegisterTestServices(Store(client.GetSlots()));
      client.Open();
      client.SendRequest<VoidService>(123);
      client.SendRequest<VoidService>(321);
      client.Close();
    });
  clientTask.Wait();
  serverTask.Wait();
  CPPUNIT_ASSERT(callbackCount == 2);
}

void ServiceProtocolClientTester::TestException() {
  ServerConnection server;
  auto callbackCount = 0;
  RoutineHandler serverTask = Spawn(
    [&] {
      server.Open();
      auto clientChannel = server.Accept();
      ServiceProtocolClientTester::ServerServiceProtocolClient client(
        std::move(clientChannel), Initialize());
      RegisterTestServices(Store(client.GetSlots()));
      VoidService::AddRequestSlot(Store(client.GetSlots()), std::bind(
        OnExceptionVoidRequest, std::placeholders::_1, std::placeholders::_2,
        &callbackCount));
      client.Open();
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
    });
  RoutineHandler clientTask = Spawn(
    [&] {
      ClientServiceProtocolClient client(Initialize(string("client"),
        Ref(server)), Initialize());
      RegisterTestServices(Store(client.GetSlots()));
      client.Open();
      CPPUNIT_ASSERT_THROW(client.SendRequest<VoidService>(123),
        ServiceRequestException);
      client.Close();
    });
  clientTask.Wait();
  serverTask.Wait();
  CPPUNIT_ASSERT(callbackCount == 1);
}

void ServiceProtocolClientTester::TestRequestBeforeConnectionClosed() {
  ServerConnection server;
  auto callbackCount = 0;
  RoutineHandler serverTask = Spawn(
    [&] {
      server.Open();
      auto clientChannel = server.Accept();
      ServiceProtocolClientTester::ServerServiceProtocolClient client(
        std::move(clientChannel), Initialize());
      RegisterTestServices(Store(client.GetSlots()));
      VoidService::AddRequestSlot(Store(client.GetSlots()), std::bind(
        OnExceptionVoidRequest, std::placeholders::_1, std::placeholders::_2,
        &callbackCount));
      client.Open();
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
    });
  RoutineHandler clientTask = Spawn(
    [&] {
      ClientChannel clientChannel(string("client"), Ref(server));
      ServiceProtocolClient<MessageProtocol<ClientChannel*,
        BinarySender<SharedBuffer>, NullEncoder>, TriggerTimer> client(
        &clientChannel, Initialize());
      RegisterTestServices(Store(client.GetSlots()));
      client.Open();
      CPPUNIT_ASSERT_THROW(client.SendRequest<VoidService>(123),
        ServiceRequestException);
      client.Close();
    });
  clientTask.Wait();
  serverTask.Wait();
}

void ServiceProtocolClientTester::TestRequestAfterConnectionClosed() {
  ServerConnection server;
  RoutineHandler serverTask = Spawn(
    [&] {
      server.Open();
      auto clientChannel = server.Accept();
      clientChannel->GetConnection().Close();
    });
  RoutineHandler clientTask = Spawn(
    [&] {
      ClientChannel clientChannel(string("client"), Ref(server));
      ServiceProtocolClient<MessageProtocol<ClientChannel*,
        BinarySender<SharedBuffer>, NullEncoder>, TriggerTimer> client(
        &clientChannel, Initialize());
      RegisterTestServices(Store(client.GetSlots()));
      client.Open();
      clientChannel.GetConnection().Close();
      CPPUNIT_ASSERT_THROW(client.SendRequest<VoidService>(123),
        ServiceRequestException);
      client.Close();
    });
  clientTask.Wait();
  serverTask.Wait();
}
