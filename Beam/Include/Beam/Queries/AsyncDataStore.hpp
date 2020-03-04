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
      m_currentDataStore(std::make_shared<ReserveDataStore>()),
      m_flushedDataStore(m_currentDataStore) {}

  
  template<typename D, typename E>
  std::vector<typename AsyncDataStore<D, E>::SequencedValue>
      AsyncDataStore<D, E>::Load(const Query& query) {
    auto buffer = std::shared_ptr<ReserveDataStore>();
    {
      auto lock = boost::lock_guard(m_mutex);
      buffer = m_flushedDataStore;
    }
    auto matches = std::vector<SequencedValue>();
    if(query.GetSnapshotLimit().GetType() == SnapshotLimit::Type::HEAD) {
      matches = m_dataStore->Load(query);
    } else {
      matches = buffer->Load(query);
    }
    if(static_cast<int>(matches.size()) < query.GetSnapshotLimit().GetSize()) {
      auto additionalMatches = std::vector<SequencedValue>();
      if(query.GetSnapshotLimit().GetType() == SnapshotLimit::Type::HEAD) {
        additionalMatches = buffer->Load(query);
      } else {
        additionalMatches = m_dataStore->Load(query);
      }
      auto mergedMatches = std::vector<SequencedValue>();
      MergeWithoutDuplicates(additionalMatches.begin(), additionalMatches.end(),
        matches.begin(), matches.end(), std::back_inserter(mergedMatches),
        SequenceComparator());
      if(static_cast<int>(mergedMatches.size()) >
          query.GetSnapshotLimit().GetSize()) {
        if(query.GetSnapshotLimit().GetType() == SnapshotLimit::Type::HEAD) {
          mergedMatches.erase(mergedMatches.begin() +
            query.GetSnapshotLimit().GetSize(), mergedMatches.end());
        } else {
          mergedMatches.erase(mergedMatches.begin(),
            mergedMatches.begin() +
            (mergedMatches.size() - query.GetSnapshotLimit().GetSize()));
        }
      }
      matches = std::move(mergedMatches);
    }
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
        Flush();
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
      [=] {
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
    }
    m_dataStore->Store(dataStore->LoadAll());
    {
      auto lock = boost::lock_guard(m_mutex);
      m_flushedDataStore = m_currentDataStore;
    }
  }
}

#endif
