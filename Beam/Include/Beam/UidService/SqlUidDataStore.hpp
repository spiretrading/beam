#ifndef BEAM_SQL_UID_DATA_STORE_HPP
#define BEAM_SQL_UID_DATA_STORE_HPP
#include <memory>
#include <Viper/Viper.hpp>
#include "Beam/IO/OpenState.hpp"
#include "Beam/Threading/Mutex.hpp"
#include "Beam/UidService/SqlDefinitions.hpp"
#include "Beam/UidService/UidDataStore.hpp"

namespace Beam {

  /**
   * Implements the UidDataStore using SQL.
   * @tparam C The type of SQL connection.
   */
  template<typename C>
  class SqlUidDataStore {
    public:

      /** The type of SQL connection. */
      using Connection = C;

      /**
       * Constructs an SqlUidDataStore.
       * @param connection The connection to the SQL database.
       */
      explicit SqlUidDataStore(std::unique_ptr<Connection> connection);

      ~SqlUidDataStore();

      std::uint64_t get_next_uid();
      std::uint64_t reserve(std::uint64_t size);
      template<std::invocable<> F>
      decltype(auto) with_transaction(F&& transaction);
      void close();

    private:
      mutable Mutex m_mutex;
      std::unique_ptr<Connection> m_connection;
      OpenState m_open_state;
  };

  template<typename C>
  SqlUidDataStore<C>::SqlUidDataStore(std::unique_ptr<Connection> connection)
      : m_connection(std::move(connection)) {
    try {
      m_connection->open();
      if(!m_connection->has_table("next_uid")) {
        m_connection->execute(Viper::create(get_next_uid_row(), "next_uid"));
        auto first_uid = 1;
        m_connection->execute(
          Viper::insert(get_next_uid_row(), "next_uid", &first_uid));
      }
    } catch(const std::exception&) {
      close();
      throw;
    }
  }

  template<typename C>
  SqlUidDataStore<C>::~SqlUidDataStore() {
    close();
  }

  template<typename C>
  std::uint64_t SqlUidDataStore<C>::get_next_uid() {
    auto next_uid = std::uint64_t();
    m_connection->execute(
      Viper::select(get_next_uid_row(), "next_uid", &next_uid));
    return next_uid;
  }

  template<typename C>
  std::uint64_t SqlUidDataStore<C>::reserve(std::uint64_t size) {
    auto next_uid = get_next_uid();
    m_connection->execute(
      Viper::update("next_uid", {"uid", Viper::literal(next_uid + size)}));
    return next_uid;
  }

  template<typename C>
  template<std::invocable<> F>
  decltype(auto) SqlUidDataStore<C>::with_transaction(F&& transaction) {
    auto lock = std::lock_guard(m_mutex);
    return Viper::transaction(*m_connection, std::forward<F>(transaction));
  }

  template<typename C>
  void SqlUidDataStore<C>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_connection->close();
    m_open_state.close();
  }
}

#endif
