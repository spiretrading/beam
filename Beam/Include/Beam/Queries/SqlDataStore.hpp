#ifndef BEAM_SQL_DATA_STORE_HPP
#define BEAM_SQL_DATA_STORE_HPP
#include <algorithm>
#include <iterator>
#include <vector>
#include <boost/noncopyable.hpp>
#include <Viper/Viper.hpp>
#include "Beam/MySql/DatabaseConnectionPool.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Queries/IndexedValue.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/SequencedValue.hpp"
#include "Beam/Queries/SqlTranslator.hpp"
#include "Beam/Queries/SqlUtilities.hpp"
#include "Beam/Threading/Sync.hpp"
#include "Beam/Threading/ThreadPool.hpp"

namespace Beam {
namespace Queries {

  /** Loads and stores SequencedValue's in an SQL database.
      \tparam C The type of SQL connection to query.
      \tparam Q The type of query used to load values.
      \tparam R The type of SQL row.
      \tparam T The type of SqlTranslator used for filtering values.
   */
  template<typename C, typename Q, typename R, typename T>
  class SqlDataStore : private boost::noncopyable {
    public:

      //! The type of SQL connection to query.
      using Connection = C;

      //! The type of query used to load values.
      using Query = Q;

      //! The type of SQL row.
      using Row = R;

      //! The type of value to store.
      using Value = typename Row::Type::Value::Value;

      //! The type of index used.
      using Index = typename Query::Index;

      //! The SequencedValue to store.
      using SequencedValue = ::Beam::Queries::SequencedValue<Value>;

      //! The IndexedValue to store.
      using IndexedValue = ::Beam::Queries::SequencedValue<
        ::Beam::Queries::IndexedValue<Value, Index>>;

      //! The type of SqlTranslator used for filtering values.
      using SqlTranslatorFilter = T;

      //! Constructs an SqlDataStore.
      /*!
        \param row The type of SQL row.
        \param table The name of the SQL table.
        \param connectionPool The pool used to acquire SQL connections.
        \param writeConnection Used to perform high priority writes.
        \param threadPool Used to perform asynchronous reads and writes.
      */
      SqlDataStore(Row row, std::string table,
        RefType<DatabaseConnectionPool<Connection>> connectionPool,
        RefType<Threading::Sync<Connection, Threading::Mutex>> writeConnection,
        RefType<Threading::ThreadPool> threadPool);

      //! Executes a search query.
      /*!
        \param query The search query to execute.
        \return The list of the values that satisfy the search <i>query</i>.
      */
      std::vector<SequencedValue> Load(const Query& query);

      //! Executes a search query.
      /*!
        \param query The search query to execute.
        \return The list of the values that satisfy the search <i>query</i>.
      */
      std::vector<SequencedValue> Load(const std::string& query);

      //! Stores a Value.
      /*!
        \param value The Value to store.
      */
      void Store(const IndexedValue& value);

      //! Stores a list of Values.
      /*!
        \param values The list of Values to store.
      */
      void Store(const std::vector<IndexedValue>& values);

    private:
      Row m_row;
      std::string m_table;
      DatabaseConnectionPool<Connection>* m_connectionPool;
      Threading::Sync<Connection, Threading::Mutex>* m_writeConnection;
      Threading::ThreadPool* m_threadPool;
  };

  template<typename C, typename Q, typename R, typename T>
  SqlDataStore<C, Q, R, T>::SqlDataStore(Row row, std::string table,
      RefType<DatabaseConnectionPool<Connection>> connectionPool,
      RefType<Threading::Sync<Connection, Threading::Mutex>> writeConnection,
      RefType<Threading::ThreadPool> threadPool)
      : m_row(std::move(row)),
        m_table(std::move(table)),
        m_connectionPool(connectionPool.Get()),
        m_writeConnection(writeConnection.Get()),
        m_threadPool(threadPool.Get()) {}

  template<typename C, typename Q, typename R, typename T>
  std::vector<typename SqlDataStore<C, Q, R, T>::SequencedValue>
      SqlDataStore<C, Q, R, T>::Load(const Query& query) {
    return {};
/*
    return LoadSqlQuery<SequencedValue, Row, SqlTranslatorFilter>(query,
      m_table, m_functor(query.GetIndex()), *m_threadPool, *m_connectionPool,
      m_functor);
*/
  }

  template<typename C, typename Q, typename R, typename T>
  std::vector<typename SqlDataStore<C, Q, R, T>::SequencedValue>
      SqlDataStore<C, Q, R, T>::Load(const std::string& query) {
    return {};
//    return LoadSqlQuery<SequencedValue, Row>(query, m_table, *m_threadPool,
//      *m_connectionPool, m_functor);
  }

  template<typename C, typename Q, typename R, typename T>
  void SqlDataStore<C, Q, R, T>::Store(const IndexedValue& value) {
    Threading::With(*m_writeConnection,
      [&] (auto& connection) {
        connection.execute(Viper::insert(m_row, m_table, &value, &value + 1));
      });
  }

  template<typename C, typename Q, typename R, typename T>
  void SqlDataStore<C, Q, R, T>::Store(
      const std::vector<IndexedValue>& values) {
    Threading::With(*m_writeConnection,
      [&] (auto& connection) {
        connection.execute(Viper::insert(m_row, m_table, values.begin(),
          values.end()));
      });
  }
}
}

#endif
