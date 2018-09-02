#ifndef BEAM_SQL_DATA_STORE_HPP
#define BEAM_SQL_DATA_STORE_HPP
#include <algorithm>
#include <iterator>
#include <vector>
#include <boost/noncopyable.hpp>
#include <Viper/Viper.hpp>
#include "Beam/MySql/DatabaseConnectionPool.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queries/IndexedValue.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/SequencedValue.hpp"
#include "Beam/Queries/SqlTranslator.hpp"
#include "Beam/Queries/SqlUtilities.hpp"
#include "Beam/Threading/Sync.hpp"
#include "Beam/Threading/ThreadPool.hpp"

namespace Beam::Queries {

  /** Loads and stores SequencedValue's in an SQL database.
      \tparam C The type of SQL connection to query.
      \tparam V The type of SQL row to store the value.
      \tparam I The type of SQL row to store the index.
      \tparam F The callable used to build the index expression.
      \tparam T The type of SqlTranslator used for filtering values.
   */
  template<typename C, typename V, typename I, typename F, typename T>
  class SqlDataStore : private boost::noncopyable {
    public:

      //! The type of SQL connection to query.
      using Connection = C;

      //! The type of SQL row to store the value.
      using ValueRow = V;

      //! The type of SQL row to store the index.
      using IndexRow = I;

      //! The callable used to build the index expression.
      using IndexBuilder = F;

      //! The type of SqlTranslator used for filtering values.
      using SqlTranslator = T;

      //! The type of value to store.
      using Value = typename ValueRow::Type;

      //! The type of index.
      using Index = typename IndexRow::Type;

      //! The type of query used to load values.
      using Query = BasicQuery<Index>;

      //! The SequencedValue to store.
      using SequencedValue = ::Beam::Queries::SequencedValue<Value>;

      //! The IndexedValue to store.
      using IndexedValue = ::Beam::Queries::SequencedValue<
        ::Beam::Queries::IndexedValue<Value, Index>>;

      //! Constructs an SqlDataStore.
      /*!
        \param table The name of the SQL table.
        \param valueRow The SQL row to store the value.
        \param indexRow The SQL row to store the index.
        \param indexBuilder The callable used to build the index expression.
        \param connectionPool The pool used to acquire SQL connections.
        \param writeConnection Used to perform high priority writes.
        \param threadPool Used to perform asynchronous reads and writes.
      */
      SqlDataStore(std::string table, ValueRow valueRow, IndexRow indexRow,
        IndexBuilder indexBuilder,
        RefType<DatabaseConnectionPool<Connection>> connectionPool,
        RefType<Threading::Sync<Connection, Threading::Mutex>> writeConnection,
        RefType<Threading::ThreadPool> threadPool);

      //! Constructs an SqlDataStore.
      /*!
        \param table The name of the SQL table.
        \param valueRow The SQL row to store the value.
        \param indexRow The SQL row to store the index.
        \param connectionPool The pool used to acquire SQL connections.
        \param writeConnection Used to perform high priority writes.
        \param threadPool Used to perform asynchronous reads and writes.
      */
      SqlDataStore(std::string table, ValueRow valueRow, IndexRow indexRow,
        RefType<DatabaseConnectionPool<Connection>> connectionPool,
        RefType<Threading::Sync<Connection, Threading::Mutex>> writeConnection,
        RefType<Threading::ThreadPool> threadPool);

      ~SqlDataStore();

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
      std::vector<SequencedValue> Load(const Viper::Expression& query);

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

      void Open();

      void Close();

    private:
      std::string m_table;
      ValueRow m_valueRow;
      IndexRow m_indexRow;
      Viper::Row<IndexedValue> m_row;
      Viper::Row<SequencedValue> m_sequencedRow;
      IndexBuilder m_indexBuilder;
      DatabaseConnectionPool<Connection>* m_connectionPool;
      Threading::Sync<Connection, Threading::Mutex>* m_writeConnection;
      Threading::ThreadPool* m_threadPool;
  };

