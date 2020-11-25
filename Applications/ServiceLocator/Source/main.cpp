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
#include "Beam/Threading/LiveTimer.hpp"
#include "Beam/Utilities/ApplicationInterrupt.hpp"
#include "Beam/Utilities/Expect.hpp"
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
    auto config = ParseCommandLine(argc, argv, "1.0-r" SERVICE_LOCATOR_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc.");
    auto interface = Extract<IpAddress>(config, "interface");
    auto mySqlConfig = TryOrNest([&] {
      return MySqlConfig::Parse(GetNode(config, "data_store"));
    }, std::runtime_error("Error parsing section 'data_store'."));
    auto server = TryOrNest([&] {
      return ServiceLocatorServletContainer(Initialize(Initialize(Initialize(
        MakeSqlConnection(MySql::Connection(mySqlConfig.m_address.GetHost(),
        mySqlConfig.m_address.GetPort(), mySqlConfig.m_username,
        mySqlConfig.m_password, mySqlConfig.m_schema))))), interface,
        std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10)));
    }, std::runtime_error("Unable to open server."));
    WaitForKillEvent();
  } catch(...) {
    ReportCurrentException();
    return -1;
  }
  return 0;
}
