#ifndef BEAM_LOCAL_UID_DATA_STORE_HPP
#define BEAM_LOCAL_UID_DATA_STORE_HPP
#include "Beam/IO/OpenState.hpp"
#include "Beam/Threading/Mutex.hpp"
#include "Beam/UidService/UidDataStore.hpp"

namespace Beam::UidService {

  /** Implements an in memory UidDataStore. */
  class LocalUidDataStore : public UidDataStore {
    public:

      /** Constructs a LocalUidDataStore starting from a UID of 1. */
      LocalUidDataStore();

      ~LocalUidDataStore() override;

      std::uint64_t GetNextUid() override;

      std::uint64_t Reserve(std::uint64_t size) override;

      void WithTransaction(const std::function<void ()>& transaction) override;

      void Close() override;

    private:
      mutable Threading::Mutex m_mutex;
      std::uint64_t m_nextUid;
      IO::OpenState m_openState;
  };

  inline LocalUidDataStore::LocalUidDataStore()
    : m_nextUid(1) {}

  inline LocalUidDataStore::~LocalUidDataStore() {
    Close();
  }

  inline std::uint64_t LocalUidDataStore::GetNextUid() {
    return m_nextUid;
  }

  inline std::uint64_t LocalUidDataStore::Reserve(std::uint64_t size) {
    auto nextUid = m_nextUid;
    m_nextUid += size;
    return nextUid;
  }

  inline void LocalUidDataStore::WithTransaction(
      const std::function<void ()>& transaction) {
    auto lock = boost::lock_guard(m_mutex);
    transaction();
  }

  inline void LocalUidDataStore::Close() {
    m_openState.Close();
  }
}

#endif
