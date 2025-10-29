#ifndef BEAM_BUFFERED_DATA_STORE_HPP
#define BEAM_BUFFERED_DATA_STORE_HPP
#include <algorithm>
#include <memory>
#include <vector>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Queries/LocalDataStore.hpp"
#include "Beam/Queries/Range.hpp"
#include "Beam/Queues/RoutineTaskQueue.hpp"
#include "Beam/Utilities/Algorithm.hpp"

namespace Beam {

  /**
   * Buffers writes to a data store.
   * @tparam D The type of data store to buffer writes to.
   * @tparam E The type of EvaluatorTranslator used for filtering values.
   */
  template<typename D,
    typename E = typename dereference_t<D>::EvaluatorTranslatorFilter>
  class BufferedDataStore {
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
      using EvaluatorTranslatorFilter = E;

      /**
       * Constructs a BufferedDataStore.
       * @param data_size Initializes the data store to buffer data to.
       * @param buffer_size The number of messages to buffer before committing
       *        to the <i>data_size</i>.
       */
      template<Initializes<D> DS>
      BufferedDataStore(DS&& data_size, std::size_t buffer_size);

      ~BufferedDataStore();

      std::vector<SequencedValue> load(const Query& query);
      void store(const IndexedValue& value);
      void store(const std::vector<IndexedValue>& values);
      void close();

    private:
      using ReserveDataStore =
        LocalDataStore<Query, Value, EvaluatorTranslatorFilter>;
      mutable boost::mutex m_mutex;
      local_ptr_t<D> m_data_store;
      std::size_t m_buffer_size;
      std::size_t m_buffer_count;
      std::shared_ptr<ReserveDataStore> m_data_store_buffer;
      std::shared_ptr<ReserveDataStore> m_flushed_data_store;
      OpenState m_open_state;
      RoutineTaskQueue m_tasks;

      BufferedDataStore(const BufferedDataStore&) = delete;
      BufferedDataStore& operator =(const BufferedDataStore&) = delete;
      void flush();
      void test_flush();
  };

  template<typename D>
  BufferedDataStore(D&&, std::size_t) -> BufferedDataStore<
    local_ptr_t<D>, typename dereference_t<D>::EvaluatorTranslatorFilter>;

  template<typename D, typename E>
  template<Initializes<D> DS>
  BufferedDataStore<D, E>::BufferedDataStore(
    DS&& data_size, std::size_t buffer_size)
    : m_data_store(std::forward<DS>(data_size)),
      m_buffer_size(buffer_size),
      m_buffer_count(0),
      m_data_store_buffer(std::make_shared<ReserveDataStore>()),
      m_flushed_data_store(m_data_store_buffer) {}

  template<typename D, typename E>
  BufferedDataStore<D, E>::~BufferedDataStore() {
    close();
  }

  template<typename D, typename E>
  std::vector<typename BufferedDataStore<D, E>::SequencedValue>
      BufferedDataStore<D, E>::load(const Query& query) {
    auto buffer = std::shared_ptr<ReserveDataStore>();
    {
      auto lock = boost::lock_guard(m_mutex);
      buffer = m_flushed_data_store;
    }
    auto matches = std::vector<SequencedValue>();
    if(query.get_snapshot_limit().get_type() == SnapshotLimit::Type::HEAD) {
      matches = m_data_store->load(query);
    } else {
      matches = buffer->load(query);
    }
    if(static_cast<int>(matches.size()) <
        query.get_snapshot_limit().get_size()) {
      auto additional_matches = std::vector<SequencedValue>();
      if(query.get_snapshot_limit().get_type() == SnapshotLimit::Type::HEAD) {
        additional_matches = buffer->load(query);
      } else {
        additional_matches = m_data_store->load(query);
      }
      auto merged_matches = std::vector<SequencedValue>();
      std::ranges::set_union(additional_matches, matches,
        std::back_inserter(merged_matches), SequenceComparator());
      if(static_cast<int>(merged_matches.size()) >
          query.get_snapshot_limit().get_size()) {
        if(query.get_snapshot_limit().get_type() == SnapshotLimit::Type::HEAD) {
          merged_matches.erase(merged_matches.begin() +
            query.get_snapshot_limit().get_size(), merged_matches.end());
        } else {
          merged_matches.erase(merged_matches.begin(), merged_matches.begin() +
            (merged_matches.size() - query.get_snapshot_limit().get_size()));
        }
      }
      matches = std::move(merged_matches);
    }
    return matches;
  }

  template<typename D, typename E>
  void BufferedDataStore<D, E>::store(const IndexedValue& value) {
    auto lock = boost::lock_guard(m_mutex);
    ++m_buffer_count;
    m_data_store_buffer->store(value);
    test_flush();
  }

  template<typename D, typename E>
  void BufferedDataStore<D, E>::store(const std::vector<IndexedValue>& values) {
    auto lock = boost::lock_guard(m_mutex);
    m_buffer_count += values.size();
    m_data_store_buffer->store(values);
    test_flush();
  }

  template<typename D, typename E>
  void BufferedDataStore<D, E>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_tasks.push([&] {
      flush();
    });
    m_tasks.close();
    m_tasks.wait();
    m_open_state.close();
  }

  template<typename D, typename E>
  void BufferedDataStore<D, E>::test_flush() {
    if(m_buffer_count < m_buffer_size) {
      return;
    }
    m_buffer_count = 0;
    m_tasks.push([this] {
      flush();
    });
  }

  template<typename D, typename E>
  void BufferedDataStore<D, E>::flush() {
    auto data_size = std::make_shared<ReserveDataStore>();
    {
      auto lock = boost::lock_guard(m_mutex);
      data_size.swap(m_data_store_buffer);
    }
    m_data_store->store(data_size->load_all());
    {
      auto lock = boost::lock_guard(m_mutex);
      m_flushed_data_store = m_data_store_buffer;
    }
  }
}

#endif