  template<typename C, typename V, typename I, typename F, typename T>
  SqlDataStore<C, V, I, F, T>::SqlDataStore(std::string table,
      ValueRow valueRow, IndexRow indexRow, IndexBuilder indexBuilder,
      RefType<DatabaseConnectionPool<Connection>> connectionPool,
      RefType<Threading::Sync<Connection, Threading::Mutex>> writeConnection,
      RefType<Threading::ThreadPool> threadPool)
      : m_table(std::move(table)),
        m_valueRow(std::move(valueRow)),
        m_indexRow(std::move(indexRow)),
        m_indexBuilder(std::move(indexBuilder)),
        m_connectionPool(connectionPool.Get()),
        m_writeConnection(writeConnection.Get()),
        m_threadPool(threadPool.Get()) {
    m_valueRow = m_valueRow.
      add_column("timestamp",
        [] (auto& row) {
          return MySql::ToMySqlTimestamp(GetTimestamp(row));
        },
        [] (auto& row, auto value) {
          GetTimestamp(row) = MySql::FromMySqlTimestamp(value);
        });
    m_sequencedRow = Row<SequencedValue>().
      extend(m_valueRow,
        [] (auto& row) -> auto& {
          return row.GetValue();
        }).
      add_column("query_sequence",
        [] (auto& row) {
          return row.GetSequence().GetOrdinal();
        },
        [] (auto& row, auto value) {
          row.GetSequence() = Sequence(value);
        });
    m_row = Row<IndexedValue>().
      extend(m_indexRow,
        [] (auto& row) -> auto& {
          return row->GetIndex();
        }).
      extend(m_valueRow,
        [] (auto& row) -> auto& {
          return row->GetValue();
        }).
      add_column("query_sequence",
        [] (auto& row) {
          return row.GetSequence().GetOrdinal();
        },
        [] (auto& row, auto value) {
          row.GetSequence() = Sequence(value);
        });
  }

  template<typename C, typename V, typename I, typename F, typename T>
  SqlDataStore<C, V, I, F, T>::SqlDataStore(std::string table,
      ValueRow valueRow, IndexRow indexRow,
      RefType<DatabaseConnectionPool<Connection>> connectionPool,
      RefType<Threading::Sync<Connection, Threading::Mutex>> writeConnection,
      RefType<Threading::ThreadPool> threadPool)
      : SqlDataStore(std::move(table), std::move(valueRow), std::move(indexRow),
          {}, Ref(connectionPool), Ref(writeConnection), Ref(threadPool)) {}

  template<typename C, typename V, typename I, typename F, typename T>
  SqlDataStore<C, V, I, F, T>::~SqlDataStore() {
    Close();
  }

  template<typename C, typename V, typename I, typename F, typename T>
  std::vector<typename SqlDataStore<C, V, I, F, T>::SequencedValue>
      SqlDataStore<C, V, I, F, T>::Load(const Query& query) {
    return LoadSqlQuery<SqlTranslator>(query, m_sequencedRow, m_table,
      m_indexBuilder(query.GetIndex()), *m_threadPool, *m_connectionPool);
  }

  template<typename C, typename V, typename I, typename F, typename T>
  std::vector<typename SqlDataStore<C, V, I, F, T>::SequencedValue>
      SqlDataStore<C, V, I, F, T>::Load(const Viper::Expression& query) {
    return LoadSqlQuery<SequencedValue, Row>(query, m_sequencedRow, m_table,
      *m_threadPool, *m_connectionPool);
  }

  template<typename C, typename V, typename I, typename F, typename T>
  void SqlDataStore<C, V, I, F, T>::Store(const IndexedValue& value) {
    Threading::With(*m_writeConnection,
      [&] (auto& connection) {
        connection.execute(Viper::insert(m_row, m_table, &value));
      });
  }

  template<typename C, typename V, typename I, typename F, typename T>
  void SqlDataStore<C, V, I, F, T>::Store(
      const std::vector<IndexedValue>& values) {
    Threading::With(*m_writeConnection,
      [&] (auto& connection) {
        connection.execute(Viper::insert(m_row, m_table, values.begin(),
          values.end()));
      });
  }

  template<typename C, typename V, typename I, typename F, typename T>
  void SqlDataStore<C, V, I, F, T>::Open() {
    m_writeConnection->With(
      [&] (auto& connection) {
        connection.open();
        connection.execute(create_if_not_exists(m_row, m_table));
      });
  }

  template<typename C, typename V, typename I, typename F, typename T>
  void SqlDataStore<C, V, I, F, T>::Close() {}
}

#endif
