#include <boost/functional/factory.hpp>
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

namespace {
  using ServletTemplateServletContainer =
    ServiceProtocolServletContainer<MetaServletTemplateServlet, TcpServerSocket,
    BinarySender<SharedBuffer>, SizeDeclarativeEncoder<ZLibEncoder>,
    std::shared_ptr<LiveTimer>>;
  using ApplicationServletTemplateServlet = ServletTemplateServlet<
    ServletTemplateServletContainer>;
}

int main(int argc, const char** argv) {
  try {
    auto config = ParseCommandLine(argc, argv, "1.0-r" SERVLET_TEMPLATE_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc.");
    auto interface = Extract<IpAddress>(config, "interface");
    auto server = ServletTemplateServletContainer(Initialize(),
      Initialize(interface),
      std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10)));
    WaitForKillEvent();
  } catch(...) {
    ReportCurrentException();
    return -1;
  }
  return 0;
}
