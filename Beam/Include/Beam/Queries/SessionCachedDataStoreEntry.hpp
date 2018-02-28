#ifndef BEAM_SESSIONCACHEDDATASTOREENTRY_HPP
#define BEAM_SESSIONCACHEDDATASTOREENTRY_HPP
#include <atomic>
#include <boost/thread/mutex.hpp>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queries/LocalDataStoreEntry.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/Sequence.hpp"
#include "Beam/Threading/CallOnce.hpp"
#include "Beam/Threading/Mutex.hpp"

namespace Beam {
namespace Queries {

  /*! \class SessionCachedDataStoreEntry
      \brief Caches the most recent writes made to a data store for a single
             index.
      \tparam DataStoreType The type of data store to buffer writes to.
      \tparam EvaluatorTranslatorFilterType The type of EvaluatorTranslator used
              for filtering values.
   */
  template<typename DataStoreType, typename EvaluatorTranslatorFilterType =
    typename GetTryDereferenceType<DataStoreType>::EvaluatorTranslatorFilter>
  class SessionCachedDataStoreEntry {
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
      using EvaluatorTranslatorFilter = EvaluatorTranslatorFilterType;

      //! Constructs a SessionCachedDataStoreEntry.
      /*!
        \param dataStore Initializes the data store to cache.
        \param blockSize The size of a single cache block.
      */
      template<typename DataStoreForward>
      SessionCachedDataStoreEntry(DataStoreForward&& dataStore, int blockSize);

      std::vector<SequencedValue> Load(const Query& query);

      void Store(const IndexedValue& value);

    private:
      using LocalDataStoreEntry = ::Beam::Queries::LocalDataStoreEntry<Query,
        Value, EvaluatorTranslatorFilterType>;
      struct DataStoreEntry {
        LocalDataStoreEntry m_dataStore;
        boost::posix_time::ptime m_timestamp;
        Sequence m_sequence;
        std::atomic_int m_size;

        DataStoreEntry(boost::posix_time::ptime timestamp, Sequence sequence);
      };
      mutable boost::mutex m_mutex;
      GetOptionalLocalPtr<DataStoreType> m_dataStore;
      int m_blockSize;
      Threading::CallOnce<Threading::Mutex> m_initializer;
      std::shared_ptr<DataStoreEntry> m_cache;

      std::shared_ptr<DataStoreEntry> InitializeCache(const Index& index);
  };

  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
  SessionCachedDataStoreEntry<DataStoreType, EvaluatorTranslatorFilterType>::
      DataStoreEntry::DataStoreEntry(boost::posix_time::ptime timestamp,
      Sequence sequence)
      : m_timestamp{timestamp},
        m_sequence{sequence},
        m_size{0} {}

  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
  template<typename DataStoreForward>
  SessionCachedDataStoreEntry<DataStoreType, EvaluatorTranslatorFilterType>::
      SessionCachedDataStoreEntry(DataStoreForward&& dataStore, int blockSize)
      : m_dataStore{std::forward<DataStoreForward>(dataStore)},
        m_blockSize{blockSize} {}

  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
  std::vector<typename SessionCachedDataStoreEntry<DataStoreType,
      EvaluatorTranslatorFilterType>::SequencedValue>
      SessionCachedDataStoreEntry<DataStoreType,
      EvaluatorTranslatorFilterType>::Load(const Query& query) {
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

  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
  void SessionCachedDataStoreEntry<DataStoreType,
      EvaluatorTranslatorFilterType>::Store(const IndexedValue& value) {
    if(m_blockSize == 0) {
      return;
    }
    auto cache = InitializeCache(value->GetIndex());
    auto size = cache->m_size.load();
    if(size > 2 * m_blockSize) {
      boost::lock_guard<boost::mutex> lock{m_mutex};
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

  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
  std::shared_ptr<typename SessionCachedDataStoreEntry<DataStoreType,
      EvaluatorTranslatorFilterType>::DataStoreEntry>
      SessionCachedDataStoreEntry<DataStoreType,
      EvaluatorTranslatorFilterType>::InitializeCache(const Index& index) {
    m_initializer.Call(
      [&] {
        Query query;
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
    boost::lock_guard<boost::mutex> lock{m_mutex};
    return m_cache;
  }
}
}

#endif
