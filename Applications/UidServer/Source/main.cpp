#include <boost/functional/factory.hpp>
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
#include "Beam/TimeService/LiveTimer.hpp"
#include "Beam/UidService/SqlUidDataStore.hpp"
#include "Beam/UidService/UidServlet.hpp"
#include "Beam/Utilities/ApplicationInterrupt.hpp"
#include "Beam/Utilities/YamlConfig.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Viper;

namespace {
  using UidServletContainer = ServiceProtocolServletContainer<
    MetaAuthenticationServletAdapter<
      MetaUidServlet<SqlUidDataStore<SqlConnection<MySql::Connection>>>,
      ApplicationServiceLocatorClient*>, TcpServerSocket,
    BinarySender<SharedBuffer>, NullEncoder, std::shared_ptr<LiveTimer>>;
}

int main(int argc, const char** argv) {
  try {
    auto config = parse_command_line(argc, argv, "1.0-r" UID_SERVER_VERSION
      "\nCopyright (C) 2026 Spire Trading Inc.");
    auto mysql_config = try_or_nest([&] {
      return MySqlConfig::parse(get_node(config, "data_store"));
    }, std::runtime_error("Error parsing section 'data_store'."));
    auto service_config = try_or_nest([&] {
      return ServiceConfiguration::parse(
        get_node(config, "server"), UID_SERVICE_NAME);
    }, std::runtime_error("Error parsing section 'server'."));
    auto service_locator_client = ApplicationServiceLocatorClient(
      ServiceLocatorClientConfig::parse(get_node(config, "service_locator")));
    auto server = UidServletContainer(init(&service_locator_client,
      init(make_sql_connection(MySql::Connection(
        mysql_config.m_address.get_host(), mysql_config.m_address.get_port(),
        mysql_config.m_username, mysql_config.m_password,
        mysql_config.m_schema)))), init(service_config.m_interface),
      std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10)));
    add(service_locator_client, service_config);
    wait_for_kill_event();
  } catch(...) {
    report_current_exception();
    return -1;
  }
  return 0;
}
