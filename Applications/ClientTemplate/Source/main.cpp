#include <fstream>
#include <iostream>
#include <boost/functional/factory.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "Beam/Codecs/SizeDeclarativeDecoder.hpp"
#include "Beam/Codecs/SizeDeclarativeEncoder.hpp"
#include "Beam/Codecs/ZLibDecoder.hpp"
#include "Beam/Codecs/ZLibEncoder.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Network/TcpSocketChannel.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Serialization/ShuttleDateTime.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/Services/ServiceProtocolClientBuilder.hpp"
#include "Beam/TimeService/LiveTimer.hpp"
#include "Beam/Utilities/ApplicationInterrupt.hpp"
#include "Beam/Utilities/Expect.hpp"
#include "Beam/Utilities/YamlConfig.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;

namespace {
  using ApplicationClient = ServiceProtocolClient<
    MessageProtocol<TcpSocketChannel, BinarySender<SharedBuffer>,
    SizeDeclarativeEncoder<ZLibEncoder>>, LiveTimer>;

  auto parse_address(const YAML::Node& config) {
    auto addresses = std::vector<IpAddress>();
    auto address = extract<IpAddress>(config, "address");
    addresses.push_back(address);
    return addresses;
  }
}

namespace Beam {
  static const auto SERVICE_NAME = std::string("TEMPLATE_SERVICE");

  BEAM_DEFINE_SERVICES(servlet_template_services,

    /**
     * Submits a request to echo a message at a specified rate.
     * @param message <code>std::string</code> The message to echo.
     * @param rate The number of times per second to repeat the message.
     * @return <code>int</code> unusued.
     */
    (EchoService, "Beam.ServletTemplate.EchoService", int,
      (std::string, message), (int, rate)));

  BEAM_DEFINE_MESSAGES(servlet_template_messages,

    /**
     * Sends an echo'd message.
     * @param message The message that was requested to be echo'd.
     */
    (EchoMessage, "Beam.ServletTemplate.EchoMessage",
      (boost::posix_time::ptime, timestamp), (std::string, message)));
}

int main(int argc, const char** argv) {
  try {
    auto config = parse_command_line(argc, argv, "1.0-r" CLIENT_TEMPLATE_VERSION
      "\nCopyright (C) 2026 Spire Trading Inc.");
    auto addresses = parse_address(config);
    auto message = extract<std::string>(config, "message");
    auto rate = extract<int>(config, "rate");
    auto client = ApplicationClient(init(addresses), init(seconds(10)));
    register_servlet_template_services(out(client.get_slots()));
    register_servlet_template_messages(out(client.get_slots()));
    auto result = client.send_request<EchoService>(message, rate);
    std::cout << result << std::endl;
    auto counter = 0;
    while(!received_kill_event()) {
      try {
        if(auto message = std::dynamic_pointer_cast<
            RecordMessage<EchoMessage, ApplicationClient>>(
              client.read_message())) {
          ++counter;
          if(counter % rate == 0) {
            std::cout << message->get_record().timestamp << std::endl;
          }
        }
      } catch(...) {
        break;
      }
    }
  } catch(...) {
    report_current_exception();
    return -1;
  }
  return 0;
}
