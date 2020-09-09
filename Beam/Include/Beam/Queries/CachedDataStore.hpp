#ifndef BEAM_CACHED_DATA_STORE_HPP
#define BEAM_CACHED_DATA_STORE_HPP
#include "Beam/Collections/SynchronizedMap.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queries/CachedDataStoreEntry.hpp"
#include "Beam/Queries/Queries.hpp"

namespace Beam::Queries {

  /**
   * Caches data read from a data store.
   * @param <D> The type of data store to buffer writes to.
   * @param <F> The type of EvaluatorTranslator used for filtering values.
   */
  template<typename D, typename F =
    typename GetTryDereferenceType<D>::EvaluatorTranslatorFilter>
  class CachedDataStore {
    public:

      /** The type of data store to buffer writes to. */
      using DataStore = GetTryDereferenceType<D>;

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
       * @param dataStore Initializes the data store to cache.
       * @param blockSize The size of a single cache block.
       */
      template<typename DF>
      CachedDataStore(DF&& dataStore, int blockSize);

      ~CachedDataStore();

      std::vector<SequencedValue> Load(const Query& query);

      void Store(const IndexedValue& value);

      void Store(const std::vector<IndexedValue>& values);

      void Close();

    private:
      using CachedDataStoreEntry = ::Beam::Queries::CachedDataStoreEntry<
        DataStore*, F>;
      GetOptionalLocalPtr<D> m_dataStore;
      int m_blockSize;
      SynchronizedUnorderedMap<Index, CachedDataStoreEntry> m_caches;
      IO::OpenState m_openState;

      CachedDataStore(const CachedDataStore&) = delete;
      CachedDataStore& operator =(const CachedDataStore&) = delete;
      CachedDataStoreEntry& LoadCache(const Index& index);
  };

  template<typename D, typename F>
  template<typename DF>
  CachedDataStore<D, F>::CachedDataStore(DF&& dataStore, int blockSize)
    : m_dataStore(std::forward<DF>(dataStore)),
      m_blockSize(blockSize) {}

  template<typename D, typename F>
  CachedDataStore<D, F>::~CachedDataStore() {
    Close();
  }

  template<typename D, typename F>
  std::vector<typename CachedDataStore<D, F>::SequencedValue>
      CachedDataStore<D, F>::Load(const Query& query) {
    auto& cache = LoadCache(query.GetIndex());
    return cache.Load(query);
  }

  template<typename D, typename F>
  void CachedDataStore<D, F>::Store(const IndexedValue& value) {
    m_dataStore->Store(value);
    auto& cache = LoadCache(value->GetIndex());
    cache.Store(value);
  }

  template<typename D, typename F>
  void CachedDataStore<D, F>::Store(const std::vector<IndexedValue>& values) {
    m_dataStore->Store(values);
    for(auto& value : values) {
      auto& cache = LoadCache(value->GetIndex());
      cache.Store(value);
    }
  }

  template<typename D, typename F>
  void CachedDataStore<D, F>::Close() {
    m_openState.Close();
  }

  template<typename D, typename F>
  typename CachedDataStore<D, F>::CachedDataStoreEntry&
      CachedDataStore<D, F>::LoadCache(const Index& index) {
    return m_caches.TestAndSet(index, [&] (auto& caches) {
      caches.emplace(std::piecewise_construct, std::forward_as_tuple(index),
        std::forward_as_tuple(&*m_dataStore, index, m_blockSize));
    });
  }
}

#endif
