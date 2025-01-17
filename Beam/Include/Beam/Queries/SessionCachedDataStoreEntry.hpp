#ifndef BEAM_SESSION_CACHED_DATA_STORE_ENTRY_HPP
#define BEAM_SESSION_CACHED_DATA_STORE_ENTRY_HPP
#include <atomic>
#include <mutex>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queries/LocalDataStoreEntry.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/Sequence.hpp"
#include "Beam/Threading/CallOnce.hpp"

namespace Beam::Queries {

  /**
   * Caches the most recent writes made to a data store for a single index.
   * @param <D> The type of data store to buffer writes to.
   * @param <T> The type of EvaluatorTranslator used for filtering values.
   */
  template<typename D, typename T =
    typename GetTryDereferenceType<D>::EvaluatorTranslatorFilter>
  class SessionCachedDataStoreEntry {
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
      using EvaluatorTranslatorFilter = T;

      /**
       * Constructs a SessionCachedDataStoreEntry.
       * @param dataStore Initializes the data store to cache.
       * @param blockSize The size of a single cache block.
       */
      template<typename DF>
      SessionCachedDataStoreEntry(DF&& dataStore, int blockSize);

      std::vector<SequencedValue> Load(const Query& query);

      void Store(const IndexedValue& value);

    private:
      using LocalDataStoreEntry =
        ::Beam::Queries::LocalDataStoreEntry<Query, Value, T>;
      struct DataStoreEntry {
        LocalDataStoreEntry m_dataStore;
        boost::posix_time::ptime m_timestamp;
        Sequence m_sequence;
        std::atomic_int m_size;

        DataStoreEntry(boost::posix_time::ptime timestamp, Sequence sequence);
      };
      mutable std::mutex m_mutex;
      GetOptionalLocalPtr<D> m_dataStore;
      int m_blockSize;
      Threading::CallOnce<std::mutex> m_initializer;
      std::shared_ptr<DataStoreEntry> m_cache;

      std::shared_ptr<DataStoreEntry> InitializeCache(const Index& index);
  };

  template<typename D, typename T>
  SessionCachedDataStoreEntry<D, T>::DataStoreEntry::DataStoreEntry(
    boost::posix_time::ptime timestamp, Sequence sequence)
    : m_timestamp(timestamp),
      m_sequence(sequence),
      m_size(0) {}

  template<typename D, typename T>
  template<typename DF>
  SessionCachedDataStoreEntry<D, T>::SessionCachedDataStoreEntry(
    DF&& dataStore, int blockSize)
    : m_dataStore(std::forward<DF>(dataStore)),
      m_blockSize(blockSize) {}

  template<typename D, typename T>
  std::vector<typename SessionCachedDataStoreEntry<D, T>::SequencedValue>
      SessionCachedDataStoreEntry<D, T>::Load(const Query& query) {
    if(m_blockSize == 0) {
      return m_dataStore->Load(query);
    }
    auto cache = InitializeCache(query.GetIndex());
    if(auto start = boost::get<boost::posix_time::ptime>(
        &query.GetRange().GetStart())) {
      if(*start > cache->m_timestamp) {
        return cache->m_dataStore.Load(query);
      }
    } else if(auto start = boost::get<Sequence>(&query.GetRange().GetStart())) {
      if(*start > cache->m_sequence) {
        return cache->m_dataStore.Load(query);
      }
    }
    if(query.GetSnapshotLimit().GetType() == SnapshotLimit::Type::TAIL) {
      auto size = cache->m_size.load();
      if(query.GetSnapshotLimit().GetSize() <= size) {
        auto endRange = boost::get<Sequence>(&query.GetRange().GetEnd());
        auto endTimestamp = boost::get<boost::posix_time::ptime>(
          &query.GetRange().GetEnd());
        if(endRange != nullptr && *endRange > cache->m_sequence ||
            endTimestamp != nullptr && *endTimestamp > cache->m_timestamp) {
          auto result = cache->m_dataStore.Load(query);
          if(result.size() >= static_cast<std::size_t>(
              query.GetSnapshotLimit().GetSize())) {
            return result;
          }
        }
      }
    }
    return m_dataStore->Load(query);
  }

  template<typename D, typename T>
  void SessionCachedDataStoreEntry<D, T>::Store(const IndexedValue& value) {
    if(m_blockSize == 0) {
      return;
    }
    auto cache = InitializeCache(value->GetIndex());
    auto size = cache->m_size.load();
    if(size > 2 * m_blockSize) {
      auto lock = std::lock_guard(m_mutex);
      auto data = cache->m_dataStore.LoadAll();
      auto referenceValue = data[m_blockSize - 1];
      data.erase(data.begin(), data.begin() + m_blockSize);
      m_cache = std::make_shared<DataStoreEntry>(
        GetTimestamp(*referenceValue), referenceValue.GetSequence());
      m_cache->m_dataStore.Store(data);
      m_cache->m_size = data.size();
      cache = m_cache;
    }
    cache->m_dataStore.Store(value);
    ++cache->m_size;
  }

  template<typename D, typename T>
  std::shared_ptr<typename SessionCachedDataStoreEntry<D, T>::DataStoreEntry>
      SessionCachedDataStoreEntry<D, T>::InitializeCache(const Index& index) {
    m_initializer.Call([&] {
      auto query = Query();
      query.SetIndex(index);
      query.SetRange(Range::Total());
      query.SetSnapshotLimit(SnapshotLimit::Type::TAIL, 1);
      auto data = m_dataStore->Load(query);
      if(data.empty()) {
        m_cache = std::make_shared<DataStoreEntry>(
          boost::posix_time::neg_infin, Sequence::First());
      } else {
        m_cache = std::make_shared<DataStoreEntry>(
          GetTimestamp(*data.back()), data.back().GetSequence());
      }
    });
    auto lock = std::lock_guard(m_mutex);
    return m_cache;
  }
}

#endif
