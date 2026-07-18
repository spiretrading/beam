module;
#include "Prelude.hpp"
#include <tclap/CmdLine.h>
#include <yaml-cpp/yaml.h>
#include <boost/functional/factory.hpp>
#include "Beam/IO/SharedBuffer.hpp"
#include "Version.hpp"

module Beam;
#include "Beam/Utilities/YamlConfig.hpp"

#include "ServletTemplate/Servlet.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;

namespace {
  using ServletTemplateServletContainer =
    ServiceProtocolServletContainer<MetaServletTemplateServlet, TcpServerSocket,
      BinarySender<SharedBuffer>, SizeDeclarativeEncoder<ZLibEncoder>,
      std::shared_ptr<LiveTimer>>;
  using ApplicationServletTemplateServlet =
    ServletTemplateServlet<ServletTemplateServletContainer>;
}

int main(int argc, const char** argv) {
  try {
    auto config =
      parse_command_line(argc, argv, "1.0-r" SERVLET_TEMPLATE_VERSION
        "\nCopyright (C) 2020 Spire Trading Inc.");
    auto interface = extract<IpAddress>(config, "interface");
    auto server = ServletTemplateServletContainer(init(), init(interface),
      std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10)));
    wait_for_kill_event();
  } catch(...) {
    report_current_exception();
    return -1;
  }
  return 0;
}
