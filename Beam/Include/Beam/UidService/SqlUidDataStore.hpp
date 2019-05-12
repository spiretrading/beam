#ifndef BEAM_SQL_UID_DATA_STORE_HPP
#define BEAM_SQL_UID_DATA_STORE_HPP
#include <memory>
#include <Viper/Viper.hpp>
#include "Beam/IO/OpenState.hpp"
#include "Beam/Threading/Mutex.hpp"
#include "Beam/UidService/SqlDefinitions.hpp"
#include "Beam/UidService/UidDataStore.hpp"

namespace Beam::UidService {

  /** Implements the UidDataStore using SQL.
      \tparam C The type of SQL connection.
   */
  template<typename C>
  class SqlUidDataStore : public UidDataStore {
    public:

      //! The type of SQL connection.
      using Connection = C;

      //! Constructs an SqlUidDataStore.
      /*!
        \param connection The connection to the SQL database.
      */
      SqlUidDataStore(std::unique_ptr<Connection> connection);

      ~SqlUidDataStore() override;

      std::uint64_t GetNextUid() override;

      std::uint64_t Reserve(std::uint64_t size) override;

      void WithTransaction(const std::function<void ()>& transaction) override;

      void Open() override;

      void Close() override;

    private:
      mutable Threading::Mutex m_mutex;
      std::unique_ptr<Connection> m_connection;
      std::uint64_t m_nextUid;
      IO::OpenState m_openState;

      void Shutdown();
  };

  template<typename C>
  SqlUidDataStore<C>::SqlUidDataStore(std::unique_ptr<Connection> connection)
    : m_connection(std::move(connection)) {}

  template<typename C>
  SqlUidDataStore<C>::~SqlUidDataStore() {
    Close();
  }

  template<typename C>
  std::uint64_t SqlUidDataStore<C>::GetNextUid() {
    auto nextUid = std::uint64_t();
    m_connection->execute(Viper::select(GetNextUidRow(), "next_uid", &nextUid));
    return nextUid;
  }

  template<typename C>
  std::uint64_t SqlUidDataStore<C>::Reserve(std::uint64_t size) {
    auto nextUid = GetNextUid();
    m_connection->execute(Viper::update("next_uid",
      {"uid", Viper::literal(nextUid + size)}));
    return nextUid;
  }

  template<typename C>
  void SqlUidDataStore<C>::WithTransaction(
      const std::function<void ()>& transaction) {
    auto lock = std::lock_guard(m_mutex);
    Viper::transaction(*m_connection, transaction);
  }

  template<typename C>
  void SqlUidDataStore<C>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_connection->open();
      if(!m_connection->has_table("next_uid")) {
        m_connection->execute(Viper::create(GetNextUidRow(), "next_uid"));
        auto firstUid = 1;
        m_connection->execute(Viper::insert(GetNextUidRow(), "next_uid",
          &firstUid));
      }
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename C>
  void SqlUidDataStore<C>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename C>
  void SqlUidDataStore<C>::Shutdown() {
    m_connection->close();
    m_openState.SetClosed();
  }
}

#endif
