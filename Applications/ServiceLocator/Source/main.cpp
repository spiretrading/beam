#include <fstream>
#include <iostream>
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>
#include <tclap/CmdLine.h>
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/MySql/MySqlConfig.hpp"
#include "Beam/Network/TcpServerSocket.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/ServiceLocator/CachedServiceLocatorDataStore.hpp"
#include "Beam/ServiceLocator/MySqlServiceLocatorDataStore.hpp"
#include "Beam/ServiceLocator/ServiceLocatorServlet.hpp"
#include "Beam/Services/ServiceProtocolServletContainer.hpp"
#include "Beam/Threading/LiveTimer.hpp"
#include "Beam/Utilities/ApplicationInterrupt.hpp"
#include "Beam/Utilities/YamlConfig.hpp"
#include "ServiceLocator/Version.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;
using namespace Beam::MySql;
using namespace Beam::Network;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace std;
using namespace TCLAP;

namespace {
  using ServiceLocatorServletContainer = ServiceProtocolServletContainer<
    MetaServiceLocatorServlet<CachedServiceLocatorDataStore<
    MySqlServiceLocatorDataStore*>>, TcpServerSocket,
    BinarySender<SharedBuffer>, NullEncoder, std::shared_ptr<LiveTimer>>;

  struct ServerConnectionInitializer {
    IpAddress m_interface;
    vector<IpAddress> m_addresses;

    void Initialize(const YAML::Node& config);
  };

  void ServerConnectionInitializer::Initialize(const YAML::Node& config) {
    m_interface = Extract<IpAddress>(config, "interface");
    vector<IpAddress> addresses;
    addresses.push_back(m_interface);
    m_addresses = Extract<vector<IpAddress>>(config, "addresses", addresses);
  }
}

int main(int argc, const char** argv) {
  string configFile;
  try {
    CmdLine cmd{"", ' ', "1.0-r" SERVICE_LOCATOR_VERSION
      "\nCopyright (C) 2009 Eidolon Systems Ltd."};
    ValueArg<string> configArg{"c", "config", "Configuration file", false,
      "config.yml", "path"};
    cmd.add(configArg);
    cmd.parse(argc, argv);
    configFile = configArg.getValue();
  } catch(const ArgException& e) {
    cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
    return -1;
  }
  YAML::Node config;
  try {
    ifstream configStream{configFile.c_str()};
    if(!configStream.good()) {
      cerr << configFile << " not found." << endl;
      return -1;
    }
    YAML::Parser configParser{configStream};
    configParser.GetNextDocument(config);
  } catch(const YAML::ParserException& e) {
    cerr << "Invalid YAML at line " << (e.mark.line + 1) << ", " << "column " <<
      (e.mark.column + 1) << ": " << e.msg << endl;
    return -1;
  }
  MySqlConfig mySqlConfig;
  try {
    mySqlConfig = MySqlConfig::Parse(GetNode(config, "data_store"));
  } catch(const std::exception& e) {
    cerr << "Error parsing section 'data_store': " << e.what() << endl;
    return -1;
  }
  ServerConnectionInitializer serverConnectionInitializer;
  try {
    serverConnectionInitializer.Initialize(GetNode(config, "server"));
  } catch(const std::exception& e) {
    cerr << "Error parsing section 'server': " << e.what() << endl;
    return -1;
  }
  SocketThreadPool socketThreadPool;
  TimerThreadPool timerThreadPool;
  MySqlServiceLocatorDataStore mysqlDataStore{mySqlConfig.m_address,
    mySqlConfig.m_schema, mySqlConfig.m_username, mySqlConfig.m_password};
  ServiceLocatorServletContainer server(Initialize(Initialize(&mysqlDataStore)),
    Initialize(serverConnectionInitializer.m_interface, Ref(socketThreadPool)),
    std::bind(factory<std::shared_ptr<LiveTimer>>{}, seconds{10},
    Ref(timerThreadPool)));
  try {
    server.Open();
  } catch(const std::exception& e) {
    cerr << "Error opening server: " << e.what() << endl;
    return -1;
  }
  WaitForKillEvent();
  return 0;
}
