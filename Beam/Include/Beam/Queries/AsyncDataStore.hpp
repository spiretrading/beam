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
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/LocalDataStore.hpp"
#include "Beam/Queues/RoutineTaskQueue.hpp"
#include "Beam/Utilities/Algorithm.hpp"

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
          m_cur(m_begin + 1),
          m_nextValue(*m_begin) {}

      Type AcquireNext() {
        auto match = std::move(m_nextValue);
        if(m_cur == m_end) {
          m_cur = m_begin;
        } else {
          m_nextValue = *m_cur;
          ++m_cur;
        }
        return match;
      }

      const Type& PeekNext() {
        return m_nextValue;
      }

      bool IsDepleted() {
        return m_cur == m_begin;
      }

    private:
      Iterator m_begin;
      Iterator m_end;
      Iterator m_cur;
      Type& m_nextValue;
  };

  template<typename I>
  std::unique_ptr<Matches<I>> MakeMatches(I begin, I end) {
    return std::make_unique<Matches<I>>(std::move(begin), std::move(end));
  }

  template<SnapshotLimit::Type LT, typename T>
  std::tuple_element_t<0, T> MergeMatches(T matches, int limit) {
    using Type = typename std::tuple_element_t<0, T>::value_type;
    using Iterator = std::conditional_t<LT == SnapshotLimit::Type::HEAD,
      std::vector<Type>::iterator, std::vector<Type>::reverse_iterator>;
    auto data = std::list<std::unique_ptr<Matches<Iterator>>>();
    auto count = std::apply([&] (auto&... parts) {
      auto testInclude = [&] (auto& part) {
        if(!part.empty()) {
          if constexpr(LT == SnapshotLimit::Type::HEAD) {
            data.push_back(MakeMatches(part.begin(), part.end()));
          } else {
            data.push_back(MakeMatches(part.rbegin(), part.rend()));
          }
        }
        return part.size();
      };
      return (testInclude(parts) + ...);
    }, std::move(matches));
    auto result = std::vector<Type>();
    result.reserve(std::min(static_cast<std::size_t>(limit), count));
    for(auto i = 0; !data.empty() && i < limit; ++i) {
      auto partIter = data.begin();
      auto comparator = [](auto& lhs, auto& rhs) {
        return SequenceComparator()(lhs->PeekNext(), rhs->PeekNext());
      };
      if constexpr(LT == SnapshotLimit::Type::HEAD) {
        partIter = std::min_element(data.begin(), data.end(), comparator);
      } else {
        partIter = std::max_element(data.begin(), data.end(), comparator);
      }
      auto& part = *partIter;
      auto value = part->AcquireNext();
      if(part->IsDepleted()) {
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
      bool m_flushPending;
      bool m_flushInProgress;
      std::shared_ptr<ReserveDataStore> m_currentDataStore;
      std::shared_ptr<ReserveDataStore> m_flushedDataStore;
      IO::OpenState m_openState;
      RoutineTaskQueue m_tasks;

      void Shutdown();
      void Flush();
      void TestFlush();
  };

  template<typename D, typename E>
  template<typename DS>
  AsyncDataStore<D, E>::AsyncDataStore(DS&& dataStore)
    : m_dataStore(std::forward<DS>(dataStore)),
      m_flushPending(false),
      m_flushInProgress(false),
      m_currentDataStore(std::make_shared<ReserveDataStore>()),
      m_flushedDataStore(m_currentDataStore) {}
  
  template<typename D, typename E>
  std::vector<typename AsyncDataStore<D, E>::SequencedValue>
      AsyncDataStore<D, E>::Load(const Query& query) {
    auto latestMatches = std::vector<SequencedValue>();
    auto buffer = std::shared_ptr<ReserveDataStore>();
    {
      auto lock = boost::lock_guard(m_mutex);
      buffer = m_flushedDataStore;
      if(m_flushInProgress) {
        latestMatches = m_currentDataStore->Load(query);
      }
    }
    auto bufferedMatches = m_flushedDataStore->Load(query);
    auto dataStoreMatches = m_dataStore->Load(query);
    auto consolidatedMatches = std::make_tuple(std::move(latestMatches),
      std::move(bufferedMatches), std::move(dataStoreMatches));
    auto matches = [&] {
      if(query.GetSnapshotLimit().GetType() == SnapshotLimit::Type::HEAD) {
        return Details::MergeMatches<SnapshotLimit::Type::HEAD>(
          std::move(consolidatedMatches), query.GetSnapshotLimit().GetSize());
      } else {
        return Details::MergeMatches<SnapshotLimit::Type::TAIL>(
          std::move(consolidatedMatches), query.GetSnapshotLimit().GetSize());
      }
    }();
    return matches;
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
      m_currentDataStore->Open();
      m_dataStore->Open();
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
    if(m_flushPending) {
      return;
    }
    m_flushPending = true;
    m_tasks.Push(
      [&] {
        Flush();
      });
  }

  template<typename D, typename E>
  void AsyncDataStore<D, E>::Flush() {
    auto dataStore = std::make_shared<ReserveDataStore>();
    {
      auto lock = boost::lock_guard(m_mutex);
      dataStore.swap(m_currentDataStore);
      m_flushPending = false;
      m_flushInProgress = true;
    }
    m_dataStore->Store(dataStore->LoadAll());
    {
      auto lock = boost::lock_guard(m_mutex);
      m_flushInProgress = false;
      m_flushedDataStore = m_currentDataStore;
    }
  }
}

#endif
