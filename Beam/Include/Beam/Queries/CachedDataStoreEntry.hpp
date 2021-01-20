#ifndef BEAM_CACHED_DATA_STORE_ENTRY_HPP
#define BEAM_CACHED_DATA_STORE_ENTRY_HPP
#include "Beam/Collections/SynchronizedList.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queries/LocalDataStoreEntry.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/Sequence.hpp"
#include "Beam/Threading/CallOnce.hpp"
#include "Beam/Threading/Mutex.hpp"

namespace Beam::Queries {

  /**
   * Stores cached data for a single index.
   * @param <D> The type of data store to buffer writes to.
   * @param <T> The type of EvaluatorTranslator used for filtering values.
   */
  template<typename D, typename T =
    typename GetTryDereferenceType<D>::EvaluatorTranslatorFilter>
  class CachedDataStoreEntry {
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
       * Constructs a CachedDataStoreEntry.
       * @param dataStore Initializes the data store to cache.
       * @param index The Index to cache.
       * @param blockSize The size of a single cache block.
       */
      template<typename DF>
      CachedDataStoreEntry(DF&& dataStore, const Index& index, int blockSize);

      std::vector<SequencedValue> Load(const Query& query);

      void Store(const IndexedValue& value);

    private:
      using LocalDataStoreEntry =
        ::Beam::Queries::LocalDataStoreEntry<Query, Value, T>;
      struct DataStoreEntry {
        Sequence m_sequence;
        LocalDataStoreEntry m_dataStore;
        Threading::CallOnce<Threading::Mutex> m_initializer;

        DataStoreEntry(Sequence sequence);
      };
      GetOptionalLocalPtr<D> m_dataStore;
      Index m_index;
      int m_blockSize;
      SynchronizedVector<std::unique_ptr<DataStoreEntry>> m_dataStores;

      Sequence Normalize(Sequence sequence) const;
      Range ToSequence(const Index& index, const Range& range);
      boost::optional<LocalDataStoreEntry&> FindDataStore(Sequence sequence);
      LocalDataStoreEntry& LoadDataStore(Sequence sequence);
      std::vector<SequencedValue> LoadHead(const Query& query, Sequence start,
        Sequence end);
      std::vector<SequencedValue> LoadTail(const Query& query, Sequence start,
        Sequence end);
  };

  template<typename D, typename T>
  CachedDataStoreEntry<D, T>::DataStoreEntry::DataStoreEntry(Sequence sequence)
    : m_sequence(sequence) {}

  template<typename D, typename T>
  template<typename DF>
  CachedDataStoreEntry<D, T>::CachedDataStoreEntry(
    DF&& dataStore, const Index& index, int blockSize)
    : m_dataStore(std::forward<DF>(dataStore)),
      m_index(index),
      m_blockSize(blockSize) {}

  template<typename D, typename T>
  std::vector<typename CachedDataStoreEntry<D, T>::SequencedValue>
      CachedDataStoreEntry<D, T>::Load(const Query& query) {
    auto sequencedQuery = query;
    sequencedQuery.SetRange(ToSequence(query.GetIndex(), query.GetRange()));
    auto start =
      Normalize(boost::get<Sequence>(sequencedQuery.GetRange().GetStart()));
    auto end =
      Normalize(boost::get<Sequence>(sequencedQuery.GetRange().GetEnd()));
    if(sequencedQuery.GetSnapshotLimit().GetType() ==
        SnapshotLimit::Type::HEAD) {
      return LoadHead(sequencedQuery, start, end);
    } else {
      return LoadTail(sequencedQuery, start, end);
    }
  }

  template<typename D, typename T>
  void CachedDataStoreEntry<D, T>::Store(const IndexedValue& value) {
    auto& cachedDataStore = LoadDataStore(Normalize(value.GetSequence()));
    cachedDataStore.Store(value);
  }

  template<typename D, typename T>
  Sequence CachedDataStoreEntry<D, T>::Normalize(Sequence sequence) const {
    return
      Sequence(sequence.GetOrdinal() - (sequence.GetOrdinal() % m_blockSize));
  }

  template<typename D, typename T>
  Range CachedDataStoreEntry<D, T>::ToSequence(
      const Index& index, const Range& range) {
    auto start = [&] {
      if(auto rangeStart =
          boost::get<boost::posix_time::ptime>(&range.GetStart())) {
        auto startRangeQuery = Query();
        startRangeQuery.SetIndex(index);
        startRangeQuery.SetRange(Range(*rangeStart, Sequence::Last()));
        startRangeQuery.SetSnapshotLimit(SnapshotLimit::Type::HEAD, 1);
        auto matches = m_dataStore->Load(startRangeQuery);
        if(matches.empty()) {
          return Sequence::Last();
        }
        return matches.front().GetSequence();
      }
      return boost::get<Sequence>(range.GetStart());
    }();
    auto end = [&] {
      if(auto rangeEnd =
          boost::get<boost::posix_time::ptime>(&range.GetEnd())) {
        auto endRangeQuery = Query();
        endRangeQuery.SetIndex(index);
        endRangeQuery.SetRange(Range(Sequence::First(), *rangeEnd));
        endRangeQuery.SetSnapshotLimit(SnapshotLimit::Type::TAIL, 1);
        auto matches = m_dataStore->Load(endRangeQuery);
        if(matches.empty()) {
          return Sequence::First();
        }
        return matches.front().GetSequence();
      }
      return boost::get<Sequence>(range.GetEnd());
    }();
    return Range(start, end);
  }

