#include <fstream>
#include <iostream>
#include <tclap/CmdLine.h>
#include "Beam/Network/TcpServerSocket.hpp"
#include "Beam/Threading/TimerThreadPool.hpp"
#include "Beam/Utilities/ApplicationInterrupt.hpp"
#include "Beam/Utilities/Expect.hpp"
#include "Beam/Utilities/YamlConfig.hpp"
#include "Beam/WebServices/HttpServletContainer.hpp"
#include "WebSocketEchoServer/WebSocketEchoServlet.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Threading;
using namespace Beam::WebServices;
using namespace Beam::WebSocketEchoServer;
using namespace boost;
using namespace boost::posix_time;
using namespace std;
using namespace TCLAP;

namespace {
  using WebSocketEchoServletContainer =
    HttpServletContainer<MetaWebSocketEchoServlet, TcpServerSocket>;

  struct ServerConnectionInitializer {
    IpAddress m_interface;
    vector<IpAddress> m_addresses;

    void Initialize(const YAML::Node& config);
  };

  void ServerConnectionInitializer::Initialize(const YAML::Node& config) {
    m_interface = Extract<IpAddress>(config, "interface");
    auto addresses = vector<IpAddress>();
    addresses.push_back(m_interface);
    m_addresses = Extract<vector<IpAddress>>(config, "addresses", addresses);
  }
}

int main(int argc, const char** argv) {
  auto configFile = string();
  try {
    auto cmd = CmdLine("", ' ', "0.9-r" WEB_SOCKET_ECHO_SERVER_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc.");
    auto configArg = ValueArg<string>("c", "config", "Configuration file",
      false, "config.yml", "path");
    cmd.add(configArg);
    cmd.parse(argc, argv);
    configFile = configArg.getValue();
  } catch(const ArgException& e) {
    cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
    return -1;
  }
  auto config = Require(LoadFile, configFile);
  auto socketThreadPool = SocketThreadPool();
  auto timerThreadPool = TimerThreadPool();
  auto serverConnectionInitializer = ServerConnectionInitializer();
  try {
    serverConnectionInitializer.Initialize(GetNode(config, "server"));
  } catch(const std::exception& e) {
    cerr << "Error parsing section 'server': " << e.what() << endl;
    return -1;
  }
  auto server = WebSocketEchoServletContainer(Initialize(),
    Initialize(serverConnectionInitializer.m_interface, Ref(socketThreadPool)));
  try {
    server.Open();
  } catch(const std::exception& e) {
    cerr << "Error opening server: " << e.what() << endl;
    return -1;
  }
  WaitForKillEvent();
  return 0;
}
