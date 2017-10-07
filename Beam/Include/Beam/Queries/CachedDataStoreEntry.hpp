#ifndef BEAM_CACHEDDATASTOREENTRY_HPP
#define BEAM_CACHEDDATASTOREENTRY_HPP
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queries/LocalDataStoreEntry.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/Sequence.hpp"
#include "Beam/Threading/CallOnce.hpp"
#include "Beam/Threading/Mutex.hpp"
#include "Beam/Utilities/DefaultValue.hpp"
#include "Beam/Utilities/SynchronizedList.hpp"

namespace Beam {
namespace Queries {

  /*! \class CachedDataStoreEntry
      \brief Stores cached data for a single index.
      \tparam DataStoreType The type of data store to buffer writes to.
      \tparam EvaluatorTranslatorFilterType The type of EvaluatorTranslator
              used for filtering values.
   */
  template<typename DataStoreType, typename EvaluatorTranslatorFilterType =
    typename GetTryDereferenceType<DataStoreType>::EvaluatorTranslatorFilter>
  class CachedDataStoreEntry {
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

      //! Constructs a CachedDataStoreEntry.
      /*!
        \param dataStore Initializes the data store to cache.
        \param index The Index to cache.
        \param blockSize The size of a single cache block.
      */
      template<typename DataStoreForward>
      CachedDataStoreEntry(DataStoreForward&& dataStore, const Index& index,
        int blockSize);

      std::vector<SequencedValue> Load(const Query& query);

      void Store(const IndexedValue& value);

    private:
      using LocalDataStoreEntry = ::Beam::Queries::LocalDataStoreEntry<Query,
        Value, EvaluatorTranslatorFilterType>;
      struct DataStoreEntry {
        Sequence m_sequence;
        LocalDataStoreEntry m_dataStore;
        Threading::CallOnce<Threading::Mutex> m_initializer;

