#include <fstream>
#include <iostream>
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>
#include <tclap/CmdLine.h>
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
#include "ClientTemplate/Version.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Serialization;
using namespace Beam::Services;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace std;
using namespace TCLAP;

namespace {
  using ApplicationClient = ServiceProtocolClient<
    MessageProtocol<TcpSocketChannel, BinarySender<SharedBuffer>,
    SizeDeclarativeEncoder<ZLibEncoder>>, LiveTimer>;

  vector<IpAddress> ParseAddress(const YAML::Node& config) {
    vector<IpAddress> addresses;
    auto address = Extract<IpAddress>(config, "address");
    addresses.push_back(address);
    return addresses;
  }
}

namespace Beam {
  static const std::string SERVICE_NAME = "TEMPLATE_SERVICE";

  BEAM_DEFINE_SERVICES(ServletTemplateServices,

    /*! \interface Beam::ServletTemplate::EchoService
        \brief Submits a request to echo a message at a specified rate.
        \param message <code>std::string</code> The message to echo.
        \param rate The number of times per second to repeat the message.
        \return <code>int</code> unusued.
    */
    //! \cond
    (EchoService, "Beam.ServletTemplate.EchoService", int, std::string, message,
      int, rate));
    //! \endcond

  BEAM_DEFINE_MESSAGES(ServletTemplateMessages,

    /*! \interface Beam::ServletTemplate::EchoMessage
        \brief Sends an echo'd message.
        \param message The message that was requested to be echo'd.
    */
    //! \cond
    (EchoMessage, "Beam.ServletTemplate.EchoMessage", boost::posix_time::ptime,
      timestamp, std::string, message));
    //! \endcond
}

int main(int argc, const char** argv) {
  string configFile;
  try {
    CmdLine cmd{"", ' ', "1.0-r" CLIENT_TEMPLATE_VERSION
      "\nCopyright (C) 2009 Eidolon Systems Ltd."};
    ValueArg<string> configArg{"c", "config", "Configuration file", false,
      "config.yml", "path"};
    cmd.add(configArg);
    cmd.parse(argc, argv);
    configFile = configArg.getValue();
  } catch(ArgException& e) {
    cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
    return -1;
  }
  auto config = Require(LoadFile, configFile);
  SocketThreadPool socketThreadPool;
  TimerThreadPool timerThreadPool;
  string message;
  optional<ApplicationClient> client;
  int rate;
  try {
    auto addresses = ParseAddress(config);
    message = Extract<string>(config, "message");
    rate = Extract<int>(config, "rate");
    client.emplace(Initialize(addresses, Ref(socketThreadPool)),
      Initialize(seconds{10}, Ref(timerThreadPool)));
  } catch(const std::exception& e) {
    cerr << "Unable to initialize client: " << e.what() << endl;
    return -1;
  }
  RegisterServletTemplateServices(Store(client->GetSlots()));
  RegisterServletTemplateMessages(Store(client->GetSlots()));
  client->Open();
  auto result = client->SendRequest<EchoService>(message, rate);
  std::cout << result << std::endl;
  int counter = 0;
  while(!ReceivedKillEvent()) {
    try {
      auto message = std::dynamic_pointer_cast<
        RecordMessage<EchoMessage, ApplicationClient>>(client->ReadMessage());
      if(message != nullptr) {
        ++counter;
        if(counter % rate == 0) {
          std::cout << message->GetRecord().timestamp << std::endl;
        }
      }
    } catch(...) {
      break;
    }
  }
  return 0;
}
