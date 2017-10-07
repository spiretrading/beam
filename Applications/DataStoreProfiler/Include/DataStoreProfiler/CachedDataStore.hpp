#ifndef BEAM_DATASTOREPROFILER_CACHEDDATASTORE_HPP
#define BEAM_DATASTOREPROFILER_CACHEDDATASTORE_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Queries/CachedDataStore.hpp>
#include <Beam/Queries/EvaluatorTranslator.hpp>
#include <boost/noncopyable.hpp>
#include "DataStoreProfiler/DataStoreQueryWrapper.hpp"
#include "DataStoreProfiler/EntryQuery.hpp"

namespace Beam {

  /*! \class CachedDataStore
      \brief Caches ongoing writes for quicker read access.
      \tparam BaseDataStoreType The underlying data store to cache.
   */
  template<typename BaseDataStoreType>
  class CachedDataStore : private boost::noncopyable {
    public:

      //! The type of DataStore to buffer.
      using BaseDataStore = GetTryDereferenceType<BaseDataStoreType>;

      //! Constructs a CachedDataStore.
      /*!
        \param dataStore Initializes the data store to commit data to.
        \param blockSize The number of messages to cache per index.
      */
      template<typename BaseDataStoreForward>
      CachedDataStore(BaseDataStoreForward&& dataStore, int blockSize);

      ~CachedDataStore();

      void Clear();

      std::vector<SequencedEntry> LoadEntries(const EntryQuery& query);

      void Store(const SequencedIndexedEntry& entry);

      void Store(const std::vector<SequencedIndexedEntry>& entries);

      void Open();

      void Close();

    private:
      GetOptionalLocalPtr<BaseDataStoreType> m_dataStore;
      Queries::CachedDataStore<DataStoreQueryWrapper<BaseDataStore*>,
        Queries::EvaluatorTranslator<Queries::QueryTypes>> m_cachedDataStore;
      IO::OpenState m_openState;

      void Shutdown();
  };

  template<typename BaseDataStoreType>
  template<typename BaseDataStoreForward>
  CachedDataStore<BaseDataStoreType>::CachedDataStore(
      BaseDataStoreForward&& dataStore, int blockSize)
      : m_dataStore{std::forward<BaseDataStoreForward>(dataStore)},
        m_cachedDataStore{&*m_dataStore, blockSize} {}

  template<typename BaseDataStoreType>
  CachedDataStore<BaseDataStoreType>::~CachedDataStore() {
    Close();
  }

  template<typename BaseDataStoreType>
  void CachedDataStore<BaseDataStoreType>::Clear() {
    m_dataStore->Clear();
  }

  template<typename BaseDataStoreType>
  std::vector<SequencedEntry> CachedDataStore<BaseDataStoreType>::LoadEntries(
      const EntryQuery& query) {
    return m_cachedDataStore.Load(query);
  }

  template<typename BaseDataStoreType>
  void CachedDataStore<BaseDataStoreType>::Store(
      const SequencedIndexedEntry& entry) {
    m_cachedDataStore.Store(entry);
  }

  template<typename BaseDataStoreType>
  void CachedDataStore<BaseDataStoreType>::Open() {
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
  void CachedDataStore<BaseDataStoreType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename BaseDataStoreType>
  void CachedDataStore<BaseDataStoreType>::Shutdown() {
    m_cachedDataStore.Close();
    m_dataStore->Close();
    m_openState.SetClosed();
  }
}

#endif
