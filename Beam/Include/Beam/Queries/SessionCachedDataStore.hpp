#ifndef BEAM_SESSION_CACHED_DATA_STORE_HPP
#define BEAM_SESSION_CACHED_DATA_STORE_HPP
#include "Beam/Collections/SynchronizedMap.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queries/SessionCachedDataStoreEntry.hpp"

namespace Beam {

  /**
   * Caches the most recent writes made to a data store.
   * @tparam D The type of data store to buffer writes to.
   * @tparam F The type of EvaluatorTranslator used for filtering values.
   */
  template<typename D, typename F =
    typename dereference_t<D>::EvaluatorTranslatorFilter>
  class SessionCachedDataStore {
    public:

      /** The type of data store to buffer writes to. */
      using DataStore = dereference_t<D>;

      /** The type of query used to load values. */
      using Query = typename DataStore::Query;

      /** The type of index used. */
      using Index = typename DataStore::Index;

      /** The type of value to store. */
      using Value = typename DataStore::Value;

      /** The SequencedValue to store. */
      using SequencedValue = typename DataStore::SequencedValue;

      /** The IndexedValue to store. */
      using IndexedValue = typename DataStore::IndexedValue;

      /** The type of EvaluatorTranslator used for filtering values. */
      using EvaluationTranslatorFilter = F;

      /**
       * Constructs a SessionCachedDataStore.
       * @param data_store Initializes the data store to cache.
       * @param block_size The number of values to cache per index.
       */
      template<Initializes<D> DF>
      SessionCachedDataStore(DF&& data_store, int block_size);

      ~SessionCachedDataStore();

      std::vector<SequencedValue> load(const Query& query);
      void store(const IndexedValue& value);
      void store(const std::vector<IndexedValue>& values);
      void close();

    private:
      using SessionCachedDataStoreEntry =
        Beam::SessionCachedDataStoreEntry<DataStore*, F>;
      local_ptr_t<D> m_data_store;
      int m_block_size;
      SynchronizedUnorderedMap<Index, SessionCachedDataStoreEntry> m_caches;
      OpenState m_open_state;

      SessionCachedDataStore(const SessionCachedDataStore&) = delete;
      SessionCachedDataStore& operator =(
        const SessionCachedDataStore&) = delete;
      SessionCachedDataStoreEntry& load_cache(const Index& index);
  };

  template<typename D, typename F>
  template<Initializes<D> DF>
  SessionCachedDataStore<D, F>::SessionCachedDataStore(
    DF&& data_store, int block_size)
    : m_data_store(std::forward<DF>(data_store)),
      m_block_size(block_size) {}

  template<typename D, typename F>
  SessionCachedDataStore<D, F>::~SessionCachedDataStore() {
    close();
  }

  template<typename D, typename F>
  std::vector<typename SessionCachedDataStore<D, F>::SequencedValue>
      SessionCachedDataStore<D, F>::load(const Query& query) {
    auto& cache = load_cache(query.get_index());
    return cache.load(query);
  }

  template<typename D, typename F>
  void SessionCachedDataStore<D, F>::store(const IndexedValue& value) {
    auto& cache = load_cache(value->get_index());
    cache.store(value);
    m_data_store->store(value);
  }

  template<typename D, typename F>
  void SessionCachedDataStore<D, F>::store(
      const std::vector<IndexedValue>& values) {
    for(auto& value : values) {
      auto& cache = load_cache(value->get_index());
      cache.store(value);
    }
    m_data_store->store(values);
  }

  template<typename D, typename F>
  void SessionCachedDataStore<D, F>::close() {
    m_open_state.close();
  }

  template<typename D, typename F>
  typename SessionCachedDataStore<D, F>::SessionCachedDataStoreEntry&
      SessionCachedDataStore<D, F>::load_cache(const Index& index) {
    return m_caches.test_and_set(index, [&] (auto& caches) {
      caches.emplace(std::piecewise_construct, std::forward_as_tuple(index),
        std::forward_as_tuple(&*m_data_store, m_block_size));
    });
  }
}

#endif