  template<typename D, typename T>
  boost::optional<typename CachedDataStoreEntry<D, T>::LocalDataStoreEntry&>
      CachedDataStoreEntry<D, T>::FindDataStore(Sequence sequence) {
    auto dataStore = m_dataStores.With(
      [&] (auto& dataStores) -> DataStoreEntry* {
        auto dataStoreIterator = std::lower_bound(dataStores.begin(),
          dataStores.end(), sequence, [] (const auto& lhs, auto rhs) {
            return lhs->m_sequence < rhs;
          });
        if(dataStoreIterator == dataStores.end() ||
            (*dataStoreIterator)->m_sequence != sequence) {
          return nullptr;
        }
        return dataStoreIterator->get();
      });
    if(dataStore) {
      return dataStore->m_dataStore;
    }
    return boost::none;
  }

  template<typename D, typename T>
  typename CachedDataStoreEntry<D, T>::LocalDataStoreEntry&
      CachedDataStoreEntry<D, T>::LoadDataStore(Sequence sequence) {
    auto dataStore = m_dataStores.With(
      [&] (auto& dataStores) -> DataStoreEntry* {
        auto dataStoreIterator = std::lower_bound(dataStores.begin(),
          dataStores.end(), sequence, [] (const auto& lhs, auto rhs) {
            return lhs->m_sequence < rhs;
          });
        if(dataStoreIterator == dataStores.end() ||
            (*dataStoreIterator)->m_sequence != sequence) {
          auto dataStoreEntry = std::make_unique<DataStoreEntry>(sequence);
          dataStoreIterator = dataStores.insert(dataStoreIterator,
            std::move(dataStoreEntry));
        }
        return dataStoreIterator->get();
      });
    dataStore->m_initializer.Call([&] {
      auto query = Query();
      query.SetIndex(m_index);
      query.SetRange(sequence,
        Sequence(sequence.GetOrdinal() + m_blockSize - 1));
      query.SetSnapshotLimit(SnapshotLimit::Unlimited());
      auto matches = m_dataStore->Load(query);
      dataStore->m_dataStore.Store(std::move(matches));
    });
    return dataStore->m_dataStore;
  }

  template<typename D, typename T>
  std::vector<typename CachedDataStoreEntry<D, T>::SequencedValue>
      CachedDataStoreEntry<D, T>::LoadHead(
        const Query& query, Sequence start, Sequence end) {
    auto matches = std::vector<SequencedValue>();
    auto subsetQuery = query;
    auto subsetStart = boost::get<Sequence>(query.GetRange().GetStart());
    auto remainingLimit = subsetQuery.GetSnapshotLimit().GetSize();
    for(auto ordinal = start.GetOrdinal(); ordinal <= end.GetOrdinal();
        ordinal += m_blockSize) {
      if(query.GetSnapshotLimit() != SnapshotLimit::Unlimited()) {
        subsetQuery.SetSnapshotLimit(SnapshotLimit::Type::HEAD, remainingLimit);
      }
      subsetQuery.SetRange(subsetStart, query.GetRange().GetEnd());
      if(auto blockDataStore = FindDataStore(Sequence(ordinal))) {
        auto subsetMatches = blockDataStore->Load(subsetQuery);
        remainingLimit -= static_cast<int>(subsetMatches.size());
        if(matches.empty()) {
          matches = std::move(subsetMatches);
        } else {
          for(auto& match : subsetMatches) {
            matches.push_back(std::move(match));
          }
        }
        if(remainingLimit <= 0 || ordinal == end.GetOrdinal()) {
          break;
        }
        subsetStart = Sequence(ordinal + m_blockSize);
      } else {
        auto subsetMatches = m_dataStore->Load(subsetQuery);
        LoadDataStore(Sequence(ordinal));
        if(matches.empty()) {
          matches = std::move(subsetMatches);
        } else {
          for(auto& match : subsetMatches) {
            matches.push_back(std::move(match));
          }
        }
        break;
      }
    }
    return matches;
  }

  template<typename D, typename T>
  std::vector<typename CachedDataStoreEntry<D, T>::SequencedValue>
      CachedDataStoreEntry<D, T>::LoadTail(
        const Query& query, Sequence start, Sequence end) {
    auto partitions = std::vector<std::vector<SequencedValue>>();
    auto subsetQuery = query;
    auto subsetEnd = boost::get<Sequence>(query.GetRange().GetEnd());
    auto remainingLimit = subsetQuery.GetSnapshotLimit().GetSize();
    for(auto ordinal = end.GetOrdinal(); ordinal >= start.GetOrdinal();
        ordinal -= m_blockSize) {
      if(query.GetSnapshotLimit() != SnapshotLimit::Unlimited()) {
        subsetQuery.SetSnapshotLimit(SnapshotLimit::Type::TAIL,
          remainingLimit);
      }
      subsetQuery.SetRange(query.GetRange().GetStart(), subsetEnd);
      auto blockDataStore = FindDataStore(Sequence(ordinal));
      if(blockDataStore.is_initialized()) {
        partitions.push_back(blockDataStore->Load(subsetQuery));
        remainingLimit -= static_cast<int>(partitions.back().size());
        if(remainingLimit <= 0 || ordinal == start.GetOrdinal()) {
          break;
        }
        subsetEnd = Decrement(Sequence(ordinal));
      } else {
        partitions.push_back(m_dataStore->Load(subsetQuery));
        LoadDataStore(Sequence(ordinal));
        break;
      }
    }
    if(partitions.empty()) {
      return {};
    } else if(partitions.size() == 1) {
      return std::move(partitions.front());
    }
    auto matches = std::vector<SequencedValue>();
    for(auto& partition : boost::adaptors::reverse(partitions)) {
      for(auto& match : partition) {
        matches.push_back(std::move(match));
      }
    }
    return matches;
  }
}

#endif
