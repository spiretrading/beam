#ifndef BEAM_ASYNC_DATA_STORE_HPP
#define BEAM_ASYNC_DATA_STORE_HPP
#include <array>
#include <memory>
#include <boost/noncopyable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queries/LocalDataStore.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queues/RoutineTaskQueue.hpp"

namespace Beam::Queries {
namespace Details {
  template<SnapshotLimit::Type D, typename C>
  struct Matches {
    static constexpr auto Direction = D;
    using Container = C;
    using Type = typename Container::value_type;
    using Iterator = std::conditional_t<D == SnapshotLimit::Type::HEAD, Type*,
      std::reverse_iterator<Type*>>;

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
  C MergeMatches(C match1, C match2, C match3, int limit) {
    constexpr auto MATCH_COUNT = 3;
    auto matches = std::array<Matches<D, C>, MATCH_COUNT>();
    auto matchesRemaining = 0;
    for(auto& match : {&match1, &match2, &match3}) {
      if(!match->empty()) {
        matches[matchesRemaining] = Matches<D, C>(*match);
        ++matchesRemaining;
      }
    }
    auto result = C();
    result.reserve(std::min(static_cast<std::size_t>(limit),
      match1.size() + match2.size() + match3.size()));
    auto comparator = [](const auto& lhs, const auto& rhs) {
      if(D == SnapshotLimit::Type::HEAD) {
        return SequenceComparator()(*lhs->m_current, *rhs->m_current);
      } else {
        return SequenceComparator()(*rhs->m_current, *lhs->m_current);
      }
    };
    while(result.size() != limit && matchesRemaining != 0) {
      auto& match = *([&] {
        if(matchesRemaining == 1) {
          return &matches[0];
        } else if(matchesRemaining == 2) {
          return std::min(&matches[0], &matches[1], comparator);
        } else {
          return std::min({&matches[0], &matches[1], &matches[2]}, comparator);
        }
      }());
      auto& value = *match.m_current;
      ++match.m_current;
      if(match.m_current == match.m_end) {
        std::swap(match, matches[matchesRemaining - 1]);
        --matchesRemaining;
      }
      if(result.empty() || value.GetSequence() != result.back().GetSequence()) {
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
   * @param <D> The type of data store to buffer writes to.
   * @param <E> The type of EvaluatorTranslator used for filtering values.
   */
  template<typename D, typename E =
    typename GetTryDereferenceType<D>::EvaluatorTranslatorFilter>
  class AsyncDataStore : private boost::noncopyable {
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
      using EvaluatorTranslatorFilter = E;

      /**
       * Constructs an AsyncDataStore.
       * @param dataStore Initializes the data store to buffer data to.
       */
      template<typename DS>
      explicit AsyncDataStore(DS&& dataStore);

      ~AsyncDataStore();

      std::vector<SequencedValue> Load(const Query& query);

      void Store(const IndexedValue& value);

      void Store(const std::vector<IndexedValue>& values);

      void Open();

      void Close();

    private:
      using ReserveDataStore = LocalDataStore<Query, Value,
        EvaluatorTranslatorFilter>;
      mutable boost::mutex m_mutex;
      GetOptionalLocalPtr<D> m_dataStore;
      std::shared_ptr<ReserveDataStore> m_currentDataStore;
      std::shared_ptr<ReserveDataStore> m_flushedDataStore;
      bool m_isFlushing;
      IO::OpenState m_openState;
      RoutineTaskQueue m_tasks;

      void Shutdown();
      void TestFlush();
      void Flush();
  };

  template<typename DS>
  AsyncDataStore(DS&& dataStore) -> AsyncDataStore<std::remove_reference_t<DS>>;

  template<typename D, typename E>
  template<typename DS>
  AsyncDataStore<D, E>::AsyncDataStore(DS&& dataStore)
    : m_dataStore(std::forward<DS>(dataStore)),
      m_currentDataStore(std::make_shared<ReserveDataStore>()),
      m_flushedDataStore(std::make_shared<ReserveDataStore>()),
      m_isFlushing(false) {}

  template<typename D, typename E>
  std::vector<typename AsyncDataStore<D, E>::SequencedValue>
      AsyncDataStore<D, E>::Load(const Query& query) {
    auto [currentDataStore, flushedDataStore] = [&] {
      auto lock = boost::lock_guard(m_mutex);
      return std::tuple{m_currentDataStore, m_flushedDataStore};
    }();
    if(query.GetSnapshotLimit().GetType() == SnapshotLimit::Type::HEAD) {
      return Details::MergeMatches<SnapshotLimit::Type::HEAD>(
        currentDataStore->Load(query), flushedDataStore->Load(query),
        m_dataStore->Load(query), query.GetSnapshotLimit().GetSize());
    } else {
      return Details::MergeMatches<SnapshotLimit::Type::TAIL>(
        currentDataStore->Load(query), flushedDataStore->Load(query),
        m_dataStore->Load(query), query.GetSnapshotLimit().GetSize());
    }
  }

  template<typename D, typename E>
  void AsyncDataStore<D, E>::Store(const IndexedValue& value) {
    auto lock = boost::lock_guard(m_mutex);
    m_currentDataStore->Store(value);
    TestFlush();
  }

  template<typename D, typename E>
  void AsyncDataStore<D, E>::Store(const std::vector<IndexedValue>& values) {
    auto lock = boost::lock_guard(m_mutex);
    m_currentDataStore->Store(values);
    TestFlush();
  }

  template<typename D, typename E>
  AsyncDataStore<D, E>::~AsyncDataStore() {
    Close();
  }

  template<typename D, typename E>
  void AsyncDataStore<D, E>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_dataStore->Open();
      m_currentDataStore->Open();
      m_flushedDataStore->Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename D, typename E>
  void AsyncDataStore<D, E>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename D, typename E>
  void AsyncDataStore<D, E>::Shutdown() {
    auto writeToken = Routines::Async<void>();
    m_tasks.Push(
      [&] {
        writeToken.GetEval().SetResult();
      });
    writeToken.Get();
    m_openState.SetClosed();
  }

  template<typename D, typename E>
  void AsyncDataStore<D, E>::TestFlush() {
    if(!m_isFlushing) {
      m_isFlushing = true;
      m_tasks.Push(
        [=] {
          Flush();
        });
    }
  }

  template<typename D, typename E>
  void AsyncDataStore<D, E>::Flush() {
    {
      auto lock = boost::lock_guard(m_mutex);
      m_flushedDataStore.swap(m_currentDataStore);
      m_isFlushing = false;
    }
    m_dataStore->Store(m_flushedDataStore->LoadAll());
    auto newDataStore = std::make_shared<ReserveDataStore>();
    newDataStore->Open();
    {
      auto lock = boost::lock_guard(m_mutex);
      m_flushedDataStore = std::move(newDataStore);
    }
  }
}

#endif
