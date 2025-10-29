#include "Beam/Network/TcpServerSocket.hpp"
#include "Beam/Utilities/ApplicationInterrupt.hpp"
#include "Beam/Utilities/Expect.hpp"
#include "Beam/Utilities/YamlConfig.hpp"
#include "Beam/WebServices/HttpServletContainer.hpp"
#include "HttpFileServer/HttpFileServlet.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;

namespace {
  using HttpFileServletContainer =
    HttpServletContainer<HttpFileServlet, TcpServerSocket>;
}

int main(int argc, const char** argv) {
  try {
    auto config =
      parse_command_line(argc, argv, "1.0-r" HTTP_FILE_SERVER_VERSION
        "\nCopyright (C) 2026 Spire Trading Inc.");
    auto interface = extract<IpAddress>(config, "interface");
    auto server = HttpFileServletContainer(init(), init(interface));
    wait_for_kill_event();
  } catch(...) {
    report_current_exception();
    return -1;
  }
  return 0;
}
