#ifndef BEAM_LOCAL_UID_DATA_STORE_HPP
#define BEAM_LOCAL_UID_DATA_STORE_HPP
#include "Beam/IO/OpenState.hpp"
#include "Beam/Threading/Mutex.hpp"
#include "Beam/UidService/UidDataStore.hpp"

namespace Beam {

  /** Implements an in memory UidDataStore. */
  class LocalUidDataStore {
    public:

      /** Constructs a LocalUidDataStore starting from a UID of 1. */
      LocalUidDataStore();

      ~LocalUidDataStore();

      std::uint64_t get_next_uid();
      std::uint64_t reserve(std::uint64_t size);
      template<std::invocable<> F>
      decltype(auto) with_transaction(F&& transaction);
      void close();

    private:
      mutable Mutex m_mutex;
      std::uint64_t m_next_uid;
      OpenState m_open_state;
  };

  inline LocalUidDataStore::LocalUidDataStore()
    : m_next_uid(1) {}

  inline LocalUidDataStore::~LocalUidDataStore() {
    close();
  }

  inline std::uint64_t LocalUidDataStore::get_next_uid() {
    m_open_state.ensure_open();
    return m_next_uid;
  }

  inline std::uint64_t LocalUidDataStore::reserve(std::uint64_t size) {
    m_open_state.ensure_open();
    auto next_uid = m_next_uid;
    m_next_uid += size;
    return next_uid;
  }

  template<std::invocable<> F>
  decltype(auto) LocalUidDataStore::with_transaction(F&& transaction) {
    auto lock = boost::lock_guard(m_mutex);
    return std::forward<F>(transaction)();
  }

  inline void LocalUidDataStore::close() {
    m_open_state.close();
  }
}

#endif
