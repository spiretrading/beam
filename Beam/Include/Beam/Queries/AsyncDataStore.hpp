#ifndef BEAM_ASYNC_DATA_STORE_HPP
#define BEAM_ASYNC_DATA_STORE_HPP
#include <array>
#include <iostream>
#include <memory>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queries/LocalDataStore.hpp"
#include "Beam/Queues/RoutineTaskQueue.hpp"
#include "Beam/Utilities/ReportException.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {
namespace Details {
  template<SnapshotLimit::Type D, typename C>
  struct Matches {
    static constexpr auto Direction = D;
    using Container = C;
    using Type = typename Container::value_type;
    using Iterator = std::conditional_t<
      D == SnapshotLimit::Type::HEAD, Type*, std::reverse_iterator<Type*>>;

    Matches() = default;

    Matches(Container& container) {
      if constexpr(Direction == SnapshotLimit::Type::HEAD) {
        m_current = container.data();
        m_end = container.data() + container.size();
      } else {
        m_current = Iterator(container.data() + container.size());
        m_end = Iterator(container.data());
      }
    }

    Iterator m_current;
    Iterator m_end;
  };

  template<SnapshotLimit::Type D, typename C>
  C merge(C match1, C match2, C match3, int limit) {
    const auto MATCH_COUNT = 3;
    auto matches = std::array<Matches<D, C>, MATCH_COUNT>();
    auto matches_remaining = 0;
    for(auto& match : {&match1, &match2, &match3}) {
      if(!match->empty()) {
        matches[matches_remaining] = Matches<D, C>(*match);
        ++matches_remaining;
      }
    }
    auto result = C();
    result.reserve(std::min(static_cast<std::size_t>(limit),
      match1.size() + match2.size() + match3.size()));
    auto comparator = [] (const auto& lhs, const auto& rhs) {
      if(D == SnapshotLimit::Type::HEAD) {
        return SequenceComparator()(*lhs->m_current, *rhs->m_current);
      } else {
        return SequenceComparator()(*rhs->m_current, *lhs->m_current);
      }
    };
    while(result.size() != limit && matches_remaining != 0) {
      auto& match = *([&] {
        if(matches_remaining == 1) {
          return &matches[0];
        } else if(matches_remaining == 2) {
          return std::min(&matches[0], &matches[1], comparator);
        } else {
          return std::min({&matches[0], &matches[1], &matches[2]}, comparator);
        }
      }());
      auto& value = *match.m_current;
      ++match.m_current;
      if(match.m_current == match.m_end) {
        std::swap(match, matches[matches_remaining - 1]);
        --matches_remaining;
      }
      if(result.empty() ||
          value.get_sequence() != result.back().get_sequence()) {
        result.push_back(std::move(value));
      }
    }
    if(D == SnapshotLimit::Type::TAIL) {
      std::reverse(result.begin(), result.end());
    }
    return result;
  }
}

  /**
   * Asynchronously writes to a data store.
   * @tparam D The type of data store to buffer writes to.
   * @tparam E The type of EvaluatorTranslator used for filtering values.
   */
  template<typename D,
    typename E = typename dereference_t<D>::EvaluatorTranslatorFilter>
  class AsyncDataStore {
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
       * Constructs an AsyncDataStore.
       * @param data_store Initializes the data store to buffer data to.
       */
      template<Initializes<D> DS>
      explicit AsyncDataStore(DS&& data_store);

      ~AsyncDataStore();

      std::vector<SequencedValue> load(const Query& query);
      void store(const IndexedValue& value);
      void store(const std::vector<IndexedValue>& values);
      void close();

    private:
      using ReserveDataStore =
        LocalDataStore<Query, Value, EvaluatorTranslatorFilter>;
      mutable boost::mutex m_mutex;
      local_ptr_t<D> m_data_store;
      std::shared_ptr<ReserveDataStore> m_current_data_store;
      std::shared_ptr<ReserveDataStore> m_flushed_data_store;
      bool m_is_flushing;
      OpenState m_open_state;
      RoutineTaskQueue m_tasks;

      AsyncDataStore(const AsyncDataStore&) = delete;
      AsyncDataStore& operator =(const AsyncDataStore&) = delete;
      void test_flush();
      void flush();
  };

  template<typename DS>
  AsyncDataStore(DS&& data_store) -> AsyncDataStore<std::remove_cvref_t<DS>>;

  template<typename D, typename E>
  template<Initializes<D> DS>
  AsyncDataStore<D, E>::AsyncDataStore(DS&& data_store)
    : m_data_store(std::forward<DS>(data_store)),
      m_current_data_store(std::make_shared<ReserveDataStore>()),
      m_flushed_data_store(std::make_shared<ReserveDataStore>()),
      m_is_flushing(false) {}

  template<typename D, typename E>
  AsyncDataStore<D, E>::~AsyncDataStore() {
    close();
  }

  template<typename D, typename E>
  std::vector<typename AsyncDataStore<D, E>::SequencedValue>
      AsyncDataStore<D, E>::load(const Query& query) {
    auto [current_data_store, flushed_data_store] = [&] {
      auto lock = boost::lock_guard(m_mutex);
      return std::tuple(m_current_data_store, m_flushed_data_store);
    }();
    if(query.get_snapshot_limit().get_type() == SnapshotLimit::Type::HEAD) {
      return Details::merge<SnapshotLimit::Type::HEAD>(
        current_data_store->load(query), flushed_data_store->load(query),
        m_data_store->load(query), query.get_snapshot_limit().get_size());
    } else {
      return Details::merge<SnapshotLimit::Type::TAIL>(
        current_data_store->load(query), flushed_data_store->load(query),
        m_data_store->load(query), query.get_snapshot_limit().get_size());
    }
  }

  template<typename D, typename E>
  void AsyncDataStore<D, E>::store(const IndexedValue& value) {
    auto lock = boost::lock_guard(m_mutex);
    m_current_data_store->store(value);
    test_flush();
  }

  template<typename D, typename E>
  void AsyncDataStore<D, E>::store(const std::vector<IndexedValue>& values) {
    auto lock = boost::lock_guard(m_mutex);
    m_current_data_store->store(values);
    test_flush();
  }

  template<typename D, typename E>
  void AsyncDataStore<D, E>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_tasks.close();
    m_tasks.wait();
    m_open_state.close();
  }

  template<typename D, typename E>
  void AsyncDataStore<D, E>::test_flush() {
    if(!m_is_flushing) {
      m_is_flushing = true;
      m_tasks.push([this] {
        flush();
      });
    }
  }

  template<typename D, typename E>
  void AsyncDataStore<D, E>::flush() {
    {
      auto lock = boost::lock_guard(m_mutex);
      m_flushed_data_store.swap(m_current_data_store);
      m_is_flushing = false;
    }
    try {
      m_data_store->store(m_flushed_data_store->load_all());
    } catch(const std::exception&) {
      std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
    }
    auto new_data_store = std::make_shared<ReserveDataStore>();
    {
      auto lock = boost::lock_guard(m_mutex);
      m_flushed_data_store = std::move(new_data_store);
    }
  }
}

#endif
