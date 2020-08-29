#ifndef BEAM_SQL_DATA_STORE_HPP
#define BEAM_SQL_DATA_STORE_HPP
#include <algorithm>
#include <iterator>
#include <vector>
#include <boost/noncopyable.hpp>
#include <Viper/Viper.hpp>
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queries/IndexedValue.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/SequencedValue.hpp"
#include "Beam/Queries/SqlTranslator.hpp"
#include "Beam/Queries/SqlUtilities.hpp"
#include "Beam/Sql/DatabaseConnectionPool.hpp"
#include "Beam/Sql/PosixTimeToSqlDateTime.hpp"
#include "Beam/Threading/ThreadPool.hpp"

namespace Beam::Queries {

  /** Loads and stores SequencedValue's in an SQL database.
      \tparam C The type of SQL connection to query.
      \tparam V The type of SQL row to store the value.
      \tparam I The type of SQL row to store the index.
      \tparam T The type of SqlTranslator used for filtering values.
   */
  template<typename C, typename V, typename I, typename T>
  class SqlDataStore : private boost::noncopyable {
    public:

      //! The type of SQL connection to query.
      using Connection = C;

      //! The type of SQL row to store the value.
      using ValueRow = V;

      //! The type of SQL row to store the index.
      using IndexRow = I;

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
        \param readerPool The pool of SQL connections used for reading.
        \param writerPool The pool of SQL connections used for writing.
        \param threadPool Used to perform asynchronous reads and writes.
      */
      SqlDataStore(std::string table, ValueRow valueRow, IndexRow indexRow,
        Ref<DatabaseConnectionPool<Connection>> readerPool,
        Ref<DatabaseConnectionPool<Connection>> writerPool,
        Ref<Threading::ThreadPool> threadPool);

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

      void Close();

    private:
      std::string m_table;
      ValueRow m_valueRow;
      IndexRow m_indexRow;
      Viper::Row<IndexedValue> m_row;
      Viper::Row<SequencedValue> m_sequencedRow;
      DatabaseConnectionPool<Connection>* m_readerPool;
      DatabaseConnectionPool<Connection>* m_writerPool;
      Threading::ThreadPool* m_threadPool;
  };

  template<typename C, typename V, typename I, typename T>
  SqlDataStore<C, V, I, T>::SqlDataStore(std::string table, ValueRow valueRow,
      IndexRow indexRow, Ref<DatabaseConnectionPool<Connection>> readerPool,
      Ref<DatabaseConnectionPool<Connection>> writerPool,
      Ref<Threading::ThreadPool> threadPool)
      : m_table(std::move(table)),
        m_valueRow(std::move(valueRow)),
        m_indexRow(std::move(indexRow)),
        m_readerPool(readerPool.Get()),
        m_writerPool(writerPool.Get()),
        m_threadPool(threadPool.Get()) {
    m_valueRow = m_valueRow.
      add_column("timestamp",
        [] (const auto& row) {
          return GetTimestamp(row);
        },
        [] (auto& row, auto value) {
          GetTimestamp(row) = value;
        });
    m_sequencedRow = Viper::Row<SequencedValue>().
      extend(m_valueRow,
        [] (auto& row) -> auto& {
          return row.GetValue();
        }).
      add_column("query_sequence",
        [] (const auto& row) {
          return row.GetSequence().GetOrdinal();
        },
        [] (auto& row, auto value) {
          row.GetSequence() = Sequence(value);
        });
    auto isIndexEmbedded = false;
    for(auto& indexColumn : m_indexRow.get_columns()) {
      for(auto& valueColumn : m_valueRow.get_columns()) {
        if(indexColumn.m_name == valueColumn.m_name) {
          isIndexEmbedded = true;
          break;
        }
      }
    }
    if(!isIndexEmbedded) {
      m_row = m_row.extend(m_indexRow,
        [] (auto& row) -> auto& {
          return row->GetIndex();
        });
    }
    m_row = m_row.
      extend(m_valueRow,
        [] (auto& row) -> auto& {
          return row->GetValue();
        }).
      add_column("query_sequence",
        [] (const auto& row) {
          return row.GetSequence().GetOrdinal();
        },
        [] (auto& row, auto value) {
          row.GetSequence() = Sequence(value);
        });
    auto index = std::vector<std::string>();
    for(auto& column : m_indexRow.get_columns()) {
      index.push_back(column.m_name);
    }
    auto sequence_index = index;
    sequence_index.push_back("query_sequence");
    m_row = m_row.add_index("sequence_index", std::move(sequence_index));
    auto timestamp_index = index;
    timestamp_index.push_back("query_sequence");
    timestamp_index.push_back("timestamp");
    m_row = m_row.add_index("timestamp_index", std::move(timestamp_index));
    auto result =  Routines::Async<void>();
    auto connection = m_writerPool->Acquire();
    m_threadPool->Queue(
      [&] {
        connection->execute(create_if_not_exists(m_row, m_table));
      }, result.GetEval());
    result.Get();
  }

  template<typename C, typename V, typename I, typename T>
  SqlDataStore<C, V, I, T>::~SqlDataStore() {
    Close();
  }

  template<typename C, typename V, typename I, typename T>
  std::vector<typename SqlDataStore<C, V, I, T>::SequencedValue>
      SqlDataStore<C, V, I, T>::Load(const Query& query) {
    std::optional<Viper::Expression> index;
    std::string column;
    for(auto i = std::size_t(0); i != m_indexRow.get_columns().size(); ++i) {
      m_indexRow.append_value(query.GetIndex(), i, column);
      auto term = Viper::sym(m_indexRow.get_columns()[i].m_name) ==
        Viper::sym(column);
      if(index.has_value()) {
        *index = *index && term;
      } else {
        index.emplace(std::move(term));
      }
      column.clear();
    }
    if(!index.has_value()) {
      index.emplace();
    }
    return LoadSqlQuery<SqlTranslator>(query, m_sequencedRow, m_table, *index,
      *m_threadPool, *m_readerPool);
  }

  template<typename C, typename V, typename I, typename T>
  std::vector<typename SqlDataStore<C, V, I, T>::SequencedValue>
      SqlDataStore<C, V, I, T>::Load(const Viper::Expression& query) {
    return LoadSqlQuery(query, m_sequencedRow, m_table, *m_threadPool,
      *m_readerPool);
  }

  template<typename C, typename V, typename I, typename T>
  void SqlDataStore<C, V, I, T>::Store(const IndexedValue& value) {
    auto result =  Routines::Async<void>();
    auto connection = m_writerPool->Acquire();
    m_threadPool->Queue(
      [&] {
        connection->execute(Viper::insert(m_row, m_table, &value));
      }, result.GetEval());
    result.Get();
  }

  template<typename C, typename V, typename I, typename T>
  void SqlDataStore<C, V, I, T>::Store(
      const std::vector<IndexedValue>& values) {
    auto result =  Routines::Async<void>();
    auto connection = m_writerPool->Acquire();
    m_threadPool->Queue(
      [&] {
        connection->execute(Viper::insert(m_row, m_table, values.begin(),
          values.end()));
      }, result.GetEval());
    result.Get();
  }

  template<typename C, typename V, typename I, typename T>
  void SqlDataStore<C, V, I, T>::Close() {}
}

#endif
