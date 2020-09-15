#ifndef BEAM_SCOPED_DATABASE_CONNECTION_HPP
#define BEAM_SCOPED_DATABASE_CONNECTION_HPP
#include <memory>
#include "Beam/Sql/Sql.hpp"
#include "Beam/Pointers/Ref.hpp"

namespace Beam {

  /**
   * Stores an SQL database connection acquired from a DatabaseConnectionPool.
   * @param C The type of SQL connection.
   */
  template<typename C>
  class ScopedDatabaseConnection {
    public:

      /** The type of SQL connection. */
      using Connection = C;

      /** Constructs a ScopedDatabaseConnection. */
      ScopedDatabaseConnection(Ref<DatabaseConnectionPool<Connection>> pool,
        std::unique_ptr<Connection> connection);

      /** Moves a ScopedDatabaseConnection. */
      ScopedDatabaseConnection(ScopedDatabaseConnection&& connection);

      ~ScopedDatabaseConnection();

      /** Returns a reference to connection. */
      Connection& operator *() const;

      /** Returns a pointer to the connection. */
      Connection* operator ->() const;

    private:
      DatabaseConnectionPool<Connection>* m_pool;
      std::unique_ptr<Connection> m_connection;

      ScopedDatabaseConnection(const ScopedDatabaseConnection&) = delete;
      ScopedDatabaseConnection& operator =(
        const ScopedDatabaseConnection&) = delete;
  };
}

#endif
