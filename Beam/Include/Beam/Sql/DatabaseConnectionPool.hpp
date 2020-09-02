#ifndef BEAM_DATABASE_CONNECTION_POOL_HPP
#define BEAM_DATABASE_CONNECTION_POOL_HPP
#include <deque>
#include <memory>
#include <boost/thread/locks.hpp>
#include "Beam/Sql/ScopedDatabaseConnection.hpp"
#include "Beam/Sql/Sql.hpp"
#include "Beam/Threading/ConditionVariable.hpp"
#include "Beam/Threading/Mutex.hpp"

namespace Beam {

  /**
   * Provides a pool of SQL database connections for use in a multithreaded
   * database application.
   * @param <C> The type of SQL connection.
   */
  template<typename C>
  class DatabaseConnectionPool {
    public:

      /** The type of SQL connection. */
      using Connection = C;

      /**
       * Constructs an empty DatabaseConnectionPool.
       * @param count The number of connections to pool.
       * @param builder The function used to build connections.
       */
      template<typename F>
      DatabaseConnectionPool(int count, F&& builder);

      ~DatabaseConnectionPool();

      /** Acquires a database connection. */
      ScopedDatabaseConnection<Connection> Acquire();

      /** Closes all database connections. */
      void Close();

    private:
      friend class ScopedDatabaseConnection<Connection>;
      Threading::Mutex m_mutex;
      std::deque<std::unique_ptr<Connection>> m_connections;
      Threading::ConditionVariable m_connectionAvailableCondition;

      DatabaseConnectionPool(const DatabaseConnectionPool&) = delete;
      DatabaseConnectionPool& operator =(
        const DatabaseConnectionPool&) = delete;
      void Add(std::unique_ptr<Connection> connection);
  };

  template<typename C>
  template<typename F>
  DatabaseConnectionPool<C>::DatabaseConnectionPool(int count, F&& builder) {
    for(auto i = 0; i < count; ++i) {
      Add(builder());
    }
  }

  template<typename C>
  DatabaseConnectionPool<C>::~DatabaseConnectionPool() {
    Close();
  }

  template<typename C>
  ScopedDatabaseConnection<typename DatabaseConnectionPool<C>::Connection>
      DatabaseConnectionPool<C>::Acquire() {
    auto lock = boost::unique_lock(m_mutex);
    while(m_connections.empty()) {
      m_connectionAvailableCondition.wait(lock);
    }
    auto connection = ScopedDatabaseConnection(Ref(*this),
      std::move(m_connections.front()));
    m_connections.pop_front();
    return connection;
  }

  template<typename C>
  void DatabaseConnectionPool<C>::Add(std::unique_ptr<Connection> connection) {
    auto lock = boost::unique_lock(m_mutex);
    m_connections.push_back(std::move(connection));
    m_connectionAvailableCondition.notify_all();
  }

  template<typename C>
  void DatabaseConnectionPool<C>::Close() {
    m_connections.clear();
  }

  template<typename C>
  ScopedDatabaseConnection<C>::ScopedDatabaseConnection(
    Ref<DatabaseConnectionPool<Connection>> pool,
    std::unique_ptr<Connection> connection)
    : m_pool(pool.Get()),
      m_connection(std::move(connection)) {}

  template<typename C>
  ScopedDatabaseConnection<C>::ScopedDatabaseConnection(
    ScopedDatabaseConnection&& connection)
    : m_pool(connection.m_pool),
      m_connection(std::move(connection.m_connection)) {}

  template<typename C>
  ScopedDatabaseConnection<C>::~ScopedDatabaseConnection() {
    if(m_connection) {
      m_pool->Add(std::move(m_connection));
    }
  }

  template<typename C>
  typename ScopedDatabaseConnection<C>::Connection&
      ScopedDatabaseConnection<C>::operator *() const {
    return *m_connection;
  }

  template<typename C>
  typename ScopedDatabaseConnection<C>::Connection*
      ScopedDatabaseConnection<C>::operator ->() const {
    return m_connection.get();
  }
}

#endif
