#ifndef BEAM_DATA_STORE_PROFILER_SESSION_CACHED_DATASTORE_HPP
#define BEAM_DATA_STORE_PROFILER_SESSION_CACHED_DATASTORE_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Queries/SessionCachedDataStore.hpp>
#include <Beam/Queries/EvaluatorTranslator.hpp>
#include <boost/noncopyable.hpp>
#include "DataStoreProfiler/DataStoreQueryWrapper.hpp"
#include "DataStoreProfiler/EntryQuery.hpp"

namespace Beam {

  /*! \class SessionCachedDataStore
      \brief Caches ongoing writes for quicker read access.
      \tparam BaseDataStoreType The underlying data store to cache.
   */
  template<typename BaseDataStoreType>
  class SessionCachedDataStore : private boost::noncopyable {
    public:

      //! The type of DataStore to buffer.
      using BaseDataStore = GetTryDereferenceType<BaseDataStoreType>;

      //! Constructs a SessionCachedDataStore.
      /*!
        \param dataStore Initializes the data store to commit data to.
        \param blockSize The number of messages to cache per index.
      */
      template<typename BaseDataStoreForward>
      SessionCachedDataStore(BaseDataStoreForward&& dataStore, int blockSize);

      ~SessionCachedDataStore();

      void Clear();

      std::vector<SequencedEntry> LoadEntries(const EntryQuery& query);

      void Store(const SequencedIndexedEntry& entry);

      void Store(const std::vector<SequencedIndexedEntry>& entries);

      void Open();

      void Close();

    private:
      GetOptionalLocalPtr<BaseDataStoreType> m_dataStore;
      Queries::SessionCachedDataStore<DataStoreQueryWrapper<BaseDataStore*>,
        Queries::EvaluatorTranslator<Queries::QueryTypes>> m_cachedDataStore;
      IO::OpenState m_openState;

      void Shutdown();
  };

  template<typename BaseDataStoreForward>
  SessionCachedDataStore(BaseDataStoreForward&&, int) ->
    SessionCachedDataStore<std::remove_reference_t<BaseDataStoreForward>>;

  template<typename BaseDataStoreType>
  template<typename BaseDataStoreForward>
  SessionCachedDataStore<BaseDataStoreType>::SessionCachedDataStore(
    BaseDataStoreForward&& dataStore, int blockSize)
    : m_dataStore(std::forward<BaseDataStoreForward>(dataStore)),
      m_cachedDataStore(&*m_dataStore, blockSize) {}

  template<typename BaseDataStoreType>
  SessionCachedDataStore<BaseDataStoreType>::~SessionCachedDataStore() {
    Close();
  }

  template<typename BaseDataStoreType>
  void SessionCachedDataStore<BaseDataStoreType>::Clear() {
    m_dataStore->Clear();
  }

  template<typename BaseDataStoreType>
  std::vector<SequencedEntry> SessionCachedDataStore<BaseDataStoreType>::
      LoadEntries(const EntryQuery& query) {
    return m_cachedDataStore.Load(query);
  }

  template<typename BaseDataStoreType>
  void SessionCachedDataStore<BaseDataStoreType>::Store(
      const SequencedIndexedEntry& entry) {
    m_cachedDataStore.Store(entry);
  }

  template<typename BaseDataStoreType>
  void SessionCachedDataStore<BaseDataStoreType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_dataStore->Open();
      m_cachedDataStore.Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename BaseDataStoreType>
  void SessionCachedDataStore<BaseDataStoreType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename BaseDataStoreType>
  void SessionCachedDataStore<BaseDataStoreType>::Shutdown() {
    m_cachedDataStore.Close();
    m_dataStore->Close();
    m_openState.SetClosed();
  }
}

#endif
