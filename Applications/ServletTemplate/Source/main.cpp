#include <fstream>
#include <iostream>
#include <boost/functional/factory.hpp>
#include <tclap/CmdLine.h>
#include "Beam/Codecs/SizeDeclarativeDecoder.hpp"
#include "Beam/Codecs/SizeDeclarativeEncoder.hpp"
#include "Beam/Codecs/ZLibDecoder.hpp"
#include "Beam/Codecs/ZLibEncoder.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Network/TcpServerSocket.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Services/ServiceProtocolServletContainer.hpp"
#include "Beam/Threading/LiveTimer.hpp"
#include "Beam/Utilities/ApplicationInterrupt.hpp"
#include "Beam/Utilities/Expect.hpp"
#include "Beam/Utilities/YamlConfig.hpp"
#include "ServletTemplate/Servlet.hpp"
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
using namespace TCLAP;

namespace {
  using ServletTemplateServletContainer =
    ServiceProtocolServletContainer<MetaServletTemplateServlet, TcpServerSocket,
    BinarySender<SharedBuffer>, SizeDeclarativeEncoder<ZLibEncoder>,
    std::shared_ptr<LiveTimer>>;
  using ApplicationServletTemplateServlet = ServletTemplateServlet<
    ServletTemplateServletContainer>;
}

void sub_main(const YAML::Node& config) {
  auto interface = Extract<IpAddress>(config, "interface");
  auto server = TryOrNest([&] {
    return ServletTemplateServletContainer(Initialize(), Initialize(interface),
      std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10)));
  }, std::runtime_error("Error opening server."));
  WaitForKillEvent();
}

int main(int argc, const char** argv) {
  auto configFile = std::string();
  try {
    auto cmd = CmdLine("", ' ', "1.0-r" SERVLET_TEMPLATE_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc.");
    auto configArg = ValueArg<std::string>("c", "config", "Configuration file",
      false, "config.yml", "path");
    cmd.add(configArg);
    cmd.parse(argc, argv);
    configFile = configArg.getValue();
  } catch(const ArgException& e) {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() <<
      std::endl;
    return -1;
  }
  try {
    sub_main(Require(LoadFile, configFile));
  } catch(...) {
    ReportCurrentException();
    return -1;
  }
  return 0;
}
