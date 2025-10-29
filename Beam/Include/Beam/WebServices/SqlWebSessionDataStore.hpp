#ifndef BEAM_SQL_WEB_SESSION_DATA_STORE_HPP
#define BEAM_SQL_WEB_SESSION_DATA_STORE_HPP
#include <boost/throw_exception.hpp>
#include <Viper/Viper.hpp>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Serialization/JsonReceiver.hpp"
#include "Beam/Serialization/JsonSender.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"
#include "Beam/WebServices/WebSessionDataStore.hpp"
#include "Beam/WebServices/SqlDefinitions.hpp"

namespace Beam {

  /**
   * Stores persistent web sessions in a MySQL database.
   * @tparam C The SQL connection to use.
   */
  template<typename C>
  class SqlWebSessionDataStore {
    public:

      /** The SQL connection to use. */
      using Connection = C;

      /**
       * Constructs a SqlWebSessionDataStore.
       * @param connection The SQL connection.
       */
      explicit SqlWebSessionDataStore(std::unique_ptr<Connection> connection);

      ~SqlWebSessionDataStore();

      template<std::derived_from<WebSession> S>
      std::unique_ptr<S> load(const std::string& id);
      template<std::derived_from<WebSession> S>
      void store(const S& session);
      template<std::derived_from<WebSession> S>
      void remove(const S& session);
      template<std::invocable<> F>
      decltype(auto) with_transaction(F&& transaction);
      void close();

    private:
      mutable RecursiveMutex m_mutex;
      std::unique_ptr<Connection> m_connection;
      JsonSender<SharedBuffer> m_sender;
      JsonReceiver<SharedBuffer> m_receiver;
      OpenState m_open_state;
  };

  template<typename C>
  SqlWebSessionDataStore<C>::SqlWebSessionDataStore(
      std::unique_ptr<Connection> connection)
      : m_connection(std::move(connection)) {
    try {
      m_connection->open();
      m_connection->execute(
        Viper::create_if_not_exists(get_web_sessions_row(), "web_sessions"));
    } catch(const std::exception&) {
      close();
      throw;
    }
  }

  template<typename C>
  SqlWebSessionDataStore<C>::~SqlWebSessionDataStore() {
    close();
  }

  template<typename C>
  template<std::derived_from<WebSession> S>
  std::unique_ptr<S> SqlWebSessionDataStore<C>::load(const std::string& id) {
    auto sql_session = SqlSession();
    auto lock = std::lock_guard(m_mutex);
    try {
      m_connection->execute(
        Viper::select(get_web_sessions_row(), "web_sessions",
          Viper::sym("id") == id, &sql_session));
      return from_row<S>(sql_session, m_receiver);
    } catch(const std::exception& e) {
      boost::throw_with_location(WebSessionDataStoreException(e.what()));
    }
  }

  template<typename C>
  template<std::derived_from<WebSession> S>
  void SqlWebSessionDataStore<C>::store(const S& session) {
    auto lock = std::lock_guard(m_mutex);
    auto row = to_row(session, m_sender);
    try {
      m_connection->execute(
        Viper::upsert(get_web_sessions_row(), "web_sessions", &row));
    } catch(const std::exception& e) {
      boost::throw_with_location(WebSessionDataStoreException(e.what()));
    }
  }

  template<typename C>
  template<std::derived_from<WebSession> S>
  void SqlWebSessionDataStore<C>::remove(const S& session) {
    auto lock = std::lock_guard(m_mutex);
    try {
      m_connection->execute(
        Viper::erase("web_sessions", Viper::sym("id") == session.get_id()));
    } catch(const std::exception& e) {
      boost::throw_with_location(WebSessionDataStoreException(e.what()));
    }
  }

  template<typename C>
  template<std::invocable<> F>
  decltype(auto) SqlWebSessionDataStore<C>::with_transaction(F&& transaction) {
    auto lock = std::lock_guard(m_mutex);
    return Viper::transaction(*m_connection, std::forward<F>(transaction));
  }

  template<typename C>
  void SqlWebSessionDataStore<C>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_connection->close();
    m_open_state.close();
  }
}

#endif
