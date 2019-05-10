#ifndef BEAM_SQL_SESSION_DATA_STORE_HPP
#define BEAM_SQL_SESSION_DATA_STORE_HPP
#include <boost/noncopyable.hpp>
#include <boost/throw_exception.hpp>
#include <Viper/Viper.hpp>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Serialization/JsonReceiver.hpp"
#include "Beam/Serialization/JsonSender.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"
#include "Beam/WebServices/Session.hpp"
#include "Beam/WebServices/SessionDataStore.hpp"
#include "Beam/WebServices/SqlDefinitions.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam::WebServices {

  /** Stores persistent web sessions in a MySQL database.
      \tparam - The SQL connection to use.
   */
  template<typename C>
  class SqlSessionDataStore : private boost::noncopyable {
    public:

      //! The SQL connection to use.
      using Connection = C;

      //! Constructs a SqlSessionDataStore.
      /*!
        \param connection The SQL connection.
      */
      SqlSessionDataStore(std::unique_ptr<Connection> connection);

      ~SqlSessionDataStore();

      template<typename Session>
      std::unique_ptr<Session> Load(const std::string& id);

      template<typename Session>
      void Store(const Session& session);

      template<typename Session>
      void Delete(const Session& session);

      template<typename F>
      void WithTransaction(F&& transaction);

      void Open();

      void Close();

    private:
      mutable Threading::RecursiveMutex m_mutex;
      std::unique_ptr<Connection> m_connection;
      Serialization::JsonSender<IO::SharedBuffer> m_sender;
      Serialization::JsonReceiver<IO::SharedBuffer> m_receiver;
      IO::OpenState m_openState;

      void Shutdown();
  };

  template<typename C>
  SqlSessionDataStore<C>::SqlSessionDataStore(
      std::unique_ptr<Connection> connection)
      : m_connection(std::move(connection)) {}

  template<typename C>
  SqlSessionDataStore<C>::~SqlSessionDataStore() {
    Close();
  }

  template<typename C>
  template<typename Session>
  std::unique_ptr<Session> SqlSessionDataStore<C>::Load(const std::string& id) {
    auto sqlSession = SqlSession{};
    auto lock = std::lock_guard(m_mutex);
    try {
      m_connection->execute(Viper::select(GetWebSessionsRow(), "web_sessions",
        &sqlSession));
      return FromRow<Session>(sqlSession, m_receiver);
    } catch(const std::exception& e) {
      BOOST_THROW_EXCEPTION(SessionDataStoreException(e.what()));
    }
  }

  template<typename C>
  template<typename Session>
  void SqlSessionDataStore<C>::Store(const Session& session) {
    auto lock = std::lock_guard(m_mutex);
    auto row = ToRow(session, m_sender);
    try {
      m_connection->execute(Viper::upsert(GetWebSessionsRow(), "web_sessions",
        &row));
    } catch(const std::exception& e) {
      BOOST_THROW_EXCEPTION(SessionDataStoreException(e.what()));
    }
  }

  template<typename C>
  template<typename Session>
  void SqlSessionDataStore<C>::Delete(const Session& session) {
    auto lock = std::lock_guard(m_mutex);
    try {
      m_connection->execute(Viper::erase("web_sessions",
        Viper::sym("id") == session.GetId()));
    } catch(const std::exception& e) {
      BOOST_THROW_EXCEPTION(SessionDataStoreException(e.what()));
    }
  }

  template<typename C>
  template<typename F>
  void SqlSessionDataStore<C>::WithTransaction(F&& transaction) {
    auto lock = std::lock_guard(m_mutex);
    Viper::transaction(*m_connection, std::forward<F>(transaction));
  }

  template<typename C>
  void SqlSessionDataStore<C>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_connection->open();
      m_connection->execute(Viper::create_if_not_exists(GetWebSessionsRow(),
        "web_sessions"));
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename C>
  void SqlSessionDataStore<C>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename C>
  void SqlSessionDataStore<C>::Shutdown() {
    m_connection->close();
    m_openState.SetClosed();
  }
}

#endif
