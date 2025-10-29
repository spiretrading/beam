#ifndef BEAM_SQL_DATA_STORE_HPP
#define BEAM_SQL_DATA_STORE_HPP
#include <algorithm>
#include <iterator>
#include <vector>
#include <Viper/Viper.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/IO/ConnectException.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queries/IndexedValue.hpp"
#include "Beam/Queries/SequencedValue.hpp"
#include "Beam/Queries/SqlTranslator.hpp"
#include "Beam/Queries/SqlUtilities.hpp"
#include "Beam/Sql/DatabaseConnectionPool.hpp"
#include "Beam/Sql/PosixTimeToSqlDateTime.hpp"

namespace Beam {

  /** Specifies whether to create the table on connection. */
  enum class SqlConnectionOption {

    /** Perform no action if the table doesn't exist. */
    NONE,

    /** Raise an exception if the table doesn't exist. */
    ENSURE,

    /** Create the table if it doesn't exist (default option). */
    CREATE
  };

  /**
   * Loads and stores SequencedValue's in an SQL database.
   * @tparam C The type of SQL connection to query.
   * @tparam V The type of SQL row to store the value.
   * @tparam I The type of SQL row to store the index.
   * @tparam T The type of SqlTranslator used for filtering values.
   */
  template<typename C, typename V, typename I, typename T>
  class SqlDataStore {
    public:

      /** The type of SQL connection to query. */
      using Connection = C;

      /** The type of SQL row to store the value. */
      using ValueRow = V;

      /** The type of SQL row to store the index. */
      using IndexRow = I;

      /** The type of SqlTranslator used for filtering values. */
      using SqlTranslator = T;

      /** The type of value to store. */
      using Value = typename ValueRow::Type;

      /** The type of index. */
      using Index = typename IndexRow::Type;

      /** The type of query used to load values. */
      using Query = BasicQuery<Index>;

      /** The SequencedValue to store. */
      using SequencedValue = Beam::SequencedValue<Value>;

      /** The IndexedValue to store. */
      using IndexedValue =
        Beam::SequencedValue<Beam::IndexedValue<Value, Index>>;

      /**
       * Constructs an SqlDataStore.
       * @param table The name of the SQL table.
       * @param value_row The SQL row to store the value.
       * @param index_row The SQL row to store the index.
       * @param reader_pool The pool of SQL connections used for reading.
       * @param writer_pool The pool of SQL connections used for writing.
       */
      SqlDataStore(std::string table, ValueRow value_row, IndexRow index_row,
        Ref<DatabaseConnectionPool<Connection>> reader_pool,
        Ref<DatabaseConnectionPool<Connection>> writer_pool);

      /**
       * Constructs an SqlDataStore.
       * @param table The name of the SQL table.
       * @param value_row The SQL row to store the value.
       * @param index_row The SQL row to store the index.
       * @param reader_pool The pool of SQL connections used for reading.
       * @param writer_pool The pool of SQL connections used for writing.
       * @param connection_option Specifies the ConnectionOption to use.
       */
      SqlDataStore(std::string table, ValueRow value_row, IndexRow index_row,
        Ref<DatabaseConnectionPool<Connection>> reader_pool,
        Ref<DatabaseConnectionPool<Connection>> writer_pool,
        SqlConnectionOption connection_option);

      ~SqlDataStore();

      /**
       * Executes a search query.
       * @param query The search query to execute.
       * @return The list of the values that satisfy the search <i>query</i>.
       */
      std::vector<SequencedValue> load(const Viper::Expression& query);

      std::vector<SequencedValue> load(const Query& query);
      void store(const IndexedValue& value);
      void store(const std::vector<IndexedValue>& values);
      void close();

    private:
      std::string m_table;
      ValueRow m_value_row;
      IndexRow m_index_row;
      Viper::Row<IndexedValue> m_row;
      Viper::Row<SequencedValue> m_sequenced_row;
      DatabaseConnectionPool<Connection>* m_reader_pool;
      DatabaseConnectionPool<Connection>* m_writer_pool;
  };

