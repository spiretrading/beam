#ifndef BEAM_CACHED_DATA_STORE_ENTRY_HPP
#define BEAM_CACHED_DATA_STORE_ENTRY_HPP
#include "Beam/Collections/SynchronizedList.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queries/LocalDataStoreEntry.hpp"
#include "Beam/Queries/Sequence.hpp"
#include "Beam/Threading/CallOnce.hpp"
#include "Beam/Threading/Mutex.hpp"

namespace Beam {

  /**
   * Stores cached data for a single index.
   * @tparam D The type of data store to buffer writes to.
   * @tparam T The type of EvaluatorTranslator used for filtering values.
   */
  template<typename D,
    typename T = typename dereference_t<D>::EvaluatorTranslatorFilter>
  class CachedDataStoreEntry {
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
      using EvaluatorTranslatorFilter = T;

      /**
       * Constructs a CachedDataStoreEntry.
       * @param data_store Initializes the data store to cache.
       * @param index The Index to cache.
       * @param block_size The size of a single cache block.
       */
      template<Initializes<D> DF>
      CachedDataStoreEntry(DF&& data_store, const Index& index, int block_size);

      std::vector<SequencedValue> load(const Query& query);
      void store(const IndexedValue& value);

    private:
      using LocalDataStoreEntry = Beam::LocalDataStoreEntry<Query, Value, T>;
      struct DataStoreEntry {
        Sequence m_sequence;
        LocalDataStoreEntry m_data_store;
        CallOnce<Mutex> m_initializer;

        DataStoreEntry(Sequence sequence);
      };
      local_ptr_t<D> m_data_store;
      Index m_index;
      int m_block_size;
      SynchronizedVector<std::unique_ptr<DataStoreEntry>> m_data_stores;

      Sequence normalize(Sequence sequence) const;
      Range to_sequence(const Index& index, const Range& range);
      boost::optional<LocalDataStoreEntry&> find_data_store(Sequence sequence);
      LocalDataStoreEntry& load_data_store(Sequence sequence);
      std::vector<SequencedValue> load_head(
        const Query& query, Sequence start, Sequence end);
      std::vector<SequencedValue> load_tail(
        const Query& query, Sequence start, Sequence end);
  };

  template<typename D, typename T>
  CachedDataStoreEntry<D, T>::DataStoreEntry::DataStoreEntry(Sequence sequence)
    : m_sequence(sequence) {}

  template<typename D, typename T>
  template<Initializes<D> DF>
  CachedDataStoreEntry<D, T>::CachedDataStoreEntry(
    DF&& data_store, const Index& index, int block_size)
    : m_data_store(std::forward<DF>(data_store)),
      m_index(index),
      m_block_size(block_size) {}

  template<typename D, typename T>
  std::vector<typename CachedDataStoreEntry<D, T>::SequencedValue>
      CachedDataStoreEntry<D, T>::load(const Query& query) {
    auto sequenced_query = query;
    sequenced_query.set_range(
      to_sequence(query.get_index(), query.get_range()));
    auto start =
      normalize(boost::get<Sequence>(sequenced_query.get_range().get_start()));
    auto end =
      normalize(boost::get<Sequence>(sequenced_query.get_range().get_end()));
    if(sequenced_query.get_snapshot_limit().get_type() ==
        SnapshotLimit::Type::HEAD) {
      return load_head(sequenced_query, start, end);
    } else {
      return load_tail(sequenced_query, start, end);
    }
  }

  template<typename D, typename T>
  void CachedDataStoreEntry<D, T>::store(const IndexedValue& value) {
    auto& cached_data_store = load_data_store(normalize(value.get_sequence()));
    cached_data_store.store(value);
  }

  template<typename D, typename T>
  Sequence CachedDataStoreEntry<D, T>::normalize(Sequence sequence) const {
    return Sequence(
      sequence.get_ordinal() - (sequence.get_ordinal() % m_block_size));
  }

  template<typename D, typename T>
  Range CachedDataStoreEntry<D, T>::to_sequence(
      const Index& index, const Range& range) {
    auto start = [&] {
      if(auto start =
          boost::get<boost::posix_time::ptime>(&range.get_start())) {
        auto start_query = Query();
        start_query.set_index(index);
        start_query.set_range(Range(*start, Sequence::LAST));
        start_query.set_snapshot_limit(SnapshotLimit::Type::HEAD, 1);
        auto matches = m_data_store->load(start_query);
        if(matches.empty()) {
          return Sequence::LAST;
        }
        return matches.front().get_sequence();
      }
      return boost::get<Sequence>(range.get_start());
    }();
    auto end = [&] {
      if(auto end = boost::get<boost::posix_time::ptime>(&range.get_end())) {
        auto end_query = Query();
        end_query.set_index(index);
        end_query.set_range(Range(Sequence::FIRST, *end));
        end_query.set_snapshot_limit(SnapshotLimit::Type::TAIL, 1);
        auto matches = m_data_store->load(end_query);
        if(matches.empty()) {
          return Sequence::FIRST;
        }
        return matches.front().get_sequence();
      }
      return boost::get<Sequence>(range.get_end());
    }();
    return Range(start, end);
  }

