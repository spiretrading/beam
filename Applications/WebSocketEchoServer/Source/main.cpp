#include "Beam/Network/TcpServerSocket.hpp"
#include "Beam/Utilities/ApplicationInterrupt.hpp"
#include "Beam/Utilities/Expect.hpp"
#include "Beam/Utilities/YamlConfig.hpp"
#include "Beam/WebServices/HttpServletContainer.hpp"
#include "WebSocketEchoServer/WebSocketEchoServlet.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;

namespace {
  using WebSocketEchoServletContainer =
    HttpServletContainer<MetaWebSocketEchoServlet, TcpServerSocket>;
}

int main(int argc, const char** argv) {
  try {
    auto config =
      parse_command_line(argc, argv, "1.0-r" WEB_SOCKET_ECHO_SERVER_VERSION
        "\nCopyright (C) 2026 Spire Trading Inc.");
    auto interface = extract<IpAddress>(config, "interface");
    auto server = WebSocketEchoServletContainer(init(), init(interface));
    wait_for_kill_event();
  } catch(...) {
    report_current_exception();
    return -1;
  }
  return 0;
}
