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
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/Range.hpp"
#include "Beam/Queues/RoutineTaskQueue.hpp"
#include "Beam/Utilities/Algorithm.hpp"

namespace Beam::Queries {

  /**
   * Buffers writes to a data store.
   * @param <D> The type of data store to buffer writes to.
   * @param <E> The type of EvaluatorTranslator used for filtering values.
   */
  template<typename D, typename E =
    typename GetTryDereferenceType<D>::EvaluatorTranslatorFilter>
  class BufferedDataStore {
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
       * Constructs a BufferedDataStore.
       * @param dataStore Initializes the data store to buffer data to.
       * @param bufferSize The number of messages to buffer before committing to
       *        to the <i>dataStore</i>.
       */
      template<typename DS>
      BufferedDataStore(DS&& dataStore, std::size_t bufferSize);

      ~BufferedDataStore();

      std::vector<SequencedValue> Load(const Query& query);

      void Store(const IndexedValue& value);

      void Store(const std::vector<IndexedValue>& values);

      void Close();

    private:
      using ReserveDataStore = LocalDataStore<Query, Value,
        EvaluatorTranslatorFilter>;
      mutable boost::mutex m_mutex;
      GetOptionalLocalPtr<D> m_dataStore;
      std::size_t m_bufferSize;
      std::size_t m_bufferCount;
      std::shared_ptr<ReserveDataStore> m_dataStoreBuffer;
      std::shared_ptr<ReserveDataStore> m_flushedDataStore;
      IO::OpenState m_openState;
      RoutineTaskQueue m_tasks;

      BufferedDataStore(const BufferedDataStore&) = delete;
      BufferedDataStore& operator =(const BufferedDataStore&) = delete;
      void Flush();
      void TestFlush();
  };

  template<typename D, typename E>
  template<typename DS>
  BufferedDataStore<D, E>::BufferedDataStore(DS&& dataStore,
    std::size_t bufferSize)
    : m_dataStore(std::forward<DS>(dataStore)),
      m_bufferSize(bufferSize),
      m_bufferCount(0),
      m_dataStoreBuffer(std::make_shared<ReserveDataStore>()),
      m_flushedDataStore(m_dataStoreBuffer) {}

  template<typename D, typename E>
  BufferedDataStore<D, E>::~BufferedDataStore() {
    Close();
  }

  template<typename D, typename E>
  std::vector<typename BufferedDataStore<D, E>::SequencedValue>
      BufferedDataStore<D, E>::Load(const Query& query) {
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
  void BufferedDataStore<D, E>::Store(const IndexedValue& value) {
    auto lock = boost::lock_guard(m_mutex);
    ++m_bufferCount;
    m_dataStoreBuffer->Store(value);
    TestFlush();
  }

  template<typename D, typename E>
  void BufferedDataStore<D, E>::Store(const std::vector<IndexedValue>& values) {
    auto lock = boost::lock_guard(m_mutex);
    m_bufferCount += values.size();
    m_dataStoreBuffer->Store(values);
    TestFlush();
  }

  template<typename D, typename E>
  void BufferedDataStore<D, E>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    auto writeToken = Routines::Async<void>();
    m_tasks.Push(
      [&] {
        Flush();
        writeToken.GetEval().SetResult();
      });
    writeToken.Get();
    m_openState.Close();
  }

  template<typename D, typename E>
  void BufferedDataStore<D, E>::TestFlush() {
    if(m_bufferCount < m_bufferSize) {
      return;
    }
    m_bufferCount = 0;
    m_tasks.Push(
      [=] {
        Flush();
      });
  }

  template<typename D, typename E>
  void BufferedDataStore<D, E>::Flush() {
    auto dataStore = std::make_shared<ReserveDataStore>();
    {
      auto lock = boost::lock_guard(m_mutex);
      dataStore.swap(m_dataStoreBuffer);
    }
    m_dataStore->Store(dataStore->LoadAll());
    {
      auto lock = boost::lock_guard(m_mutex);
      m_flushedDataStore = m_dataStoreBuffer;
    }
  }
}

#endif
