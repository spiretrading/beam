#include <fstream>
#include <iostream>
#include <boost/functional/factory.hpp>
#include "Beam/Codecs/SizeDeclarativeDecoder.hpp"
#include "Beam/Codecs/SizeDeclarativeEncoder.hpp"
#include "Beam/Codecs/ZLibDecoder.hpp"
#include "Beam/Codecs/ZLibEncoder.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Network/TcpSocketChannel.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Serialization/ShuttleDateTime.hpp"
#include "Beam/Services/ServiceProtocolClientBuilder.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/Threading/LiveTimer.hpp"
#include "Beam/Utilities/ApplicationInterrupt.hpp"
#include "Beam/Utilities/Expect.hpp"
#include "Beam/Utilities/YamlConfig.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Serialization;
using namespace Beam::Services;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;

namespace {
  using ApplicationClient = ServiceProtocolClient<
    MessageProtocol<TcpSocketChannel, BinarySender<SharedBuffer>,
    SizeDeclarativeEncoder<ZLibEncoder>>, LiveTimer>;

  auto ParseAddress(const YAML::Node& config) {
    auto addresses = std::vector<IpAddress>();
    auto address = Extract<IpAddress>(config, "address");
    addresses.push_back(address);
    return addresses;
  }
}

namespace Beam {
  static const auto SERVICE_NAME = std::string("TEMPLATE_SERVICE");

  BEAM_DEFINE_SERVICES(ServletTemplateServices,

    /**
     * Submits a request to echo a message at a specified rate.
     * @param message <code>std::string</code> The message to echo.
     * @param rate The number of times per second to repeat the message.
     * @return <code>int</code> unusued.
     */
    (EchoService, "Beam.ServletTemplate.EchoService", int, std::string, message,
      int, rate));

  BEAM_DEFINE_MESSAGES(ServletTemplateMessages,

    /**
     * Sends an echo'd message.
     * @param message The message that was requested to be echo'd.
     */
    (EchoMessage, "Beam.ServletTemplate.EchoMessage", boost::posix_time::ptime,
      timestamp, std::string, message));
}

int main(int argc, const char** argv) {
  try {
    auto config = ParseCommandLine(argc, argv, "1.0-r" CLIENT_TEMPLATE_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc.");
    auto addresses = ParseAddress(config);
    auto message = Extract<std::string>(config, "message");
    auto rate = Extract<int>(config, "rate");
    auto client = ApplicationClient(Initialize(addresses),
      Initialize(seconds(10)));
    RegisterServletTemplateServices(Store(client.GetSlots()));
    RegisterServletTemplateMessages(Store(client.GetSlots()));
    auto result = client.SendRequest<EchoService>(message, rate);
    std::cout << result << std::endl;
    auto counter = 0;
    while(!ReceivedKillEvent()) {
      try {
        if(auto message = std::dynamic_pointer_cast<
            RecordMessage<EchoMessage, ApplicationClient>>(
            client.ReadMessage())) {
          ++counter;
          if(counter % rate == 0) {
            std::cout << message->GetRecord().timestamp << std::endl;
          }
        }
      } catch(...) {
        break;
      }
    }
  } catch(...) {
    ReportCurrentException();
    return -1;
  }
  return 0;
}
