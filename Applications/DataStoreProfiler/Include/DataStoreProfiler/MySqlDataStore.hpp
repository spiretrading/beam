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
#include <Beam/Threading/Sync.hpp>
#include <Beam/Threading/ThreadPool.hpp>
#include <boost/noncopyable.hpp>
#include <boost/throw_exception.hpp>
#include <Viper/MySql/Connection.hpp>

namespace Beam {

  /** Stores data in a MySQL database. */
  class MySqlDataStore : private boost::noncopyable {
    public:

      //! Constructs a MySqlDataStore.
      /*!
        \param address The IP address of the MySQL database to connect to.
        \param schema The name of the schema.
        \param username The username to connect as.
        \param password The password associated with the <i>username</i>.
      */
      MySqlDataStore(Network::IpAddress address, std::string schema,
        std::string username, std::string password);

      ~MySqlDataStore();

      //! Clears the contents of the database.
      void Clear();

      std::vector<SequencedEntry> LoadEntries(const EntryQuery& query);

      void Store(const SequencedIndexedEntry& entry);

      void Store(const std::vector<SequencedIndexedEntry>& entries);

      void Open();

      void Close();

    private:
      template<typename V, typename I>
      using DataStore = Queries::SqlDataStore<Viper::MySql::Connection, V, I,
        Queries::SqlTranslator>;
      Network::IpAddress m_address;
      std::string m_schema;
      std::string m_username;
      std::string m_password;
      DatabaseConnectionPool<Viper::MySql::Connection> m_connectionPool;
      Threading::Sync<Viper::MySql::Connection, Threading::Mutex>
        m_writeConnection;
      Threading::ThreadPool m_threadPool;
      DataStore<Viper::Row<Entry>, Viper::Row<std::string>> m_entryDataStore;
      IO::OpenState m_openState;

      static Viper::Row<Entry> BuildValueRow();
      static Viper::Row<std::string> BuildIndexRow();
      void Shutdown();
  };

  inline MySqlDataStore::MySqlDataStore(Network::IpAddress address,
      std::string schema, std::string username, std::string password)
      : m_address(std::move(address)),
        m_schema(std::move(schema)),
        m_username(std::move(username)),
        m_password(std::move(password)),
        m_writeConnection(m_address.GetHost(), m_address.GetPort(), m_username,
          m_password, m_schema),
        m_entryDataStore("entries", BuildValueRow(), BuildIndexRow(),
          Ref(m_connectionPool), Ref(m_writeConnection), Ref(m_threadPool)) {}

  inline MySqlDataStore::~MySqlDataStore() {
    Close();
  }

  inline void MySqlDataStore::Clear() {
    m_writeConnection.With(
      [&] (auto& connection) {
        connection.execute(Viper::truncate("entries"));
      });
  }

  inline std::vector<SequencedEntry> MySqlDataStore::LoadEntries(
      const EntryQuery& query) {
    return m_entryDataStore.Load(query);
  }

  inline void MySqlDataStore::Store(const SequencedIndexedEntry& entry) {
    return m_entryDataStore.Store(entry);
  }

  inline void MySqlDataStore::Store(
      const std::vector<SequencedIndexedEntry>& entries) {
    return m_entryDataStore.Store(entries);
  }

  inline void MySqlDataStore::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      Beam::Threading::With(m_writeConnection,
        [&] (auto& connection) {
          connection.open();
        });
      for(auto i = std::size_t(0);
          i <= std::thread::hardware_concurrency(); ++i) {
        auto connection = std::make_unique<Viper::MySql::Connection>(
          m_address.GetHost(), m_address.GetPort(), m_username,
          m_password, m_schema);
        connection->open();
        m_connectionPool.Add(std::move(connection));
      }
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  inline void MySqlDataStore::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline void MySqlDataStore::Shutdown() {
    m_connectionPool.Close();
    Beam::Threading::With(m_writeConnection,
      [] (auto& connection) {
        connection.close();
      });
    m_openState.SetClosed();
  }

  inline Viper::Row<Entry> MySqlDataStore::BuildValueRow() {
    return Viper::Row<Entry>().
      add_column("item_a", &Entry::m_itemA).
      add_column("item_b", &Entry::m_itemB).
      add_column("item_c", &Entry::m_itemC).
      add_column("item_d", &Entry::m_itemD);
  }

  inline Viper::Row<std::string> MySqlDataStore::BuildIndexRow() {
    return Viper::Row<std::string>().add_column("name",
      Viper::VarCharDataType(16));
  }
}

#endif
