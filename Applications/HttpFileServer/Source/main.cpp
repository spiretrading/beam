#include <fstream>
#include <iostream>
#include <tclap/CmdLine.h>
#include "Beam/Network/TcpServerSocket.hpp"
#include "Beam/Utilities/ApplicationInterrupt.hpp"
#include "Beam/Utilities/Expect.hpp"
#include "Beam/Utilities/YamlConfig.hpp"
#include "Beam/WebServices/HttpServletContainer.hpp"
#include "HttpFileServer/HttpFileServlet.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace Beam::HttpFileServer;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::WebServices;
using namespace boost;
using namespace boost::posix_time;
using namespace TCLAP;

namespace {
  using HttpFileServletContainer = HttpServletContainer<HttpFileServlet,
    TcpServerSocket>;
}

int main(int argc, const char** argv) {
  auto configFile = std::string();
  try {
    auto cmd = CmdLine("", ' ', "0.9-r" HTTP_FILE_SERVER_VERSION
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
  auto config = Require(LoadFile, configFile);
  try {
    auto interface = Extract<IpAddress>(config, "interface");
    auto server = HttpFileServletContainer(Initialize(), Initialize(interface));
    WaitForKillEvent();
  } catch(...) {
    ReportCurrentException();
    return -1;
  }
  return 0;
}
