#ifndef BEAM_DATA_STORE_PROFILER_ASYNC_DATA_STORE_HPP
#define BEAM_DATA_STORE_PROFILER_ASYNC_DATA_STORE_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queries/AsyncDataStore.hpp>
#include <Beam/Queries/EvaluatorTranslator.hpp>
#include "DataStoreProfiler/DataStoreQueryWrapper.hpp"
#include "DataStoreProfiler/EntryQuery.hpp"

namespace Beam {

  /**
   * Performs asyncronous writes to an underlying data store.
   * @param <D> The underlying data store to commit the data to.
   */
  template<typename D>
  class AsyncDataStore {
    public:

      /** The type of DataStore to buffer. */
      using BaseDataStore = GetTryDereferenceType<D>;

      /**
       * Constructs an AsyncDataStore.
       * @param dataStore Initializes the data store to commit data to.
       */
      template<typename DF>
      AsyncDataStore(DF&& dataStore);

      ~AsyncDataStore();

      void Clear();

      std::vector<SequencedEntry> LoadEntries(const EntryQuery& query);

      void Store(const SequencedIndexedEntry& entry);

      void Store(const std::vector<SequencedIndexedEntry>& entries);

      void Close();

    private:
      GetOptionalLocalPtr<D> m_dataStore;
      Queries::AsyncDataStore<DataStoreQueryWrapper<BaseDataStore*>,
        Queries::EvaluatorTranslator<Queries::QueryTypes>> m_asyncDataStore;
      IO::OpenState m_openState;

      AsyncDataStore(const AsyncDataStore&) = delete;
      AsyncDataStore& operator =(const AsyncDataStore&) = delete;
  };

  template<typename DF>
  AsyncDataStore(DF&&) -> AsyncDataStore<std::remove_reference_t<DF>>;

  template<typename D>
  template<typename DF>
  AsyncDataStore<D>::AsyncDataStore(DF&& dataStore)
    : m_dataStore(std::forward<DF>(dataStore)),
      m_asyncDataStore(&*m_dataStore) {}

  template<typename D>
  AsyncDataStore<D>::~AsyncDataStore() {
    Close();
  }

  template<typename D>
  void AsyncDataStore<D>::Clear() {
    m_dataStore->Clear();
  }

  template<typename D>
  std::vector<SequencedEntry> AsyncDataStore<D>::LoadEntries(
      const EntryQuery& query) {
    return m_asyncDataStore.Load(query);
  }

  template<typename D>
  void AsyncDataStore<D>::Store(const SequencedIndexedEntry& entry) {
    m_asyncDataStore.Store(entry);
  }

  template<typename D>
  void AsyncDataStore<D>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_asyncDataStore.Close();
    m_dataStore->Close();
    m_openState.Close();
  }
}

#endif
