#ifndef BEAM_CACHED_DATA_STORE_HPP
#define BEAM_CACHED_DATA_STORE_HPP
#include "Beam/Collections/SynchronizedMap.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queries/CachedDataStoreEntry.hpp"

namespace Beam {

  /**
   * Caches data read from a data store.
   * @tparam D The type of data store to buffer writes to.
   * @tparam F The type of EvaluatorTranslator used for filtering values.
   */
  template<typename D,
    typename F = typename dereference_t<D>::EvaluatorTranslatorFilter>
  class CachedDataStore {
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
       * Constructs a CachedDataStore.
       * @param data_store Initializes the data store to cache.
       * @param block_size The size of a single cache block.
       */
      template<Initializes<D> DF>
      CachedDataStore(DF&& data_store, int block_size);

      ~CachedDataStore();

      std::vector<SequencedValue> load(const Query& query);
      void store(const IndexedValue& value);
      void store(const std::vector<IndexedValue>& values);
      void close();

    private:
      using CachedDataStoreEntry = Beam::CachedDataStoreEntry<DataStore*, F>;
      local_ptr_t<D> m_data_store;
      int m_block_size;
      SynchronizedUnorderedMap<Index, CachedDataStoreEntry> m_caches;
      OpenState m_open_state;

      CachedDataStore(const CachedDataStore&) = delete;
      CachedDataStore& operator =(const CachedDataStore&) = delete;
      CachedDataStoreEntry& load_cache(const Index& index);
  };

  template<typename D, typename F>
  template<Initializes<D> DF>
  CachedDataStore<D, F>::CachedDataStore(DF&& data_store, int block_size)
    : m_data_store(std::forward<DF>(data_store)),
      m_block_size(block_size) {}

  template<typename D, typename F>
  CachedDataStore<D, F>::~CachedDataStore() {
    close();
  }

  template<typename D, typename F>
  std::vector<typename CachedDataStore<D, F>::SequencedValue>
      CachedDataStore<D, F>::load(const Query& query) {
    auto& cache = load_cache(query.get_index());
    return cache.load(query);
  }

  template<typename D, typename F>
  void CachedDataStore<D, F>::store(const IndexedValue& value) {
    m_data_store->store(value);
    auto& cache = load_cache(value->get_index());
    cache.store(value);
  }

  template<typename D, typename F>
  void CachedDataStore<D, F>::store(const std::vector<IndexedValue>& values) {
    m_data_store->store(values);
    for(auto& value : values) {
      auto& cache = load_cache(value->get_index());
      cache.store(value);
    }
  }

  template<typename D, typename F>
  void CachedDataStore<D, F>::close() {
    m_open_state.close();
  }

  template<typename D, typename F>
  typename CachedDataStore<D, F>::CachedDataStoreEntry&
      CachedDataStore<D, F>::load_cache(const Index& index) {
    return m_caches.test_and_set(index, [&] (auto& caches) {
      caches.emplace(std::piecewise_construct, std::forward_as_tuple(index),
        std::forward_as_tuple(&*m_data_store, index, m_block_size));
    });
  }
}

#endif
