#ifndef BEAM_SESSION_CACHED_DATA_STORE_ENTRY_HPP
#define BEAM_SESSION_CACHED_DATA_STORE_ENTRY_HPP
#include <atomic>
#include <boost/thread/mutex.hpp>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queries/LocalDataStoreEntry.hpp"
#include "Beam/Queries/Sequence.hpp"
#include "Beam/Threading/CallOnce.hpp"
#include "Beam/Threading/Mutex.hpp"

namespace Beam {

  /**
   * Caches the most recent writes made to a data store for a single index.
   * @tparam D The type of data store to buffer writes to.
   * @tparam T The type of EvaluatorTranslator used for filtering values.
   */
  template<typename D,
    typename T = typename dereference_t<D>::EvaluatorTranslatorFilter>
  class SessionCachedDataStoreEntry {
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
       * Constructs a SessionCachedDataStoreEntry.
       * @param data_store Initializes the data store to cache.
       * @param block_size The size of a single cache block.
       */
      template<Initializes<D> DF>
      SessionCachedDataStoreEntry(DF&& data_store, int block_size);

      std::vector<SequencedValue> load(const Query& query);
      void store(const IndexedValue& value);

    private:
      using LocalDataStoreEntry = Beam::LocalDataStoreEntry<Query, Value, T>;
      struct DataStoreEntry {
        LocalDataStoreEntry m_data_store;
        boost::posix_time::ptime m_timestamp;
        Sequence m_sequence;
        std::atomic_int m_size;

        DataStoreEntry(boost::posix_time::ptime timestamp, Sequence sequence);
      };
      mutable boost::mutex m_mutex;
      local_ptr_t<D> m_data_store;
      int m_block_size;
      CallOnce<Mutex> m_initializer;
      std::shared_ptr<DataStoreEntry> m_cache;

      std::shared_ptr<DataStoreEntry> initialize_cache(const Index& index);
  };

  template<typename D, typename T>
  SessionCachedDataStoreEntry<D, T>::DataStoreEntry::DataStoreEntry(
    boost::posix_time::ptime timestamp, Sequence sequence)
    : m_timestamp(timestamp),
      m_sequence(sequence),
      m_size(0) {}

  template<typename D, typename T>
  template<Initializes<D> DF>
  SessionCachedDataStoreEntry<D, T>::SessionCachedDataStoreEntry(
    DF&& data_store, int block_size)
    : m_data_store(std::forward<DF>(data_store)),
      m_block_size(block_size) {}

  template<typename D, typename T>
  std::vector<typename SessionCachedDataStoreEntry<D, T>::SequencedValue>
      SessionCachedDataStoreEntry<D, T>::load(const Query& query) {
    if(m_block_size == 0) {
      return m_data_store->load(query);
    }
    auto cache = initialize_cache(query.get_index());
    if(auto start =
        boost::get<boost::posix_time::ptime>(&query.get_range().get_start())) {
      if(*start > cache->m_timestamp) {
        return cache->m_data_store.load(query);
      }
    } else if(auto start =
        boost::get<Sequence>(&query.get_range().get_start())) {
      if(*start > cache->m_sequence) {
        return cache->m_data_store.load(query);
      }
    }
    if(query.get_snapshot_limit().get_type() == SnapshotLimit::Type::TAIL) {
      auto size = cache->m_size.load();
      if(query.get_snapshot_limit().get_size() <= size) {
        auto end = boost::get<Sequence>(&query.get_range().get_end());
        auto end_timestamp =
          boost::get<boost::posix_time::ptime>(&query.get_range().get_end());
        if(end && *end > cache->m_sequence ||
            end_timestamp && *end_timestamp > cache->m_timestamp) {
          auto result = cache->m_data_store.load(query);
          if(result.size() >= static_cast<std::size_t>(
              query.get_snapshot_limit().get_size())) {
            return result;
          }
        }
      }
    }
    return m_data_store->load(query);
  }

  template<typename D, typename T>
  void SessionCachedDataStoreEntry<D, T>::store(const IndexedValue& value) {
    if(m_block_size == 0) {
      return;
    }
    auto cache = initialize_cache(value->get_index());
    auto size = cache->m_size.load();
    if(size > 2 * m_block_size) {
      auto lock = boost::lock_guard(m_mutex);
      auto data = cache->m_data_store.load_all();
      auto reference_value = data[m_block_size - 1];
      data.erase(data.begin(), data.begin() + m_block_size);
      m_cache = std::make_shared<DataStoreEntry>(
        get_timestamp(*reference_value), reference_value.get_sequence());
      m_cache->m_data_store.store(data);
      m_cache->m_size = data.size();
      cache = m_cache;
    }
    cache->m_data_store.store(value);
    ++cache->m_size;
  }

  template<typename D, typename T>
  std::shared_ptr<typename SessionCachedDataStoreEntry<D, T>::DataStoreEntry>
      SessionCachedDataStoreEntry<D, T>::initialize_cache(const Index& index) {
    m_initializer.call([&] {
      auto query = Query();
      query.set_index(index);
      query.set_range(Range::TOTAL);
      query.set_snapshot_limit(SnapshotLimit::Type::TAIL, 1);
      auto data = m_data_store->load(query);
      if(data.empty()) {
        m_cache = std::make_shared<DataStoreEntry>(
          boost::posix_time::neg_infin, Sequence::FIRST);
      } else {
        m_cache = std::make_shared<DataStoreEntry>(
          get_timestamp(*data.back()), data.back().get_sequence());
      }
    });
    auto lock = boost::lock_guard(m_mutex);
    return m_cache;
  }
}

#endif
