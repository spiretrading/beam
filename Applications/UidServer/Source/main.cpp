#include <fstream>
#include <iostream>
#include <boost/functional/factory.hpp>
#include <tclap/CmdLine.h>
#include <Viper/MySql/Connection.hpp>
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Network/TcpServerSocket.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/ServiceLocator/ApplicationDefinitions.hpp"
#include "Beam/ServiceLocator/AuthenticationServletAdapter.hpp"
#include "Beam/Services/ServiceProtocolServletContainer.hpp"
#include "Beam/Sql/MySqlConfig.hpp"
#include "Beam/Sql/SqlConnection.hpp"
#include "Beam/Threading/LiveTimer.hpp"
#include "Beam/UidService/SqlUidDataStore.hpp"
#include "Beam/UidService/UidServlet.hpp"
#include "Beam/Utilities/ApplicationInterrupt.hpp"
#include "Beam/Utilities/YamlConfig.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::Threading;
using namespace Beam::UidService;
using namespace boost;
using namespace boost::posix_time;
using namespace TCLAP;
using namespace Viper;

namespace {
  using UidServletContainer = ServiceProtocolServletContainer<
    MetaAuthenticationServletAdapter<
    MetaUidServlet<SqlUidDataStore<SqlConnection<MySql::Connection>>>,
    ApplicationServiceLocatorClient::Client*>, TcpServerSocket,
    BinarySender<SharedBuffer>, NullEncoder, std::shared_ptr<LiveTimer>>;
}

void sub_main(const YAML::Node& config) {
  auto mySqlConfig = TryOrNest([&] {
    return MySqlConfig::Parse(GetNode(config, "data_store"));
  }, std::runtime_error("Error parsing section 'data_store'."));
  auto serviceConfig = TryOrNest([&] {
    return ServiceConfiguration::Parse(GetNode(config, "server"),
      UidService::SERVICE_NAME);
  }, std::runtime_error("Error parsing section 'server'."));
  auto serviceLocatorClient = MakeApplicationServiceLocatorClient(
    GetNode(config, "service_locator"));
  auto server = TryOrNest([&] {
    return UidServletContainer(Initialize(serviceLocatorClient.Get(),
      Initialize(MakeSqlConnection(MySql::Connection(
      mySqlConfig.m_address.GetHost(), mySqlConfig.m_address.GetPort(),
      mySqlConfig.m_username, mySqlConfig.m_password, mySqlConfig.m_schema)))),
      Initialize(serviceConfig.m_interface),
      std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10)));
  }, std::runtime_error("Error opening server."));
  Register(*serviceLocatorClient, serviceConfig);
  WaitForKillEvent();
}

int main(int argc, const char** argv) {
  auto configFile = std::string();
  try {
    auto cmd = CmdLine("", ' ', "1.0-r" UID_SERVER_VERSION
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
