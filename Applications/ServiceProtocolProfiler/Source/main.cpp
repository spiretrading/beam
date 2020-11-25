#include <iostream>
#include <boost/format.hpp>
#include "Beam/Codecs/SizeDeclarativeDecoder.hpp"
#include "Beam/Codecs/SizeDeclarativeEncoder.hpp"
#include "Beam/Codecs/ZLibDecoder.hpp"
#include "Beam/Codecs/ZLibEncoder.hpp"
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/NotConnectedException.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Network/TcpServerSocket.hpp"
#include "Beam/Routines/RoutineHandlerGroup.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/Threading/TriggerTimer.hpp"
#include "Beam/Utilities/ApplicationInterrupt.hpp"
#include "Beam/Utilities/Expect.hpp"
#include "Beam/Utilities/YamlConfig.hpp"
#include "ServiceProtocolProfiler/Services.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Routines;
using namespace Beam::Serialization;
using namespace Beam::Services;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;

namespace {
  using ServiceEncoder = SizeDeclarativeEncoder<ZLibEncoder>;
  using ApplicationServerConnection = LocalServerConnection<SharedBuffer>;
  using ServerChannel = ApplicationServerConnection::Channel;
  using ApplicationServerServiceProtocolClient = ServiceProtocolClient<
    MessageProtocol<std::unique_ptr<ServerChannel>, BinarySender<SharedBuffer>,
    ServiceEncoder>, TriggerTimer>;
  using ClientChannel = LocalClientChannel<SharedBuffer>;
  using ApplicationClientServiceProtocolClient = ServiceProtocolClient<
    MessageProtocol<ClientChannel*, BinarySender<SharedBuffer>,
    ServiceEncoder>, TriggerTimer>;

  std::string OnEchoRequest(ApplicationServerServiceProtocolClient& client,
      std::string message) {
    return message;
  }

  void ServerLoop(ApplicationServerConnection& server) {
    auto routines = RoutineHandlerGroup();
    while(true) {
      auto channel = server.Accept();
      routines.Spawn([channel = std::move(channel)] () mutable {
        auto client = ApplicationServerServiceProtocolClient(std::move(channel),
          Initialize());
        RegisterServiceProtocolProfilerServices(Store(client.GetSlots()));
        RegisterServiceProtocolProfilerMessages(Store(client.GetSlots()));
        EchoService::AddSlot(Store(client.GetSlots()),
          std::bind(OnEchoRequest, std::placeholders::_1,
          std::placeholders::_2));
        try {
          auto counter = 0;
          while(true) {
            auto message = client.ReadMessage();
            auto timestamp = microsec_clock::universal_time();
            ++counter;
            if(counter % 100000 == 0) {
              std::cout << boost::format("Server: %1% %2%\n") % &client %
                timestamp << std::flush;
            }
          }
        } catch(const ServiceRequestException&) {
        } catch(const NotConnectedException&) {
        }
      });
    }
  }

  void ClientLoop(ApplicationServerConnection& server) {
    auto channel = ClientChannel("client", server);
    auto client = ApplicationClientServiceProtocolClient(&channel,
      Initialize());
    RegisterServiceProtocolProfilerServices(Store(client.GetSlots()));
    RegisterServiceProtocolProfilerMessages(Store(client.GetSlots()));
    auto counter = 0;
    while(true) {
      auto timestamp = microsec_clock::universal_time();
      SendRecordMessage<EchoMessage>(client, timestamp, "hello world");
      ++counter;
      if(counter % 100000 == 0) {
        std::cout <<
          boost::format("Client: %1% %2%\n") % &channel % timestamp <<
          std::flush;
      }
      Defer();
    }
    client.Close();
  }
}

int main(int argc, const char** argv) {
  try {
    auto config = ParseCommandLine(argc, argv,
      "1.0-r" SERVICE_PROTOCOL_PROFILER_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc.");
    auto clientCount = Extract<int>(config, "clients",
      static_cast<int>(boost::thread::hardware_concurrency()));
    auto server = ApplicationServerConnection();
    auto routines = RoutineHandlerGroup();
    routines.Spawn([&] {
      ServerLoop(server);
    });
    for(auto i = 0; i < clientCount; ++i) {
      routines.Spawn([&] {
        ClientLoop(server);
      });
    }
    routines.Wait();
  } catch(...) {
    ReportCurrentException();
    return -1;
  }
  return 0;
}
