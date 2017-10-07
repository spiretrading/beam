#ifndef BEAM_DATASTOREPROFILER_MYSQLDATASTORE_HPP
#define BEAM_DATASTOREPROFILER_MYSQLDATASTORE_HPP
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/MySql/DatabaseConnectionPool.hpp>
#include <Beam/Network/IpAddress.hpp>
#include <Beam/Queries/SqlDataStore.hpp>
#include <Beam/Queries/SqlTranslator.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/Threading/ThreadPool.hpp>
#include <boost/noncopyable.hpp>
#include <boost/throw_exception.hpp>
#include "DataStoreProfiler/MySqlDataStoreDetails.hpp"

namespace Beam {

  /*! \class MySqlDataStore
      \brief Stores data in a MySQL database.
   */
  class MySqlDataStore : private boost::noncopyable {
    public:

      //! Constructs a MySqlDataStore.
      /*!
        \param address The IP address of the MySQL database to connect to.
        \param schema The name of the schema.
        \param username The username to connect as.
        \param password The password associated with the <i>username</i>.
      */
      MySqlDataStore(const Network::IpAddress& address,
        const std::string& schema, const std::string& username,
        const std::string& password);

      ~MySqlDataStore();

      //! Clears the contents of the database.
      void Clear();

      std::vector<SequencedEntry> LoadEntries(const EntryQuery& query);

      void Store(const SequencedIndexedEntry& entry);

      void Store(const std::vector<SequencedIndexedEntry>& entries);

      void Open();

      void Close();

    private:
      template<typename Query, typename T, typename Row>
      using DataStore = Queries::SqlDataStore<Query, T, Row,
        Queries::SqlTranslator, Details::SqlFunctor>;
      Network::IpAddress m_address;
      std::string m_schema;
      std::string m_username;
      std::string m_password;
      MySql::DatabaseConnectionPool m_readerDatabaseConnectionPool;
      Threading::Sync<mysqlpp::Connection, Threading::Mutex>
        m_writerDatabaseConnection;
      Threading::ThreadPool m_readerThreadPool;
      DataStore<EntryQuery, Entry, Details::entries> m_entryDataStore;
      IO::OpenState m_openState;

      void Shutdown();
      void OpenDatabaseConnection(mysqlpp::Connection& connection);
  };

  inline MySqlDataStore::MySqlDataStore(const Network::IpAddress& address,
      const std::string& schema, const std::string& username,
      const std::string& password)
      : m_address{address},
        m_schema{schema},
        m_username{username},
        m_password{password},
        m_writerDatabaseConnection{false},
        m_entryDataStore{Ref(m_readerDatabaseConnectionPool),
          Ref(m_writerDatabaseConnection), Ref(m_readerThreadPool)} {}

  inline MySqlDataStore::~MySqlDataStore() {
    Close();
  }

  inline void MySqlDataStore::Clear() {
    m_writerDatabaseConnection.With(
      [&] (auto& connection) {
        auto query = connection.query();
        query << "TRUNCATE TABLE entries";
        if(!query.execute()) {
          BOOST_THROW_EXCEPTION(std::runtime_error{query.error()});
        }
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
      Beam::Threading::With(m_writerDatabaseConnection,
        [&] (auto& connection) {
          this->OpenDatabaseConnection(connection);
          if(!Details::LoadTables(connection, m_schema)) {
            BOOST_THROW_EXCEPTION(IO::ConnectException{
              "Unable to load database tables."});
          }
        });
      for(std::size_t i = 0; i <= boost::thread::hardware_concurrency(); ++i) {
        auto connection = std::make_unique<mysqlpp::Connection>(false);
        OpenDatabaseConnection(*connection);
        m_readerDatabaseConnectionPool.Add(std::move(connection));
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
    m_readerDatabaseConnectionPool.Close();
    Beam::Threading::With(m_writerDatabaseConnection,
      [] (auto& connection) {
        connection.disconnect();
      });
    m_openState.SetClosed();
  }

  inline void MySqlDataStore::OpenDatabaseConnection(
      mysqlpp::Connection& connection) {
    auto connectionResult = connection.set_option(
      new mysqlpp::ReconnectOption{true});
    if(!connectionResult) {
      BOOST_THROW_EXCEPTION(IO::ConnectException{
        "Unable to set MySQL reconnect option."});
    }
    connectionResult = connection.connect(m_schema.c_str(),
      m_address.GetHost().c_str(), m_username.c_str(), m_password.c_str(),
      m_address.GetPort());
    if(!connectionResult) {
      BOOST_THROW_EXCEPTION(IO::ConnectException{std::string{
        "Unable to connect to MySQL database - "} + connection.error()});
    }
  }
}

#endif
