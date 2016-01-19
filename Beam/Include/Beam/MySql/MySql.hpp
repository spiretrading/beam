#ifndef BEAM_MYSQL_HPP
#define BEAM_MYSQL_HPP
#ifdef _MSC_VER
  #include <winsock2.h>
  #include <boost/asio.hpp>
  #include <windows.h>
#endif

namespace Beam {
namespace MySql {
  class DatabaseConnectionPool;
  struct MySqlConfig;
  class ScopedDatabaseConnection;
}
}

#endif
