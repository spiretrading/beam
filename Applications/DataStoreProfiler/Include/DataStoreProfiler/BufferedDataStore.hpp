#ifndef BEAM_DATA_STORE_PROFILER_BUFFERED_DATA_STORE_HPP
#define BEAM_DATA_STORE_PROFILER_BUFFERED_DATA_STORE_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queries/BufferedDataStore.hpp>
#include <Beam/Queries/EvaluatorTranslator.hpp>
#include "DataStoreProfiler/DataStoreQueryWrapper.hpp"
#include "DataStoreProfiler/EntryQuery.hpp"

namespace Beam {

  /**
   * Buffers writes to an underlying data store.
   * @param <D> The underlying data store to commit the data to.
   */
  template<typename D>
  class BufferedDataStore {
    public:

      /** The type of DataStore to buffer. */
      using BaseDataStore = GetTryDereferenceType<D>;

      /**
       * Constructs a BufferedDataStore.
       * @param dataStore Initializes the data store to commit data to.
       * @param bufferSize The number of messages to buffer before committing to
       *        to the <i>dataStore</i>.
       */
      template<typename DF>
      BufferedDataStore(DF&& dataStore, std::size_t bufferSize);

      ~BufferedDataStore();

      void Clear();

      std::vector<SequencedEntry> LoadEntries(const EntryQuery& query);

      void Store(const SequencedIndexedEntry& entry);

      void Store(const std::vector<SequencedIndexedEntry>& entries);

      void Close();

    private:
      GetOptionalLocalPtr<D> m_dataStore;
      Queries::BufferedDataStore<DataStoreQueryWrapper<BaseDataStore*>,
        Queries::EvaluatorTranslator<Queries::QueryTypes>> m_bufferedDataStore;
      IO::OpenState m_openState;

      BufferedDataStore(const BufferedDataStore&) = delete;
      BufferedDataStore& operator =(const BufferedDataStore&) = delete;
  };

  template<typename DF>
  BufferedDataStore(DF&&, std::size_t) ->
    BufferedDataStore<std::remove_reference_t<DF>>;

  template<typename D>
  template<typename DF>
  BufferedDataStore<D>::BufferedDataStore(DF&& dataStore,
    std::size_t bufferSize)
    : m_dataStore(std::forward<DF>(dataStore)),
      m_bufferedDataStore(&*m_dataStore, bufferSize) {}

  template<typename D>
  BufferedDataStore<D>::~BufferedDataStore() {
    Close();
  }

  template<typename D>
  void BufferedDataStore<D>::Clear() {
    m_dataStore->Clear();
  }

  template<typename D>
  std::vector<SequencedEntry> BufferedDataStore<D>::LoadEntries(
      const EntryQuery& query) {
    return m_bufferedDataStore.Load(query);
  }

  template<typename D>
  void BufferedDataStore<D>::Store(const SequencedIndexedEntry& entry) {
    m_bufferedDataStore.Store(entry);
  }

  template<typename D>
  void BufferedDataStore<D>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_bufferedDataStore.Close();
    m_dataStore->Close();
    m_openState.Close();
  }
}

#endif
