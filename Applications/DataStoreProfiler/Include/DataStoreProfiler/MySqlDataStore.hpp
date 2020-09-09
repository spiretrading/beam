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
#include <Beam/Threading/ThreadPool.hpp>
#include <boost/noncopyable.hpp>
#include <boost/throw_exception.hpp>
#include <Viper/MySql/Connection.hpp>

namespace Beam {

  /** Stores data in a MySQL database. */
  class MySqlDataStore : private boost::noncopyable {
    public:

      /**
       * Constructs a MySqlDataStore.
       * @param address The IP address of the MySQL database to connect to.
       * @param schema The name of the schema.
       * @param username The username to connect as.
       * @param password The password associated with the <i>username</i>.
       */
      MySqlDataStore(Network::IpAddress address, std::string schema,
        std::string username, std::string password);

      ~MySqlDataStore();

      /** Clears the contents of the database. */
      void Clear();

      std::vector<SequencedEntry> LoadEntries(const EntryQuery& query);

      void Store(const SequencedIndexedEntry& entry);

      void Store(const std::vector<SequencedIndexedEntry>& entries);

      void Close();

    private:
      template<typename V, typename I>
      using DataStore = Queries::SqlDataStore<Viper::MySql::Connection, V, I,
        Queries::SqlTranslator>;
      DatabaseConnectionPool<Viper::MySql::Connection> m_readerPool;
      DatabaseConnectionPool<Viper::MySql::Connection> m_writerPool;
      Threading::ThreadPool m_threadPool;
      DataStore<Viper::Row<Entry>, Viper::Row<std::string>> m_dataStore;
      IO::OpenState m_openState;

      static Viper::Row<Entry> BuildValueRow();
      static Viper::Row<std::string> BuildIndexRow();
  };

  inline MySqlDataStore::MySqlDataStore(Network::IpAddress address,
    std::string schema, std::string username, std::string password)
    : m_readerPool(std::thread::hardware_concurrency(), [=] {
        auto connection = std::make_unique<Viper::MySql::Connection>(
          address.GetHost(), address.GetPort(), username, password, schema);
        connection->open();
        return connection;
      }),
      m_writerPool(std::thread::hardware_concurrency(), [=] {
        auto connection = std::make_unique<Viper::MySql::Connection>(
          address.GetHost(), address.GetPort(), username, password, schema);
        connection->open();
        return connection;
      }),
      m_dataStore("entries", BuildValueRow(), BuildIndexRow(),
        Ref(m_readerPool), Ref(m_writerPool), Ref(m_threadPool)) {}

  inline MySqlDataStore::~MySqlDataStore() {
    Close();
  }

  inline void MySqlDataStore::Clear() {
    auto connection = m_writerPool.Acquire();
    connection->execute(Viper::truncate("entries"));
  }

  inline std::vector<SequencedEntry> MySqlDataStore::LoadEntries(
      const EntryQuery& query) {
    return m_dataStore.Load(query);
  }

  inline void MySqlDataStore::Store(const SequencedIndexedEntry& entry) {
    return m_dataStore.Store(entry);
  }

  inline void MySqlDataStore::Store(
      const std::vector<SequencedIndexedEntry>& entries) {
    return m_dataStore.Store(entries);
  }

  inline void MySqlDataStore::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_writerPool.Close();
    m_readerPool.Close();
    m_openState.Close();
  }

  inline Viper::Row<Entry> MySqlDataStore::BuildValueRow() {
    return Viper::Row<Entry>().
      add_column("item_a", &Entry::m_itemA).
      add_column("item_b", &Entry::m_itemB).
      add_column("item_c", &Entry::m_itemC).
      add_column("item_d", Viper::varchar(100), &Entry::m_itemD);
  }

  inline Viper::Row<std::string> MySqlDataStore::BuildIndexRow() {
    return Viper::Row<std::string>().add_column("name",
      Viper::VarCharDataType(16));
  }
}

#endif
