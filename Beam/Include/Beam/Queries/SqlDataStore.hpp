#ifndef BEAM_SQLDATASTORE_HPP
#define BEAM_SQLDATASTORE_HPP
#include <algorithm>
#include <iterator>
#include <vector>
#include <boost/noncopyable.hpp>
#include <mysql++/connection.h>
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

  /*! \class SqlDataStore
      \brief Loads and stores SequencedValue's in an SQL database.
      \tparam QueryType The type of query used to load values.
      \tparam ValueType The value to store.
      \tparam RowType The type of row representing the values.
      \tparam SqlTranslatorFilterType The type of SqlTranslator used for
              filtering values.
      \tparam FunctorType The object containing functions to build index queries
              and translate to/from value types to row types.
   */
  template<typename QueryType, typename ValueType, typename RowType,
    typename SqlTranslatorFilterType, typename FunctorType>
  class SqlDataStore : private boost::noncopyable {
    public:

      //! The type of query used to load values.
      using Query = QueryType;

      //! The type of index used.
      using Index = typename Query::Index;

      //! The type of value to store.
      using Value = ValueType;

      //! The SequencedValue to store.
      using SequencedValue = ::Beam::Queries::SequencedValue<Value>;

      //! The IndexedValue to store.
      using IndexedValue = ::Beam::Queries::SequencedValue<
        ::Beam::Queries::IndexedValue<Value, Index>>;

      //! The type of row representing the values.
      using Row = RowType;

      //! The type of SqlTranslator used for filtering values.
      using SqlTranslatorFilter = SqlTranslatorFilterType;

      //! The The object containing functions to build index queries and
      //! translate to/from value types to row types.
      using Functor = FunctorType;

      //! Constructs an SqlDataStore.
      /*!
        \param connectionPool The pool used to acquire SQL connections.
        \param writeConnection Used to perform high priority writes.
        \param threadPool Used to perform asynchronous reads and writes.
        \param functor Initializes the Functor.
      */
      SqlDataStore(Beam::RefType<MySql::DatabaseConnectionPool> connectionPool,
        Beam::RefType<Threading::Sync<mysqlpp::Connection, Threading::Mutex>>
        writeConnection, Beam::RefType<Threading::ThreadPool> threadPool,
        const Functor& functor = Functor());

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
      MySql::DatabaseConnectionPool* m_connectionPool;
      Threading::Sync<mysqlpp::Connection, Threading::Mutex>* m_writeConnection;
      Threading::ThreadPool* m_threadPool;
      Functor m_functor;
      std::string m_table;
  };

  template<typename QueryType, typename ValueType, typename RowType,
    typename SqlTranslatorFilterType, typename FunctorType>
  SqlDataStore<QueryType, ValueType, RowType, SqlTranslatorFilterType,
      FunctorType>::SqlDataStore(
      Beam::RefType<MySql::DatabaseConnectionPool> connectionPool,
      Beam::RefType<Threading::Sync<mysqlpp::Connection, Threading::Mutex>>
      writeConnection, Beam::RefType<Threading::ThreadPool> threadPool,
      const Functor& functor)
      : m_connectionPool{connectionPool.Get()},
        m_writeConnection{writeConnection.Get()},
        m_threadPool{threadPool.Get()},
        m_functor{functor},
        m_table{Row().table()} {}

  template<typename QueryType, typename ValueType, typename RowType,
    typename SqlTranslatorFilterType, typename FunctorType>
  std::vector<typename SqlDataStore<QueryType, ValueType, RowType,
      SqlTranslatorFilterType, FunctorType>::SequencedValue>
      SqlDataStore<QueryType, ValueType, RowType, SqlTranslatorFilterType,
      FunctorType>::Load(const Query& query) {
    return LoadSqlQuery<SequencedValue, Row, SqlTranslatorFilter>(query,
      m_table, m_functor(query.GetIndex()), *m_threadPool, *m_connectionPool,
      m_functor);
  }

  template<typename QueryType, typename ValueType, typename RowType,
    typename SqlTranslatorFilterType, typename FunctorType>
  std::vector<typename SqlDataStore<QueryType, ValueType, RowType,
      SqlTranslatorFilterType, FunctorType>::SequencedValue>
      SqlDataStore<QueryType, ValueType, RowType, SqlTranslatorFilterType,
      FunctorType>::Load(const std::string& query) {
    return LoadSqlQuery<SequencedValue, Row>(query, m_table, *m_threadPool,
      *m_connectionPool, m_functor);
  }

  template<typename QueryType, typename ValueType, typename RowType,
    typename SqlTranslatorFilterType, typename FunctorType>
  void SqlDataStore<QueryType, ValueType, RowType, SqlTranslatorFilterType,
      FunctorType>::Store(const IndexedValue& value) {
    auto row = m_functor(value);
    Threading::With(*m_writeConnection,
      [&] (mysqlpp::Connection& connection) {
        auto query = connection.query();
        query.insert(row);
        query.execute();
      });
  }

  template<typename QueryType, typename ValueType, typename RowType,
    typename SqlTranslatorFilterType, typename FunctorType>
  void SqlDataStore<QueryType, ValueType, RowType, SqlTranslatorFilterType,
      FunctorType>::Store(const std::vector<IndexedValue>& values) {
    const auto MAX_ROW_WRITE = 500;
    if(values.empty()) {
      return;
    }
    std::vector<Row> rows;
    std::transform(values.begin(), values.end(), std::back_inserter(rows),
      m_functor);
    Threading::With(*m_writeConnection,
      [&] (mysqlpp::Connection& connection) {
        auto insertStart = rows.begin();
        while(insertStart != rows.end()) {
          auto query = connection.query();
          if(std::distance(insertStart, rows.end()) > MAX_ROW_WRITE) {
            query.insert(insertStart, insertStart + MAX_ROW_WRITE);
            insertStart = insertStart + MAX_ROW_WRITE;
          } else {
            query.insert(insertStart, rows.end());
            insertStart = rows.end();
          }
          query.execute();
        }
      });
  }
}
}

#endif
