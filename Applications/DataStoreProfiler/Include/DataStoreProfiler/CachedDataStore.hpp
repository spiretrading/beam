#ifndef BEAM_DATA_STORE_PROFILER_CACHED_DATA_STORE_HPP
#define BEAM_DATA_STORE_PROFILER_CACHED_DATA_STORE_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Queries/CachedDataStore.hpp>
#include <Beam/Queries/EvaluatorTranslator.hpp>
#include "DataStoreProfiler/DataStoreQueryWrapper.hpp"
#include "DataStoreProfiler/EntryQuery.hpp"

namespace Beam {

  /**
   * Caches ongoing writes for quicker read access.
   * @tparam D The underlying data store to cache.
   */
  template<typename D>
  class CachedProfileDataStore {
    public:

      /** The type of DataStore to buffer. */
      using BaseDataStore = dereference_t<D>;

      /**
       * Constructs a CachedProfileDataStore.
       * @param data_store Initializes the data store to commit data to.
       * @param block_size The number of messages to cache per index.
       */
      template<Initializes<D> DF>
      CachedProfileDataStore(DF&& data_store, int block_size);

      ~CachedProfileDataStore();

      void clear();
      std::vector<SequencedEntry> load_entries(const EntryQuery& query);
      void store(const SequencedIndexedEntry& entry);
      void store(const std::vector<SequencedIndexedEntry>& entries);
      void close();

    private:
      local_ptr_t<D> m_data_store;
      CachedDataStore<DataStoreQueryWrapper<BaseDataStore*>,
        EvaluatorTranslator<QueryTypes>> m_cached_data_store;
      OpenState m_open_state;

      CachedProfileDataStore(const CachedProfileDataStore&) = delete;
      CachedProfileDataStore& operator =(const CachedProfileDataStore&) =
        delete;
      void shutdown();
  };

  template<typename D>
  CachedProfileDataStore(D&&, int) ->
    CachedProfileDataStore<std::remove_cvref_t<D>>;

  template<typename D>
  template<Initializes<D> DF>
  CachedProfileDataStore<D>::CachedProfileDataStore(
    DF&& data_store, int block_size)
    : m_data_store(std::forward<DF>(data_store)),
      m_cached_data_store(&*m_data_store, block_size) {}

  template<typename D>
  CachedProfileDataStore<D>::~CachedProfileDataStore() {
    close();
  }

  template<typename D>
  void CachedProfileDataStore<D>::clear() {
    m_data_store->clear();
  }

  template<typename D>
  std::vector<SequencedEntry> CachedProfileDataStore<D>::load_entries(
      const EntryQuery& query) {
    return m_cached_data_store.load(query);
  }

  template<typename D>
  void CachedProfileDataStore<D>::store(const SequencedIndexedEntry& entry) {
    m_cached_data_store.store(entry);
  }

  template<typename D>
  void CachedProfileDataStore<D>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    shutdown();
  }

  template<typename D>
  void CachedProfileDataStore<D>::shutdown() {
    m_cached_data_store.close();
    m_data_store->close();
    m_open_state.set_closed();
  }
}

#endif
