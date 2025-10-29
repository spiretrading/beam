#ifndef BEAM_DATA_STORE_PROFILER_SESSION_CACHED_DATA_STORE_HPP
#define BEAM_DATA_STORE_PROFILER_SESSION_CACHED_DATA_STORE_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Queries/EvaluatorTranslator.hpp>
#include <Beam/Queries/SessionCachedDataStore.hpp>
#include "DataStoreProfiler/DataStoreQueryWrapper.hpp"
#include "DataStoreProfiler/EntryQuery.hpp"

namespace Beam {

  /**
   * Caches ongoing writes for quicker read access.
   * @tparam D The underlying data store to cache.
   */
  template<typename D>
  class SessionCachedDataStore {
    public:

      /** The type of DataStore to buffer. */
      using BaseDataStore = dereference_t<D>;

      /**
       * Constructs a SessionCachedDataStore.
       * @param data_store Initializes the data store to commit data to.
       * @param block_size The number of messages to cache per index.
       */
      template<Initializes<D> DF>
      SessionCachedDataStore(DF&& data_store, int block_size);

      ~SessionCachedDataStore();

      void clear();
      std::vector<SequencedEntry> load_entries(const EntryQuery& query);
      void store(const SequencedIndexedEntry& entry);
      void store(const std::vector<SequencedIndexedEntry>& entries);
      void close();

    private:
      local_ptr_t<D> m_data_store;
      SessionCachedDataStore<DataStoreQueryWrapper<BaseDataStore*>,
        EvaluatorTranslator<QueryTypes>> m_cached_data_store;
      OpenState m_open_state;

      SessionCachedDataStore(const SessionCachedDataStore&) = delete;
      SessionCachedDataStore& operator =(const SessionCachedDataStore&) =
        delete;
      void shutdown();
  };

  template<typename D>
  SessionCachedDataStore(D&&, int) ->
    SessionCachedDataStore<std::remove_cvref_t<D>>;

  template<typename D>
  template<Initializes<D> DF>
  SessionCachedDataStore<D>::SessionCachedDataStore(
    DF&& data_store, int block_size)
    : m_data_store(std::forward<DF>(data_store)),
      m_cached_data_store(&*m_data_store, block_size) {}

  template<typename D>
  SessionCachedDataStore<D>::~SessionCachedDataStore() {
    close();
  }

  template<typename D>
  void SessionCachedDataStore<D>::clear() {
    m_data_store->clear();
  }

  template<typename D>
  std::vector<SequencedEntry> SessionCachedDataStore<D>::load_entries(
      const EntryQuery& query) {
    return m_cached_data_store.load(query);
  }

  template<typename D>
  void SessionCachedDataStore<D>::store(const SequencedIndexedEntry& entry) {
    m_cached_data_store.store(entry);
  }

  template<typename D>
  void SessionCachedDataStore<D>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    shutdown();
  }

  template<typename D>
  void SessionCachedDataStore<D>::shutdown() {
    m_cached_data_store.close();
    m_data_store->close();
    m_open_state.set_closed();
  }
}

#endif