  template<typename D, typename T>
  boost::optional<typename CachedDataStoreEntry<D, T>::LocalDataStoreEntry&>
      CachedDataStoreEntry<D, T>::find_data_store(Sequence sequence) {
    auto data_store = m_data_stores.with(
      [&] (auto& data_stores) -> DataStoreEntry* {
        auto i = std::lower_bound(data_stores.begin(), data_stores.end(),
          sequence, [] (const auto& lhs, auto rhs) {
            return lhs->m_sequence < rhs;
          });
        if(i == data_stores.end() || (*i)->m_sequence != sequence) {
          return nullptr;
        }
        return i->get();
      });
    if(data_store) {
      return data_store->m_data_store;
    }
    return boost::none;
  }

  template<typename D, typename T>
  typename CachedDataStoreEntry<D, T>::LocalDataStoreEntry&
      CachedDataStoreEntry<D, T>::load_data_store(Sequence sequence) {
    auto data_store = m_data_stores.with(
      [&] (auto& data_stores) -> DataStoreEntry* {
        auto i = std::lower_bound(data_stores.begin(), data_stores.end(),
          sequence, [] (const auto& lhs, auto rhs) {
            return lhs->m_sequence < rhs;
          });
        if(i == data_stores.end() || (*i)->m_sequence != sequence) {
          auto entry = std::make_unique<DataStoreEntry>(sequence);
          i = data_stores.insert(i, std::move(entry));
        }
        return i->get();
      });
    data_store->m_initializer.call([&] {
      auto query = Query();
      query.set_index(m_index);
      query.set_range(
        sequence, Sequence(sequence.get_ordinal() + m_block_size - 1));
      query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
      auto matches = m_data_store->load(query);
      data_store->m_data_store.store(std::move(matches));
    });
    return data_store->m_data_store;
  }

  template<typename D, typename T>
  std::vector<typename CachedDataStoreEntry<D, T>::SequencedValue>
      CachedDataStoreEntry<D, T>::load_head(
        const Query& query, Sequence start, Sequence end) {
    auto matches = std::vector<SequencedValue>();
    auto subset_query = query;
    auto subset_start = boost::get<Sequence>(query.get_range().get_start());
    auto remaining_limit = subset_query.get_snapshot_limit().get_size();
    for(auto ordinal = start.get_ordinal(); ordinal <= end.get_ordinal();
        ordinal += m_block_size) {
      if(query.get_snapshot_limit() != SnapshotLimit::UNLIMITED) {
        subset_query.set_snapshot_limit(
          SnapshotLimit::Type::HEAD, remaining_limit);
      }
      subset_query.set_range(subset_start, query.get_range().get_end());
      if(auto block_data_store = find_data_store(Sequence(ordinal))) {
        auto subset_matches = block_data_store->load(subset_query);
        remaining_limit -= static_cast<int>(subset_matches.size());
        if(matches.empty()) {
          matches = std::move(subset_matches);
        } else {
          for(auto& match : subset_matches) {
            matches.push_back(std::move(match));
          }
        }
        if(remaining_limit <= 0 || ordinal == end.get_ordinal()) {
          break;
        }
        subset_start = Sequence(ordinal + m_block_size);
      } else {
        auto subset_matches = m_data_store->load(subset_query);
        load_data_store(Sequence(ordinal));
        if(matches.empty()) {
          matches = std::move(subset_matches);
        } else {
          for(auto& match : subset_matches) {
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
      CachedDataStoreEntry<D, T>::load_tail(
        const Query& query, Sequence start, Sequence end) {
    auto partitions = std::vector<std::vector<SequencedValue>>();
    auto subset_query = query;
    auto subset_end = boost::get<Sequence>(query.get_range().get_end());
    auto remaining_limit = subset_query.get_snapshot_limit().get_size();
    for(auto ordinal = end.get_ordinal(); ordinal >= start.get_ordinal();
        ordinal -= m_block_size) {
      if(query.get_snapshot_limit() != SnapshotLimit::UNLIMITED) {
        subset_query.set_snapshot_limit(
          SnapshotLimit::Type::TAIL, remaining_limit);
      }
      subset_query.set_range(query.get_range().get_start(), subset_end);
      auto block_data_store = find_data_store(Sequence(ordinal));
      if(block_data_store) {
        partitions.push_back(block_data_store->load(subset_query));
        remaining_limit -= static_cast<int>(partitions.back().size());
        if(remaining_limit <= 0 || ordinal == start.get_ordinal()) {
          break;
        }
        subset_end = decrement(Sequence(ordinal));
      } else {
        partitions.push_back(m_data_store->load(subset_query));
        load_data_store(Sequence(ordinal));
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
