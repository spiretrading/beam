#ifndef BEAM_CACHEDDATASTORE_HPP
#define BEAM_CACHEDDATASTORE_HPP
#include <boost/noncopyable.hpp>
#include "Beam/Collections/SynchronizedMap.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queries/CachedDataStoreEntry.hpp"
#include "Beam/Queries/Queries.hpp"

namespace Beam {
namespace Queries {

  /*! \class CachedDataStore
      \brief Caches data read from a data store.
      \tparam DataStoreType The type of data store to buffer writes to.
      \tparam EvaluatorTranslatorFilterType The type of EvaluatorTranslator used
              for filtering values.
   */
  template<typename DataStoreType, typename EvaluatorTranslatorFilterType =
    typename GetTryDereferenceType<DataStoreType>::EvaluatorTranslatorFilter>
  class CachedDataStore : private boost::noncopyable {
    public:

      //! The type of data store to buffer writes to.
      using DataStore = GetTryDereferenceType<DataStoreType>;

      //! The type of query used to load values.
      using Query = typename DataStore::Query;

      //! The type of index used.
      using Index = typename DataStore::Index;

      //! The type of value to store.
      using Value = typename DataStore::Value;

      //! The SequencedValue to store.
      using SequencedValue = typename DataStore::SequencedValue;

      //! The IndexedValue to store.
      using IndexedValue = typename DataStore::IndexedValue;

      //! The type of EvaluatorTranslator used for filtering values.
      using EvaluationTranslatorFilter = EvaluatorTranslatorFilterType;

      //! Constructs a CachedDataStore.
      /*!
        \param dataStore Initializes the data store to cache.
        \param blockSize The size of a single cache block.
      */
      template<typename DataStoreForward>
      CachedDataStore(DataStoreForward&& dataStore, int blockSize);

      ~CachedDataStore();

      std::vector<SequencedValue> Load(const Query& query);

      void Store(const IndexedValue& value);

      void Store(const std::vector<IndexedValue>& values);

      void Open();

      void Close();

    private:
      using CachedDataStoreEntry = ::Beam::Queries::CachedDataStoreEntry<
        DataStore*, EvaluatorTranslatorFilterType>;
      GetOptionalLocalPtr<DataStoreType> m_dataStore;
      int m_blockSize;
      SynchronizedUnorderedMap<Index, CachedDataStoreEntry> m_caches;
      IO::OpenState m_openState;

      void Shutdown();
      CachedDataStoreEntry& LoadCache(const Index& index);
  };

  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
  template<typename DataStoreForward>
  CachedDataStore<DataStoreType, EvaluatorTranslatorFilterType>::
      CachedDataStore(DataStoreForward&& dataStore, int blockSize)
      : m_dataStore(std::forward<DataStoreForward>(dataStore)),
        m_blockSize(blockSize) {}

  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
  CachedDataStore<DataStoreType, EvaluatorTranslatorFilterType>::
      ~CachedDataStore() {
    Close();
  }

  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
  std::vector<typename CachedDataStore<DataStoreType,
      EvaluatorTranslatorFilterType>::SequencedValue>
      CachedDataStore<DataStoreType, EvaluatorTranslatorFilterType>::Load(
      const Query& query) {
    auto& cache = LoadCache(query.GetIndex());
    return cache.Load(query);
  }

  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
  void CachedDataStore<DataStoreType, EvaluatorTranslatorFilterType>::Store(
      const IndexedValue& value) {
    m_dataStore->Store(value);
    auto& cache = LoadCache(value->GetIndex());
    cache.Store(value);
  }

  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
  void CachedDataStore<DataStoreType, EvaluatorTranslatorFilterType>::Store(
      const std::vector<IndexedValue>& values) {
    m_dataStore->Store(values);
    for(const auto& value : values) {
      auto& cache = LoadCache(value->GetIndex());
      cache.Store(value);
    }
  }

  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
  void CachedDataStore<DataStoreType, EvaluatorTranslatorFilterType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_dataStore->Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
  void CachedDataStore<DataStoreType, EvaluatorTranslatorFilterType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
  void CachedDataStore<DataStoreType, EvaluatorTranslatorFilterType>::
      Shutdown() {
    m_openState.SetClosed();
  }

  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
  typename CachedDataStore<DataStoreType, EvaluatorTranslatorFilterType>::
      CachedDataStoreEntry& CachedDataStore<DataStoreType,
      EvaluatorTranslatorFilterType>::LoadCache(const Index& index) {
    return m_caches.TestAndSet(index,
      [&] (std::unordered_map<Index, CachedDataStoreEntry>& caches) {
        caches.emplace(std::piecewise_construct, std::forward_as_tuple(index),
          std::forward_as_tuple(&*m_dataStore, index, m_blockSize));
      });
  }
}
}

#endif
