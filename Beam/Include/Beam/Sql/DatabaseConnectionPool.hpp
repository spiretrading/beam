#ifndef BEAM_DATABASE_CONNECTION_POOL_HPP
#define BEAM_DATABASE_CONNECTION_POOL_HPP
#include <concepts>
#include <deque>
#include <memory>
#include <boost/thread/locks.hpp>
#include "Beam/Sql/ScopedDatabaseConnection.hpp"
#include "Beam/Threading/ConditionVariable.hpp"
#include "Beam/Threading/Mutex.hpp"

namespace Beam {

  /**
   * Provides a pool of SQL database connections for use in a multithreaded
   * database application.
   * @tparam C The type of SQL connection.
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
      template<std::invocable<> F> requires
        std::convertible_to<std::invoke_result_t<F>, std::unique_ptr<C>>
      DatabaseConnectionPool(int count, F&& builder);

      ~DatabaseConnectionPool();

      /** Acquires a database connection. */
      ScopedDatabaseConnection<Connection> load();

      /** Closes all database connections. */
      void close();

    private:
      friend class ScopedDatabaseConnection<Connection>;
      Mutex m_mutex;
      std::deque<std::unique_ptr<Connection>> m_connections;
      ConditionVariable m_connection_available_condition;

      DatabaseConnectionPool(const DatabaseConnectionPool&) = delete;
      DatabaseConnectionPool& operator =(
        const DatabaseConnectionPool&) = delete;
      void add(std::unique_ptr<Connection> connection);
  };

  template<typename C>
  template<std::invocable<> F> requires
    std::convertible_to<std::invoke_result_t<F>, std::unique_ptr<C>>
  DatabaseConnectionPool<C>::DatabaseConnectionPool(int count, F&& builder) {
    for(auto i = 0; i < count; ++i) {
      add(builder());
    }
  }

  template<typename C>
  DatabaseConnectionPool<C>::~DatabaseConnectionPool() {
    close();
  }

  template<typename C>
  ScopedDatabaseConnection<typename DatabaseConnectionPool<C>::Connection>
      DatabaseConnectionPool<C>::load() {
    auto lock = boost::unique_lock(m_mutex);
    while(m_connections.empty()) {
      m_connection_available_condition.wait(lock);
    }
    auto connection =
      ScopedDatabaseConnection(Ref(*this), std::move(m_connections.front()));
    m_connections.pop_front();
    return connection;
  }

  template<typename C>
  void DatabaseConnectionPool<C>::add(std::unique_ptr<Connection> connection) {
    auto lock = boost::unique_lock(m_mutex);
    m_connections.push_back(std::move(connection));
    m_connection_available_condition.notify_all();
  }

  template<typename C>
  void DatabaseConnectionPool<C>::close() {
    m_connections.clear();
  }

  template<typename C>
  ScopedDatabaseConnection<C>::ScopedDatabaseConnection(
    Ref<DatabaseConnectionPool<Connection>> pool,
    std::unique_ptr<Connection> connection)
    : m_pool(pool.get()),
      m_connection(std::move(connection)) {}

  template<typename C>
  ScopedDatabaseConnection<C>::ScopedDatabaseConnection(
    ScopedDatabaseConnection&& connection)
    : m_pool(connection.m_pool),
      m_connection(std::move(connection.m_connection)) {}

  template<typename C>
  ScopedDatabaseConnection<C>::~ScopedDatabaseConnection() {
    if(m_connection) {
      m_pool->add(std::move(m_connection));
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
