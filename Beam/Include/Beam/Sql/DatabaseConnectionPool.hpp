#ifndef BEAM_DATABASE_CONNECTION_POOL_HPP
#define BEAM_DATABASE_CONNECTION_POOL_HPP
#include <deque>
#include <memory>
#include <boost/noncopyable.hpp>
#include <boost/thread/locks.hpp>
#include "Beam/Sql/ScopedDatabaseConnection.hpp"
#include "Beam/Sql/Sql.hpp"
#include "Beam/Threading/ConditionVariable.hpp"
#include "Beam/Threading/Mutex.hpp"

namespace Beam {

  /** Provides a pool of SQL database connections for use in a multithreaded
      database application.
      \tparam ConnectionType The type of SQL connection.
   */
  template<typename ConnectionType>
  class DatabaseConnectionPool : private boost::noncopyable {
    public:

      //! The type of SQL connection.
      using Connection = ConnectionType;

      //! Constructs an empty DatabaseConnectionPool.
      DatabaseConnectionPool() = default;

      ~DatabaseConnectionPool();

      //! Acquires a database connection.
      ScopedDatabaseConnection<Connection> Acquire();

      //! Adds a database connection to this pool.
      /*!
        \param connection The connection to add.
      */
      void Add(std::unique_ptr<Connection> connection);

      //! Closes all database connections.
      void Close();

    private:
      Threading::Mutex m_mutex;
      std::deque<std::unique_ptr<Connection>> m_connections;
      Threading::ConditionVariable m_connectionAvailableCondition;
  };

  template<typename ConnectionType>
  DatabaseConnectionPool<ConnectionType>::~DatabaseConnectionPool() {
    Close();
  }

  template<typename ConnectionType>
  ScopedDatabaseConnection<
      typename DatabaseConnectionPool<ConnectionType>::Connection>
      DatabaseConnectionPool<ConnectionType>::Acquire() {
    auto lock = boost::unique_lock(m_mutex);
    while(m_connections.empty()) {
      m_connectionAvailableCondition.wait(lock);
    }
    auto connection = ScopedDatabaseConnection(Ref(*this),
      std::move(m_connections.front()));
    m_connections.pop_front();
    return connection;
  }

  template<typename ConnectionType>
  void DatabaseConnectionPool<ConnectionType>::Add(
      std::unique_ptr<Connection> connection) {
    auto lock = boost::unique_lock(m_mutex);
    m_connections.push_back(std::move(connection));
    m_connectionAvailableCondition.notify_all();
  }

  template<typename ConnectionType>
  void DatabaseConnectionPool<ConnectionType>::Close() {
    m_connections.clear();
  }

  template<typename ConnectionType>
  ScopedDatabaseConnection<ConnectionType>::ScopedDatabaseConnection(
      Ref<DatabaseConnectionPool<Connection>> pool,
      std::unique_ptr<Connection> connection)
      : m_pool(pool.Get()),
        m_connection(std::move(connection)) {}

  template<typename ConnectionType>
  ScopedDatabaseConnection<ConnectionType>::ScopedDatabaseConnection(
      ScopedDatabaseConnection&& connection)
      : m_pool(connection.m_pool),
        m_connection(std::move(connection.m_connection)) {}

  template<typename ConnectionType>
  ScopedDatabaseConnection<ConnectionType>::~ScopedDatabaseConnection() {
    if(m_connection != nullptr) {
      m_pool->Add(std::move(m_connection));
    }
  }

  template<typename ConnectionType>
  typename ScopedDatabaseConnection<ConnectionType>::Connection&
      ScopedDatabaseConnection<ConnectionType>::operator *() const {
    return *m_connection;
  }

  template<typename ConnectionType>
  typename ScopedDatabaseConnection<ConnectionType>::Connection*
      ScopedDatabaseConnection<ConnectionType>::operator ->() const {
    return m_connection.get();
  }
}

#endif
