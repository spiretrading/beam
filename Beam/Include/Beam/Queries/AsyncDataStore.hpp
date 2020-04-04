#ifndef BEAM_ASYNC_DATA_STORE_HPP
#define BEAM_ASYNC_DATA_STORE_HPP
#include <memory>
#include <vector>
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
  template<typename I>
  class Matches {
    public:
      using Iterator = I;
      using Type = typename Iterator::value_type;

      Matches(Iterator begin, Iterator end)
        : m_begin(std::move(begin)),
          m_end(std::move(end)),
          m_cur(std::next(m_begin)),
          m_nextValue(&*m_begin) {}

      Type AcquireNext() {
        auto match = std::move(*m_nextValue);
        if(m_cur == m_end) {
          m_cur = m_begin;
        } else {
          *m_nextValue = *m_cur;
          ++m_cur;
        }
        return match;
      }

      const Type& PeekNext() const {
        return *m_nextValue;
      }

      bool IsDepleted() const {
        return m_cur == m_begin;
      }

    private:
      Iterator m_begin;
      Iterator m_end;
      Iterator m_cur;
      Type* m_nextValue;
  };

  template<SnapshotLimit::Type LT, typename V>
  V MergeMatches(V match1, V match2, V match3, int limit) {
    using Iterator = std::conditional_t<LT == SnapshotLimit::Type::HEAD,
      typename V::iterator, typename V::reverse_iterator>;
    auto data = std::list<Matches<Iterator>>();
    auto count = static_cast<int>(match1.size() + match2.size() + match3.size());
    if constexpr(LT == SnapshotLimit::Type::HEAD) {
      if(!match1.empty()) {
        data.push_back(Matches(match1.begin(), match1.end()));
      }
      if(!match2.empty()) {
        data.push_back(Matches(match2.begin(), match2.end()));
      }
      if(!match3.empty()) {
        data.push_back(Matches(match3.begin(), match3.end()));
      }
    } else {
      if(!match1.empty()) {
        data.push_back(Matches(match1.rbegin(), match1.rend()));
      }
      if(!match2.empty()) {
        data.push_back(Matches(match2.rbegin(), match2.rend()));
      }
      if(!match3.empty()) {
        data.push_back(Matches(match3.rbegin(), match3.rend()));
      }
    }
    auto result = V();
    result.reserve(std::min(limit, count));
    auto comparator = [](const auto& lhs, const auto& rhs) {
      return SequenceComparator()(lhs.PeekNext(), rhs.PeekNext());
    };
    for(auto i = 0; !data.empty() && i < limit; ++i) {
      auto partIter = [&] {
        if constexpr(LT == SnapshotLimit::Type::HEAD) {
          return std::min_element(data.begin(), data.end(), comparator);
        } else {
          return std::max_element(data.begin(), data.end(), comparator);
        }
      }();
      auto& part = *partIter;
      auto value = part.AcquireNext();
      if(part.IsDepleted()) {
        data.erase(partIter);
      }
      if(result.empty() || value != result.back()) {
        result.push_back(std::move(value));
      }
    }
    if constexpr(LT == SnapshotLimit::Type::TAIL) {
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
      AsyncDataStore(DS&& dataStore);

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
