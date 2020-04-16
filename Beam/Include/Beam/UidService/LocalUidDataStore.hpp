#ifndef BEAM_LOCALUIDDATASTORE_HPP
#define BEAM_LOCALUIDDATASTORE_HPP
#include "Beam/IO/OpenState.hpp"
#include "Beam/Threading/Mutex.hpp"
#include "Beam/UidService/UidDataStore.hpp"

namespace Beam {
namespace UidService {

  /*! \class LocalUidDataStore
      \brief Implements an in memory UidDataStore.
   */
  class LocalUidDataStore : public UidDataStore {
    public:

      //! Constructs a LocalUidDataStore.
      LocalUidDataStore();

      virtual ~LocalUidDataStore() override;

      virtual std::uint64_t GetNextUid() override;

      virtual std::uint64_t Reserve(std::uint64_t size) override;

      virtual void WithTransaction(
        const std::function<void ()>& transaction) override;

      virtual void Open() override;

      virtual void Close() override;

    private:
      mutable Threading::Mutex m_mutex;
      std::uint64_t m_nextUid;
      IO::OpenState m_openState;
  };

  inline LocalUidDataStore::LocalUidDataStore()
      : m_nextUid{1} {}

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
    boost::lock_guard<Threading::Mutex> lock{m_mutex};
    transaction();
  }

  inline void LocalUidDataStore::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    m_openState.SetOpen();
  }

  inline void LocalUidDataStore::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_openState.SetClosed();
  }
}
}

#endif
