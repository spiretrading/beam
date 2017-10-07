#ifndef BEAM_MYSQLUIDDATASTORE_HPP
#define BEAM_MYSQLUIDDATASTORE_HPP
#include "Beam/IO/ConnectException.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Network/IpAddress.hpp"
#include "Beam/Threading/Mutex.hpp"
#include "Beam/UidService/UidDataStore.hpp"
#include "Beam/UidService/MySqlUidDataStoreDetails.hpp"

namespace Beam {
namespace UidService {

  /*! \class MySqlUidDataStore
      \brief Implements the UidDataStore using MySQL.
   */
  class MySqlUidDataStore : public UidDataStore {
    public:

      //! Constructs a MySqlUidDataStore.
      /*!
        \param address The IP address of the MySQL database to connect to.
        \param schema The name of the schema.
        \param username The username to connect as.
        \param password The password associated with the <i>username</i>.
      */
      MySqlUidDataStore(const Network::IpAddress& address,
        const std::string& schema, const std::string& username,
        const std::string& password);

      virtual ~MySqlUidDataStore() override;

      virtual std::uint64_t GetNextUid() override;

      virtual std::uint64_t Reserve(std::uint64_t size) override;

      virtual void WithTransaction(
        const std::function<void ()>& transaction) override;

      virtual void Open() override;

      virtual void Close() override;

    private:
      mutable Threading::Mutex m_mutex;
      Network::IpAddress m_address;
      std::string m_schema;
      std::string m_username;
      std::string m_password;
      mysqlpp::Connection m_databaseConnection;
      std::uint64_t m_nextUid;
      IO::OpenState m_openState;

      void Shutdown();
  };

  inline MySqlUidDataStore::MySqlUidDataStore(const Network::IpAddress& address,
      const std::string& schema, const std::string& username,
      const std::string& password)
      : m_address{address},
        m_schema{schema},
        m_username{username},
        m_password{password},
        m_databaseConnection{false} {}

  inline MySqlUidDataStore::~MySqlUidDataStore() {
    Close();
  }

  inline std::uint64_t MySqlUidDataStore::GetNextUid() {
    auto query = m_databaseConnection.query();
    query << "SELECT * FROM next_uid";
    auto result = query.store();
    if(!result || result.size() != 1) {
      BOOST_THROW_EXCEPTION(IO::IOException{"MySQL query failed."});
    }
    return result[0][0].conv<std::uint64_t>(0);
  }

  inline std::uint64_t MySqlUidDataStore::Reserve(std::uint64_t size) {
    auto nextUid = GetNextUid();
    auto query = m_databaseConnection.query();
    query << "UPDATE next_uid SET uid = " << (nextUid + size);
    if(!query.execute()) {
      BOOST_THROW_EXCEPTION(IO::IOException{"MySQL query failed."});
    }
    return nextUid;
  }

  inline void MySqlUidDataStore::WithTransaction(
      const std::function<void ()>& transaction) {
    boost::lock_guard<Threading::Mutex> lock{m_mutex};
    mysqlpp::Transaction t(m_databaseConnection);
    try {
      transaction();
    } catch(...) {
      t.rollback();
      throw;
    }
    t.commit();
  }

  inline void MySqlUidDataStore::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      auto connectionResult = m_databaseConnection.set_option(
        new mysqlpp::ReconnectOption{true});
      if(!connectionResult) {
        BOOST_THROW_EXCEPTION(
          IO::IOException{"Unable to set MySQL reconnect option."});
      }
      connectionResult = m_databaseConnection.connect(m_schema.c_str(),
        m_address.GetHost().c_str(), m_username.c_str(), m_password.c_str(),
        m_address.GetPort());
      if(!connectionResult) {
        BOOST_THROW_EXCEPTION(IO::ConnectException{
          std::string("Unable to connect to MySQL database - ") +
          m_databaseConnection.error()});
      }
      if(!Details::LoadTables(m_databaseConnection, m_schema)) {
        BOOST_THROW_EXCEPTION(IO::ConnectException{
          "Unable to load database tables."});
      }
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  inline void MySqlUidDataStore::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline void MySqlUidDataStore::Shutdown() {
    m_databaseConnection.disconnect();
    m_openState.SetClosed();
  }
}
}

#endif
