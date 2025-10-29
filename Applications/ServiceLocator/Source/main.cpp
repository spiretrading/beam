#include <boost/functional/factory.hpp>
#include <Viper/MySql/Connection.hpp>
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Network/TcpServerSocket.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/ServiceLocator/CachedServiceLocatorDataStore.hpp"
#include "Beam/ServiceLocator/ServiceLocatorServlet.hpp"
#include "Beam/ServiceLocator/SqlServiceLocatorDataStore.hpp"
#include "Beam/Services/ServiceProtocolServletContainer.hpp"
#include "Beam/Sql/MySqlConfig.hpp"
#include "Beam/Sql/SqlConnection.hpp"
#include "Beam/TimeService/LiveTimer.hpp"
#include "Beam/Utilities/ApplicationInterrupt.hpp"
#include "Beam/Utilities/Expect.hpp"
#include "Beam/Utilities/YamlConfig.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Viper;

namespace {
  using ServiceLocatorServletContainer = ServiceProtocolServletContainer<
    MetaServiceLocatorServlet<CachedServiceLocatorDataStore<
      SqlServiceLocatorDataStore<SqlConnection<MySql::Connection>>>>,
    TcpServerSocket, BinarySender<SharedBuffer>, NullEncoder,
    std::shared_ptr<LiveTimer>>;
}

int main(int argc, const char** argv) {
  try {
    auto config = parse_command_line(argc, argv, "1.0-r" SERVICE_LOCATOR_VERSION
      "\nCopyright (C) 2026 Spire Trading Inc.");
    auto interface = extract<IpAddress>(config, "interface");
    auto mysql_config = try_or_nest([&] {
      return MySqlConfig::parse(get_node(config, "data_store"));
    }, std::runtime_error("Error parsing section 'data_store'."));
    auto server = ServiceLocatorServletContainer(
      init(init(init(make_sql_connection(MySql::Connection(
        mysql_config.m_address.get_host(), mysql_config.m_address.get_port(),
          mysql_config.m_username, mysql_config.m_password,
          mysql_config.m_schema))))), interface,
      std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10)));
    wait_for_kill_event();
  } catch(...) {
    report_current_exception();
    return -1;
  }
  return 0;
}
