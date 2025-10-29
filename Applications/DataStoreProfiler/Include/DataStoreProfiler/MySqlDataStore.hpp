#ifndef BEAM_DATA_STORE_PROFILER_MYSQL_DATA_STORE_HPP
#define BEAM_DATA_STORE_PROFILER_MYSQL_DATA_STORE_HPP
#include <string>
#include <thread>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Network/IpAddress.hpp>
#include <Beam/Queries/SqlDataStore.hpp>
#include <Beam/Queries/SqlTranslator.hpp>
#include <Beam/Sql/DatabaseConnectionPool.hpp>
#include <Beam/Sql/SqlConnection.hpp>
#include <Viper/MySql/Connection.hpp>

namespace Beam {

  /** Stores data in a MySQL database. */
  class MySqlProfileDataStore {
    public:

      /**
       * Constructs a MySqlProfileDataStore.
       * @param address The IP address of the MySQL database to connect to.
       * @param schema The name of the schema.
       * @param username The username to connect as.
       * @param password The password associated with the <i>username</i>.
       */
      MySqlProfileDataStore(IpAddress address, std::string schema,
        std::string username, std::string password);

      ~MySqlProfileDataStore();

      void clear();
      std::vector<SequencedEntry> load_entries(const EntryQuery& query);
      void store(const SequencedIndexedEntry& entry);
      void store(const std::vector<SequencedIndexedEntry>& entries);
      void close();

    private:
      template<typename V, typename I>
      using DataStore = SqlDataStore<
        SqlConnection<Viper::MySql::Connection>, V, I, SqlTranslator>;
      DatabaseConnectionPool<SqlConnection<Viper::MySql::Connection>>
        m_reader_pool;
      DatabaseConnectionPool<SqlConnection<Viper::MySql::Connection>>
        m_writer_pool;
      DataStore<Viper::Row<Entry>, Viper::Row<std::string>> m_data_store;
      OpenState m_open_state;

      MySqlProfileDataStore(const MySqlProfileDataStore&) = delete;
      MySqlProfileDataStore& operator =(const MySqlProfileDataStore&) = delete;
      static Viper::Row<Entry> build_value_row();
      static Viper::Row<std::string> build_index_row();
  };

  inline MySqlProfileDataStore::MySqlProfileDataStore(IpAddress address,
    std::string schema, std::string username, std::string password)
    : m_reader_pool(std::thread::hardware_concurrency(), [=] {
        auto connection = make_sql_connection(Viper::MySql::Connection(
          address.get_host(), address.get_port(), username, password, schema));
        connection->open();
        return connection;
      }),
      m_writer_pool(std::thread::hardware_concurrency(), [=] {
        auto connection = make_sql_connection(Viper::MySql::Connection(
          address.get_host(), address.get_port(), username, password, schema));
        connection->open();
        return connection;
      }),
      m_data_store("entries", build_value_row(), build_index_row(),
        Ref(m_reader_pool), Ref(m_writer_pool)) {}

  inline MySqlProfileDataStore::~MySqlProfileDataStore() {
    close();
  }

  inline void MySqlProfileDataStore::clear() {
    auto connection = m_writer_pool.acquire();
    connection->execute(Viper::truncate("entries"));
  }

  inline std::vector<SequencedEntry> MySqlProfileDataStore::load_entries(
      const EntryQuery& query) {
    return m_data_store.load(query);
  }

  inline void MySqlProfileDataStore::store(
      const SequencedIndexedEntry& entry) {
    return m_data_store.store(entry);
  }

  inline void MySqlProfileDataStore::store(
      const std::vector<SequencedIndexedEntry>& entries) {
    return m_data_store.store(entries);
  }

  inline void MySqlProfileDataStore::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_writer_pool.close();
    m_reader_pool.close();
    m_open_state.close();
  }

  inline Viper::Row<Entry> MySqlProfileDataStore::build_value_row() {
    return Viper::Row<Entry>().
      add_column("item_a", &Entry::m_item_a).
      add_column("item_b", &Entry::m_item_b).
      add_column("item_c", &Entry::m_item_c).
      add_column("item_d", Viper::varchar(100), &Entry::m_item_d);
  }

  inline Viper::Row<std::string> MySqlProfileDataStore::build_index_row() {
    return Viper::Row<std::string>().add_column(
      "name", Viper::VarCharDataType(16));
  }
}

#endif