        DataStoreEntry(Sequence sequence);
      };
      GetOptionalLocalPtr<DataStoreType> m_dataStore;
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

  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
  CachedDataStoreEntry<DataStoreType, EvaluatorTranslatorFilterType>::
      DataStoreEntry::DataStoreEntry(Sequence sequence)
      : m_sequence(sequence) {}

  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
  template<typename DataStoreForward>
  CachedDataStoreEntry<DataStoreType, EvaluatorTranslatorFilterType>::
      CachedDataStoreEntry(DataStoreForward&& dataStore, const Index& index,
      int blockSize)
      : m_dataStore(std::forward<DataStoreForward>(dataStore)),
        m_index(index),
        m_blockSize(blockSize) {}

  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
  std::vector<typename CachedDataStoreEntry<DataStoreType,
      EvaluatorTranslatorFilterType>::SequencedValue>
      CachedDataStoreEntry<DataStoreType, EvaluatorTranslatorFilterType>::Load(
      const Query& query) {
    auto sequencedQuery = query;
    sequencedQuery.SetRange(ToSequence(query.GetIndex(), query.GetRange()));
    auto start = Normalize(boost::get<Sequence>(
      sequencedQuery.GetRange().GetStart()));
    auto end = Normalize(boost::get<Sequence>(
      sequencedQuery.GetRange().GetEnd()));
    if(sequencedQuery.GetSnapshotLimit().GetType() ==
        SnapshotLimit::Type::HEAD) {
      return LoadHead(sequencedQuery, start, end);
    } else {
      return LoadTail(sequencedQuery, start, end);
    }
  }

  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
  void CachedDataStoreEntry<DataStoreType, EvaluatorTranslatorFilterType>::
      Store(const IndexedValue& value) {
    auto& cachedDataStore = LoadDataStore(Normalize(value.GetSequence()));
    cachedDataStore.Store(value);
  }

  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
  Sequence CachedDataStoreEntry<DataStoreType, EvaluatorTranslatorFilterType>::
      Normalize(Sequence sequence) const {
    return Sequence(
      sequence.GetOrdinal() - (sequence.GetOrdinal() % m_blockSize));
  }

  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
  Range CachedDataStoreEntry<DataStoreType, EvaluatorTranslatorFilterType>::
      ToSequence(const Index& index, const Range& range) {
    Sequence start;
    if(auto rangeStart =
        boost::get<boost::posix_time::ptime>(&range.GetStart())) {
      Query startRangeQuery;
      startRangeQuery.SetIndex(index);
      startRangeQuery.SetRange(Range(*rangeStart, Sequence::Last()));
      startRangeQuery.SetSnapshotLimit(SnapshotLimit::Type::HEAD, 1);
      auto matches = m_dataStore->Load(startRangeQuery);
      if(matches.empty()) {
        start = Sequence::Last();
      } else {
        start = matches.front().GetSequence();
      }
    } else {
      start = boost::get<Sequence>(range.GetStart());
    }
    Sequence end;
    if(auto rangeEnd = boost::get<boost::posix_time::ptime>(&range.GetEnd())) {
      Query endRangeQuery;
      endRangeQuery.SetIndex(index);
      endRangeQuery.SetRange(Range(Sequence::First(), *rangeEnd));
      endRangeQuery.SetSnapshotLimit(SnapshotLimit::Type::TAIL, 1);
      auto matches = m_dataStore->Load(endRangeQuery);
      if(matches.empty()) {
        end = Sequence::First();
      } else {
        end = matches.front().GetSequence();
      }
    } else {
      end = boost::get<Sequence>(range.GetEnd());
    }
    return Range(start, end);
  }

  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
  boost::optional<typename CachedDataStoreEntry<
      DataStoreType, EvaluatorTranslatorFilterType>::LocalDataStoreEntry&>
      CachedDataStoreEntry<DataStoreType, EvaluatorTranslatorFilterType>::
      FindDataStore(Sequence sequence) {
    auto dataStore = m_dataStores.With(
      [&] (std::vector<std::unique_ptr<DataStoreEntry>>& dataStores) ->
          DataStoreEntry* {
        auto dataStoreIterator = std::lower_bound(dataStores.begin(),
          dataStores.end(), sequence,
          [] (const std::unique_ptr<DataStoreEntry>& lhs, Sequence rhs) {
            return lhs->m_sequence < rhs;
          });
        if(dataStoreIterator == dataStores.end() ||
            (*dataStoreIterator)->m_sequence != sequence) {
          return nullptr;
        }
        return dataStoreIterator->get();
      });
    if(dataStore == nullptr) {
      return boost::none;
    }
    return dataStore->m_dataStore;
  }

  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
  typename CachedDataStoreEntry<DataStoreType, EvaluatorTranslatorFilterType>::
      LocalDataStoreEntry& CachedDataStoreEntry<DataStoreType,
      EvaluatorTranslatorFilterType>::LoadDataStore(Sequence sequence) {
    auto dataStore = m_dataStores.With(
      [&] (std::vector<std::unique_ptr<DataStoreEntry>>& dataStores) ->
          DataStoreEntry* {
        auto dataStoreIterator = std::lower_bound(dataStores.begin(),
          dataStores.end(), sequence,
          [] (const std::unique_ptr<DataStoreEntry>& lhs, Sequence rhs) {
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
    dataStore->m_initializer.Call(
      [&] {
        Query query;
        query.SetIndex(m_index);
        query.SetRange(sequence,
          Sequence(sequence.GetOrdinal() + m_blockSize - 1));
        query.SetSnapshotLimit(SnapshotLimit::Unlimited());
        auto matches = m_dataStore->Load(query);
        dataStore->m_dataStore.Store(std::move(matches));
      });
    return dataStore->m_dataStore;
  }

  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
  std::vector<typename CachedDataStoreEntry<DataStoreType,
      EvaluatorTranslatorFilterType>::SequencedValue>
      CachedDataStoreEntry<DataStoreType, EvaluatorTranslatorFilterType>::
      LoadHead(const Query& query, Sequence start, Sequence end) {
    std::vector<SequencedValue> matches;
    auto subsetQuery = query;
    auto subsetStart = boost::get<Sequence>(query.GetRange().GetStart());
    auto remainingLimit = subsetQuery.GetSnapshotLimit().GetSize();
    for(auto ordinal = start.GetOrdinal(); ordinal <= end.GetOrdinal();
        ordinal += m_blockSize) {
      if(query.GetSnapshotLimit() != SnapshotLimit::Unlimited()) {
        subsetQuery.SetSnapshotLimit(SnapshotLimit::Type::HEAD,
          remainingLimit);
      }
      subsetQuery.SetRange(subsetStart, query.GetRange().GetEnd());
      auto blockDataStore = FindDataStore(Sequence(ordinal));
      if(blockDataStore.is_initialized()) {
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

  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
  std::vector<typename CachedDataStoreEntry<DataStoreType,
      EvaluatorTranslatorFilterType>::SequencedValue>
      CachedDataStoreEntry<DataStoreType, EvaluatorTranslatorFilterType>::
      LoadTail(const Query& query, Sequence start, Sequence end) {
    std::vector<std::vector<SequencedValue>> partitions;
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
      return Default();
    } else if(partitions.size() == 1) {
      return std::move(partitions.front());
    }
    std::vector<SequencedValue> matches;
    for(auto& partition : boost::adaptors::reverse(partitions)) {
      for(auto& match : partition) {
        matches.push_back(std::move(match));
      }
    }
    return matches;
  }
}
}

#endif
