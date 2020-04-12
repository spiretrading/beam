#ifndef BEAM_DATA_STORE_PROFILER_ASYNC_DATA_STORE_HPP
#define BEAM_DATA_STORE_PROFILER_ASYNC_DATA_STORE_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queries/AsyncDataStore.hpp>
#include <Beam/Queries/EvaluatorTranslator.hpp>
#include <boost/noncopyable.hpp>
#include "DataStoreProfiler/DataStoreQueryWrapper.hpp"
#include "DataStoreProfiler/EntryQuery.hpp"

namespace Beam {

  /** Performs asyncronous writes to an underlying data store.
      \tparam BaseDataStoreType The underlying data store to commit the data to.
   */
  template<typename BaseDataStoreType>
  class AsyncDataStore : private boost::noncopyable {
    public:

      //! The type of DataStore to buffer.
      using BaseDataStore = GetTryDereferenceType<BaseDataStoreType>;

      //! Constructs an AsyncDataStore.
      /*!
        \param dataStore Initializes the data store to commit data to.
      */
      template<typename BaseDataStoreForward>
      AsyncDataStore(BaseDataStoreForward&& dataStore);

      ~AsyncDataStore();

      void Clear();

      std::vector<SequencedEntry> LoadEntries(const EntryQuery& query);

      void Store(const SequencedIndexedEntry& entry);

      void Store(const std::vector<SequencedIndexedEntry>& entries);

      void Open();

      void Close();

    private:
      GetOptionalLocalPtr<BaseDataStoreType> m_dataStore;
      Queries::AsyncDataStore<DataStoreQueryWrapper<BaseDataStore*>,
        Queries::EvaluatorTranslator<Queries::QueryTypes>> m_asyncDataStore;
      IO::OpenState m_openState;

      void Shutdown();
  };

  template<typename BaseDataStoreForward>
  AsyncDataStore(BaseDataStoreForward&&) ->
    AsyncDataStore<std::remove_reference_t<BaseDataStoreForward>>;

  template<typename BaseDataStoreType>
  template<typename BaseDataStoreForward>
  AsyncDataStore<BaseDataStoreType>::AsyncDataStore(
    BaseDataStoreForward&& dataStore)
    : m_dataStore(std::forward<BaseDataStoreForward>(dataStore)),
      m_asyncDataStore(&*m_dataStore) {}

  template<typename BaseDataStoreType>
  AsyncDataStore<BaseDataStoreType>::~AsyncDataStore() {
    Close();
  }

  template<typename BaseDataStoreType>
  void AsyncDataStore<BaseDataStoreType>::Clear() {
    m_dataStore->Clear();
  }

  template<typename BaseDataStoreType>
  std::vector<SequencedEntry> AsyncDataStore<BaseDataStoreType>::LoadEntries(
      const EntryQuery& query) {
    return m_asyncDataStore.Load(query);
  }

  template<typename BaseDataStoreType>
  void AsyncDataStore<BaseDataStoreType>::Store(
      const SequencedIndexedEntry& entry) {
    m_asyncDataStore.Store(entry);
  }

  template<typename BaseDataStoreType>
  void AsyncDataStore<BaseDataStoreType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_dataStore->Open();
      m_asyncDataStore.Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename BaseDataStoreType>
  void AsyncDataStore<BaseDataStoreType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename BaseDataStoreType>
  void AsyncDataStore<BaseDataStoreType>::Shutdown() {
    m_asyncDataStore.Close();
    m_dataStore->Close();
    m_openState.SetClosed();
  }
}

#endif
