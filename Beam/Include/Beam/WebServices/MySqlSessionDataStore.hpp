#ifndef BEAM_MYSQL_SESSION_DATA_STORE_HPP
#define BEAM_MYSQL_SESSION_DATA_STORE_HPP
#include <boost/noncopyable.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/IO/ConnectException.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Network/IpAddress.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"
#include "Beam/Serialization/JsonReceiver.hpp"
#include "Beam/Serialization/JsonSender.hpp"
#include "Beam/WebServices/MySqlSessionDataStoreDetails.hpp"
#include "Beam/WebServices/Session.hpp"
#include "Beam/WebServices/SessionDataStore.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  /*! \class MySqlSessionDataStore
      \brief Stores persistent web sessions in a MySQL database.
   */
  class MySqlSessionDataStore : private boost::noncopyable {
    public:

      //! Constructs a MySqlSessionDataStore.
      /*!
        \param address The IP address of the MySQL database to connect to.
        \param schema The name of the schema.
        \param username The username to connect as.
        \param password The password associated with the <i>username</i>.
      */
      MySqlSessionDataStore(const Network::IpAddress& address,
        const std::string& schema, const std::string& username,
        const std::string& password);

      ~MySqlSessionDataStore();

      template<typename SessionType>
      std::unique_ptr<SessionType> Load(const std::string& id);

      template<typename SessionType>
      void Store(const SessionType& session);

      template<typename SessionType>
      void Delete(const SessionType& session);

      template<typename F>
      void WithTransaction(F&& transaction);

      void Open();

      void Close();

    private:
      mutable Threading::RecursiveMutex m_mutex;
      Network::IpAddress m_address;
      std::string m_schema;
      std::string m_username;
      std::string m_password;
      mysqlpp::Connection m_databaseConnection;
      Serialization::JsonSender<IO::SharedBuffer> m_sender;
      Serialization::JsonReceiver<IO::SharedBuffer> m_receiver;
      IO::OpenState m_openState;

      void Shutdown();
  };

  inline MySqlSessionDataStore::MySqlSessionDataStore(
      const Network::IpAddress& address, const std::string& schema,
      const std::string& username, const std::string& password)
      : m_address{address},
        m_schema{schema},
        m_username{username},
        m_password{password},
        m_databaseConnection{false} {}

  inline MySqlSessionDataStore::~MySqlSessionDataStore() {
    Close();
  }

  template<typename SessionType>
  std::unique_ptr<SessionType> MySqlSessionDataStore::Load(
      const std::string& id) {
    boost::lock_guard<Threading::RecursiveMutex> lock{m_mutex};
    auto query = m_databaseConnection.query();
    query << "SELECT * FROM web_sessions WHERE id = " << mysqlpp::quote << id;
    std::vector<Details::web_sessions> rows;
    query.storein(rows);
    if(query.errnum() != 0) {
      BOOST_THROW_EXCEPTION(SessionDataStoreException{query.error()});
    }
    if(rows.empty()) {
      return nullptr;
    }
    auto& row = rows.front();
    auto session = Details::FromRow<SessionType>(row, m_receiver);
    return session;
  }

  template<typename SessionType>
  void MySqlSessionDataStore::Store(const SessionType& session) {
    boost::lock_guard<Threading::RecursiveMutex> lock{m_mutex};
    auto query = m_databaseConnection.query();
    auto row = Details::ToRow(session, m_sender);
    query.replace(row);
    if(!query.execute()) {
      BOOST_THROW_EXCEPTION(SessionDataStoreException{query.error()});
    }
  }

  template<typename SessionType>
  void MySqlSessionDataStore::Delete(const SessionType& session) {
    boost::lock_guard<Threading::RecursiveMutex> lock{m_mutex};
    auto query = m_databaseConnection.query();
    query << "DELETE FROM web_sessions WHERE id = " << mysqlpp::quote <<
      session.GetId();
    query.execute();
    if(query.errnum() != 0) {
      BOOST_THROW_EXCEPTION(SessionDataStoreException{query.error()});
    }
  }

  template<typename F>
  void MySqlSessionDataStore::WithTransaction(F&& transaction) {
    boost::lock_guard<Threading::RecursiveMutex> lock{m_mutex};
    mysqlpp::Transaction t{m_databaseConnection};
    try {
      transaction();
    } catch(...) {
      t.rollback();
      throw;
    }
    t.commit();
  }

  inline void MySqlSessionDataStore::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      auto connectionResult = m_databaseConnection.set_option(
        new mysqlpp::ReconnectOption{true});
      if(!connectionResult) {
        BOOST_THROW_EXCEPTION(IO::IOException{
          "Unable to set MySQL reconnect option."});
      }
      connectionResult = m_databaseConnection.connect(m_schema.c_str(),
        m_address.GetHost().c_str(), m_username.c_str(), m_password.c_str(),
        m_address.GetPort());
      if(!connectionResult) {
        BOOST_THROW_EXCEPTION(IO::ConnectException{std::string(
          "Unable to connect to MySQL database - ") +
          m_databaseConnection.error()});
      }
      if(!Details::LoadTables(m_databaseConnection, m_schema)) {
        BOOST_THROW_EXCEPTION(IO::IOException{
          "Unable to load database tables."});
      }
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  inline void MySqlSessionDataStore::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline void MySqlSessionDataStore::Shutdown() {
    m_databaseConnection.disconnect();
    m_openState.SetClosed();
  }
}
}

#endif
