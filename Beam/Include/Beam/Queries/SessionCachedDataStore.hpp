#ifndef BEAM_SESSION_CACHED_DATA_STORE_HPP
#define BEAM_SESSION_CACHED_DATA_STORE_HPP
#include "Beam/Collections/SynchronizedMap.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/SessionCachedDataStoreEntry.hpp"

namespace Beam::Queries {

  /**
   * Caches the most recent writes made to a data store.
   * @param <D> The type of data store to buffer writes to.
   * @param <F> The type of EvaluatorTranslator used for filtering values.
   */
  template<typename D, typename F =
    typename GetTryDereferenceType<D>::EvaluatorTranslatorFilter>
  class SessionCachedDataStore {
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
       * Constructs a SessionCachedDataStore.
       * @param dataStore Initializes the data store to cache.
       * @param blockSize The number of values to cache per index.
       */
      template<typename DF>
      SessionCachedDataStore(DF&& dataStore, int blockSize);

      ~SessionCachedDataStore();

      std::vector<SequencedValue> Load(const Query& query);

      void Store(const IndexedValue& value);

      void Store(const std::vector<IndexedValue>& values);

      void Close();

    private:
      using SessionCachedDataStoreEntry =
        ::Beam::Queries::SessionCachedDataStoreEntry<DataStore*, F>;
      GetOptionalLocalPtr<D> m_dataStore;
      int m_blockSize;
      SynchronizedUnorderedMap<Index, SessionCachedDataStoreEntry> m_caches;
      IO::OpenState m_openState;

      SessionCachedDataStore(const SessionCachedDataStore&) = delete;
      SessionCachedDataStore& operator =(
        const SessionCachedDataStore&) = delete;
      SessionCachedDataStoreEntry& LoadCache(const Index& index);
  };

  template<typename D, typename F>
  template<typename DF>
  SessionCachedDataStore<D, F>::SessionCachedDataStore(DF&& dataStore,
    int blockSize)
    : m_dataStore(std::forward<DF>(dataStore)),
      m_blockSize(blockSize) {}

  template<typename D, typename F>
  SessionCachedDataStore<D, F>::~SessionCachedDataStore() {
    Close();
  }

  template<typename D, typename F>
  std::vector<typename SessionCachedDataStore<D, F>::SequencedValue>
      SessionCachedDataStore<D, F>::Load(const Query& query) {
    auto& cache = LoadCache(query.GetIndex());
    return cache.Load(query);
  }

  template<typename D, typename F>
  void SessionCachedDataStore<D, F>::Store(const IndexedValue& value) {
    auto& cache = LoadCache(value->GetIndex());
    cache.Store(value);
    m_dataStore->Store(value);
  }

  template<typename D, typename F>
  void SessionCachedDataStore<D, F>::Store(
      const std::vector<IndexedValue>& values) {
    for(const auto& value : values) {
      auto& cache = LoadCache(value->GetIndex());
      cache.Store(value);
    }
    m_dataStore->Store(values);
  }

  template<typename D, typename F>
  void SessionCachedDataStore<D, F>::Close() {
    m_openState.Close();
  }

  template<typename D, typename F>
  typename SessionCachedDataStore<D, F>::SessionCachedDataStoreEntry&
      SessionCachedDataStore<D, F>::LoadCache(const Index& index) {
    return m_caches.TestAndSet(index, [&] (auto& caches) {
      caches.emplace(std::piecewise_construct, std::forward_as_tuple(index),
        std::forward_as_tuple(&*m_dataStore, m_blockSize));
    });
  }
}

#endif
