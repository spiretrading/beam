#ifndef BEAM_SQL_CONNECTION_HPP
#define BEAM_SQL_CONNECTION_HPP
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <Viper/CommitStatement.hpp>
#include <Viper/CreateTableStatement.hpp>
#include <Viper/DeleteStatement.hpp>
#include <Viper/ExecuteException.hpp>
#include <Viper/InsertRangeStatement.hpp>
#include <Viper/RollbackStatement.hpp>
#include <Viper/SelectStatement.hpp>
#include <Viper/StartTransactionStatement.hpp>
#include <Viper/UpdateStatement.hpp>
#include <Viper/UpsertStatement.hpp>
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Threading/ThreadPool.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /**
   * Wraps a Viper::Connection and parks any blocking calls.
   * @tparam C The Viper::Connection to wrap.
   */
  template<typename C>
  class SqlConnection {
    public:

      /**
       * Constructs an SqlConnection.
       * @param connection The Connection to wrap.
       */
      template<Initializes<C> CF>
      explicit SqlConnection(CF&& connection);

      SqlConnection(SqlConnection&&) = default;
      ~SqlConnection();

      bool has_table(std::string_view name);
      void execute(std::string_view statement);
      template<typename T>
      void execute(const Viper::CreateTableStatement<T>& statement);
      void execute(const Viper::DeleteStatement& statement);
      template<typename T, typename B, typename E>
      void execute(const Viper::InsertRangeStatement<T, B, E>& statement);
      void execute(const Viper::UpdateStatement& statement);
      template<typename R, typename B, typename E>
      void execute(const Viper::UpsertStatement<R, B, E>& statement);
      template<typename T, typename D>
      void execute(const Viper::SelectStatement<T, D>& statement);
      void execute(const Viper::StartTransactionStatement& statement);
      void execute(const Viper::CommitStatement& statement);
      void execute(const Viper::RollbackStatement& statement);
      void open();
      void close();
      SqlConnection& operator =(SqlConnection&&) = default;

    private:
      local_ptr_t<C> m_connection;

      SqlConnection(const SqlConnection&) = delete;
      SqlConnection& operator =(const SqlConnection&) = delete;
  };

  template<typename C>
  SqlConnection(C&& connection) -> SqlConnection<std::remove_cvref_t<C>>;

  /**
   * Makes an SqlConnection wrapping a Viper::Connection.
   * @param connection The Viper::Connection to wrap.
   */
  template<typename Connection>
  std::unique_ptr<SqlConnection<Connection>> make_sql_connection(
      Connection&& connection) {
    return std::make_unique<SqlConnection<Connection>>(
      std::forward<Connection>(connection));
  }

  template<typename C>
  template<Initializes<C> CF>
  SqlConnection<C>::SqlConnection(CF&& connection)
    : m_connection(std::forward<CF>(connection)) {}

  template<typename C>
  SqlConnection<C>::~SqlConnection() {
    close();
  }

  template<typename C>
  void SqlConnection<C>::execute(std::string_view statement) {
    park([&] {
      m_connection->execute(statement);
    });
  }

  template<typename C>
  bool SqlConnection<C>::has_table(std::string_view name) {
    return park([&] {
      return m_connection->has_table(name);
    });
  }

  template<typename C>
  template<typename T>
  void SqlConnection<C>::execute(
      const Viper::CreateTableStatement<T>& statement) {
    park([&] {
      m_connection->execute(statement);
    });
  }

  template<typename C>
  void SqlConnection<C>::execute(const Viper::DeleteStatement& statement) {
    park([&] {
      m_connection->execute(statement);
    });
  }

  template<typename C>
  template<typename T, typename B, typename E>
  void SqlConnection<C>::execute(
      const Viper::InsertRangeStatement<T, B, E>& statement) {
    park([&] {
      m_connection->execute(statement);
    });
  }

  template<typename C>
  void SqlConnection<C>::execute(const Viper::UpdateStatement& statement) {
    park([&] {
      m_connection->execute(statement);
    });
  }

  template<typename C>
  template<typename T, typename B, typename E>
  void SqlConnection<C>::execute(
      const Viper::UpsertStatement<T, B, E>& statement) {
    park([&] {
      m_connection->execute(statement);
    });
  }

  template<typename C>
  template<typename T, typename D>
  void SqlConnection<C>::execute(
      const Viper::SelectStatement<T, D>& statement) {
    park([&] {
      m_connection->execute(statement);
    });
  }

  template<typename C>
  void SqlConnection<C>::execute(
      const Viper::StartTransactionStatement& statement) {
    park([&] {
      m_connection->execute(statement);
    });
  }

  template<typename C>
  void SqlConnection<C>::execute(const Viper::CommitStatement& statement) {
    park([&] {
      m_connection->execute(statement);
    });
  }

  template<typename C>
  void SqlConnection<C>::execute(const Viper::RollbackStatement& statement) {
    park([&] {
      m_connection->execute(statement);
    });
  }

  template<typename C>
  void SqlConnection<C>::open() {
    park([&] {
      m_connection->open();
    });
  }

  template<typename C>
  void SqlConnection<C>::close() {
    park([&] {
      m_connection->close();
    });
  }
}

#endif
