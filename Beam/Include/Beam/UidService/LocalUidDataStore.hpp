#ifndef BEAM_LOCALUIDDATASTORE_HPP
#define BEAM_LOCALUIDDATASTORE_HPP
#include <boost/thread/mutex.hpp>
#include "Beam/IO/OpenState.hpp"
#include "Beam/UidService/UidDataStore.hpp"
#include "Beam/UidServiceTests/UidServiceTests.hpp"

namespace Beam {
namespace UidService {
namespace Tests {

  /*! \class LocalUidDataStore
      \brief Implements an in memory UidDataStore.
   */
  class LocalUidDataStore : public UidDataStore {
    public:

      //! Constructs a LocalUidDataStore.
      LocalUidDataStore();

      virtual ~LocalUidDataStore();

      virtual std::uint64_t GetNextUid();

      virtual std::uint64_t Reserve(std::uint64_t size);

      virtual void WithTransaction(const std::function<void ()>& transaction);

      virtual void Open();

      virtual void Close();

    private:
      mutable boost::mutex m_mutex;
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
    std::uint64_t nextUid = m_nextUid;
    m_nextUid += size;
    return nextUid;
  }

  inline void LocalUidDataStore::WithTransaction(
      const std::function<void ()>& transaction) {
    boost::lock_guard<boost::mutex> lock(m_mutex);
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
}

#endif
