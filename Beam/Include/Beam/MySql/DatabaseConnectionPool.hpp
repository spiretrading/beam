#ifndef BEAM_DATABASECONNECTIONPOOL_HPP
#define BEAM_DATABASECONNECTIONPOOL_HPP
#include <deque>
#include <memory>
#include <boost/noncopyable.hpp>
#include <boost/thread/locks.hpp>
#include <mysql++/connection.h>
#include "Beam/MySql/MySql.hpp"
#include "Beam/MySql/ScopedDatabaseConnection.hpp"
#include "Beam/Threading/ConditionVariable.hpp"
#include "Beam/Threading/Mutex.hpp"

namespace Beam {
namespace MySql {

  /*! \class DatabaseConnectionPool
      \brief Provides a pool of MySql database connections for use in a
             multi threaded database application.
   */
  class DatabaseConnectionPool : private boost::noncopyable {
    public:

      //! Constructs an empty DatabaseConnectionPool.
      DatabaseConnectionPool();

      ~DatabaseConnectionPool();

      //! Acquires a database connection.
      ScopedDatabaseConnection Acquire();

      //! Adds a database connection to this pool.
      /*!
        \param connection The connection to add.
      */
      void Add(std::unique_ptr<mysqlpp::Connection> connection);

      //! Closes all database connections.
      void Close();

    private:
      Threading::Mutex m_mutex;
      std::deque<std::unique_ptr<mysqlpp::Connection>> m_connections;
      Threading::ConditionVariable m_connectionAvailableCondition;
  };

  inline DatabaseConnectionPool::DatabaseConnectionPool() {}

  inline DatabaseConnectionPool::~DatabaseConnectionPool() {
    Close();
  }

  inline ScopedDatabaseConnection DatabaseConnectionPool::Acquire() {
    boost::unique_lock<Threading::Mutex> lock(m_mutex);
    while(m_connections.empty()) {
      m_connectionAvailableCondition.wait(lock);
    }
    ScopedDatabaseConnection connection(Ref(*this),
      std::move(m_connections.front()));
    m_connections.pop_front();
    return connection;
  }

  inline void DatabaseConnectionPool::Add(
      std::unique_ptr<mysqlpp::Connection> connection) {
    boost::unique_lock<Threading::Mutex> lock(m_mutex);
    m_connections.push_back(std::move(connection));
    m_connectionAvailableCondition.notify_all();
  }

  inline void DatabaseConnectionPool::Close() {
    for(std::unique_ptr<mysqlpp::Connection>& connection : m_connections) {
      connection->disconnect();
    }
    m_connections.clear();
  }

  inline ScopedDatabaseConnection::ScopedDatabaseConnection(
      Ref<DatabaseConnectionPool> pool,
      std::unique_ptr<mysqlpp::Connection> connection)
      : m_pool(pool.Get()),
        m_connection(std::move(connection)) {}

  inline ScopedDatabaseConnection::ScopedDatabaseConnection(
      ScopedDatabaseConnection&& connection)
      : m_pool(std::move(connection.m_pool)),
        m_connection(std::move(connection.m_connection)) {}

  inline ScopedDatabaseConnection::~ScopedDatabaseConnection() {
    if(m_connection != nullptr) {
      m_pool->Add(std::move(m_connection));
    }
  }

  inline mysqlpp::Connection& ScopedDatabaseConnection::operator *() const {
    return *m_connection;
  }

  inline mysqlpp::Connection* ScopedDatabaseConnection::operator ->() const {
    return m_connection.get();
  }
}
}

#endif
