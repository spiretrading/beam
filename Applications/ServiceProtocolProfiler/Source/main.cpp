#include <iostream>
#include <boost/format.hpp>
#include "Beam/Codecs/SizeDeclarativeDecoder.hpp"
#include "Beam/Codecs/SizeDeclarativeEncoder.hpp"
#include "Beam/Codecs/ZLibDecoder.hpp"
#include "Beam/Codecs/ZLibEncoder.hpp"
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Network/TcpServerSocket.hpp"
#include "Beam/Routines/RoutineHandlerGroup.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/TimeService/TriggerTimer.hpp"
#include "Beam/Utilities/ApplicationInterrupt.hpp"
#include "Beam/Utilities/Expect.hpp"
#include "Beam/Utilities/YamlConfig.hpp"
#include "ServiceProtocolProfiler/Services.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;

namespace {
  using ServiceEncoder = SizeDeclarativeEncoder<ZLibEncoder>;
  using ApplicationServerServiceProtocolClient = ServiceProtocolClient<
    MessageProtocol<std::unique_ptr<LocalServerChannel>,
      BinarySender<SharedBuffer>, ServiceEncoder>, TriggerTimer>;
  using ApplicationClientServiceProtocolClient = ServiceProtocolClient<
    MessageProtocol<LocalClientChannel*, BinarySender<SharedBuffer>,
      ServiceEncoder>, TriggerTimer>;

  std::string on_echo_request(
      ApplicationServerServiceProtocolClient& client, std::string message) {
    return message;
  }

  void server_loop(LocalServerConnection& server) {
    auto routines = RoutineHandlerGroup();
    while(true) {
      auto channel = server.accept();
      routines.spawn([channel = std::move(channel)] () mutable {
        auto client =
          ApplicationServerServiceProtocolClient(std::move(channel), init());
        register_service_protocol_profiler_services(out(client.get_slots()));
        register_service_protocol_profiler_messages(out(client.get_slots()));
        EchoService::add_slot(out(client.get_slots()), &on_echo_request);
        try {
          auto counter = 0;
          while(true) {
            auto message = client.read_message();
            auto timestamp = microsec_clock::universal_time();
            ++counter;
            if(counter % 100000 == 0) {
              std::cout << boost::format("Server: %1% %2%\n") % &client %
                timestamp << std::flush;
            }
          }
        } catch(const ServiceRequestException&) {
        }
      });
    }
  }

  void client_loop(LocalServerConnection& server) {
    auto channel = LocalClientChannel("client", server);
    auto client = ApplicationClientServiceProtocolClient(&channel, init());
    register_service_protocol_profiler_services(out(client.get_slots()));
    register_service_protocol_profiler_messages(out(client.get_slots()));
    auto counter = 0;
    while(true) {
      auto timestamp = microsec_clock::universal_time();
      send_record_message<EchoMessage>(client, timestamp, "hello world");
      ++counter;
      if(counter % 100000 == 0) {
        std::cout <<
          boost::format("Client: %1% %2%\n") % &channel % timestamp <<
          std::flush;
      }
      defer();
    }
    client.close();
  }
}

int main(int argc, const char** argv) {
  try {
    auto config =
      parse_command_line(argc, argv, "1.0-r" SERVICE_PROTOCOL_PROFILER_VERSION
        "\nCopyright (C) 2026 Spire Trading Inc.");
    auto client_count = extract<int>(config, "clients",
      static_cast<int>(boost::thread::hardware_concurrency()));
    auto server = LocalServerConnection();
    auto routines = RoutineHandlerGroup();
    routines.spawn([&] {
      server_loop(server);
    });
    for(auto i = 0; i < client_count; ++i) {
      routines.spawn([&] {
        client_loop(server);
      });
    }
    routines.wait();
  } catch(...) {
    report_current_exception();
    return -1;
  }
  return 0;
}
