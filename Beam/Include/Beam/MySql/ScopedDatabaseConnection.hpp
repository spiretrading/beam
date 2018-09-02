#ifndef BEAM_SCOPEDDATABASECONNECTION_HPP
#define BEAM_SCOPEDDATABASECONNECTION_HPP
#include <memory>
#include <boost/noncopyable.hpp>
#include <mysql++/connection.h>
#include "Beam/MySql/MySql.hpp"
#include "Beam/Pointers/Ref.hpp"

namespace Beam {
namespace MySql {

  /*! \class ScopedDatabaseConnection
      \brief Stores a MySql database connection acquired from a
             DatabaseConnectionPool.
   */
  class ScopedDatabaseConnection : private boost::noncopyable {
    public:

      //! Constructs a ScopedDatabaseConnection.
      ScopedDatabaseConnection(Ref<DatabaseConnectionPool> pool,
        std::unique_ptr<mysqlpp::Connection> connection);

      //! Acquires a ScopedDatabaseConnection.
      /*!
        \param connection The ScopedDatabaseConnection to acquire.
      */
      ScopedDatabaseConnection(ScopedDatabaseConnection&& connection);

      ~ScopedDatabaseConnection();

      //! Returns a reference to connection.
      mysqlpp::Connection& operator *() const;

      //! Returns a pointer to the connection.
      mysqlpp::Connection* operator ->() const;

    private:
      DatabaseConnectionPool* m_pool;
      std::unique_ptr<mysqlpp::Connection> m_connection;
  };
}
}

#endif