  template<typename C, typename V, typename I, typename T>
  SqlDataStore<C, V, I, T>::SqlDataStore(std::string table, ValueRow value_row,
      IndexRow index_row, Ref<DatabaseConnectionPool<Connection>> reader_pool,
      Ref<DatabaseConnectionPool<Connection>> writer_pool,
      SqlConnectionOption connection_option)
      : m_table(std::move(table)),
        m_value_row(std::move(value_row)),
        m_index_row(std::move(index_row)),
        m_reader_pool(reader_pool.get()),
        m_writer_pool(writer_pool.get()) {
    m_value_row = m_value_row.
      add_column("timestamp",
        [] (const auto& row) {
          return get_timestamp(row);
        },
        [] (auto& row, auto value) {
          get_timestamp(row) = value;
        });
    m_sequenced_row = Viper::Row<SequencedValue>().
      extend(m_value_row,
        [] (auto& row) -> auto& {
          return row.get_value();
        }).
      add_column("query_sequence",
        [] (const auto& row) {
          return row.get_sequence().get_ordinal();
        },
        [] (auto& row, auto value) {
          row.get_sequence() = Sequence(value);
        });
    auto is_index_embedded = false;
    for(auto& index_column : m_index_row.get_columns()) {
      for(auto& value_column : m_value_row.get_columns()) {
        if(index_column.m_name == value_column.m_name) {
          is_index_embedded = true;
          break;
        }
      }
    }
    if(!is_index_embedded) {
      m_row = m_row.extend(m_index_row, [] (auto& row) -> auto& {
        return row->get_index();
      });
    }
    m_row = m_row.
      extend(m_value_row,
        [] (auto& row) -> auto& {
          return row->get_value();
        }).
      add_column("query_sequence",
        [] (const auto& row) {
          return row.get_sequence().get_ordinal();
        },
        [] (auto& row, auto value) {
          row.get_sequence() = Sequence(value);
        });
    auto index = std::vector<std::string>();
    for(auto& column : m_index_row.get_columns()) {
      index.push_back(column.m_name);
    }
    auto sequence_index = index;
    sequence_index.push_back("query_sequence");
    m_row = m_row.add_index("sequence_index", std::move(sequence_index));
    auto timestamp_index = index;
    timestamp_index.push_back("query_sequence");
    timestamp_index.push_back("timestamp");
    m_row = m_row.add_index("timestamp_index", std::move(timestamp_index));
    if(connection_option == SqlConnectionOption::CREATE) {
      auto connection = m_writer_pool->acquire();
      connection->execute(create_if_not_exists(m_row, m_table));
    } else if(connection_option == SqlConnectionOption::ENSURE) {
      auto connection = m_writer_pool->acquire();
      if(!connection->has_table(m_table)) {
        boost::throw_with_location(
          ConnectException("Table " + m_table + " doesn't exist."));
      }
    }
  }

  template<typename C, typename V, typename I, typename T>
  SqlDataStore<C, V, I, T>::SqlDataStore(std::string table, ValueRow value_row,
    IndexRow index_row, Ref<DatabaseConnectionPool<Connection>> reader_pool,
    Ref<DatabaseConnectionPool<Connection>> writer_pool)
    : SqlDataStore(std::move(table), std::move(value_row), std::move(index_row),
        Ref(reader_pool), Ref(writer_pool), SqlConnectionOption::CREATE) {}

  template<typename C, typename V, typename I, typename T>
  SqlDataStore<C, V, I, T>::~SqlDataStore() {
    close();
  }

  template<typename C, typename V, typename I, typename T>
  std::vector<typename SqlDataStore<C, V, I, T>::SequencedValue>
      SqlDataStore<C, V, I, T>::load(const Query& query) {
    auto index = std::optional<Viper::Expression>();
    auto column = std::string();
    for(auto i = std::size_t(0); i != m_index_row.get_columns().size(); ++i) {
      m_index_row.append_value(query.get_index(), i, column);
      auto term =
        Viper::sym(m_index_row.get_columns()[i].m_name) == Viper::sym(column);
      if(index) {
        *index = *index && term;
      } else {
        index.emplace(std::move(term));
      }
      column.clear();
    }
    if(!index) {
      index.emplace();
    }
    return load_sql_query<SqlTranslator>(
      query, m_sequenced_row, m_table, *index, *m_reader_pool);
  }

  template<typename C, typename V, typename I, typename T>
  std::vector<typename SqlDataStore<C, V, I, T>::SequencedValue>
      SqlDataStore<C, V, I, T>::load(const Viper::Expression& query) {
    return load_sql_query(query, m_sequenced_row, m_table, *m_reader_pool);
  }

  template<typename C, typename V, typename I, typename T>
  void SqlDataStore<C, V, I, T>::store(const IndexedValue& value) {
    auto connection = m_writer_pool->acquire();
    connection->execute(Viper::insert(m_row, m_table, &value));
  }

  template<typename C, typename V, typename I, typename T>
  void SqlDataStore<C, V, I, T>::store(
      const std::vector<IndexedValue>& values) {
    auto connection = m_writer_pool->acquire();
    connection->execute(
      Viper::insert(m_row, m_table, values.begin(), values.end()));
  }

  template<typename C, typename V, typename I, typename T>
  void SqlDataStore<C, V, I, T>::close() {}
}

#endif
