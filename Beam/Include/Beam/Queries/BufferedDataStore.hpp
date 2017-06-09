#ifndef BEAM_BUFFEREDDATASTORE_HPP
#define BEAM_BUFFEREDDATASTORE_HPP
#include <algorithm>
#include <memory>
#include <vector>
#include <boost/noncopyable.hpp>
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
#include "Beam/Routines/Async.hpp"
#include "Beam/Threading/ThreadPool.hpp"
#include "Beam/Utilities/Algorithm.hpp"

namespace Beam {
namespace Queries {

  /*! \class BufferedDataStore
      \brief Buffers writes to a data store.
      \tparam DataStoreType The type of data store to buffer writes to.
      \tparam EvaluatorTranslatorFilterType The type of EvaluatorTranslator used
              for filtering values.
   */
  template<typename DataStoreType, typename EvaluatorTranslatorFilterType =
    typename TryDereferenceType<DataStoreType>::type::EvaluatorTranslatorFilter>
  class BufferedDataStore : private boost::noncopyable {
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

      //! Constructs a BufferedDataStore.
      /*!
        \param dataStore Initializes the data store to buffer data to.
        \param bufferSize The number of messages to buffer before committing to
               to the <i>dataStore</i>.
        \param threadPool The ThreadPool to queue the writes to.
      */
      template<typename DataStoreForward>
      BufferedDataStore(DataStoreForward&& dataStore, std::size_t bufferSize,
        RefType<Threading::ThreadPool> threadPool);

      ~BufferedDataStore();

      std::vector<SequencedValue> Load(const Query& query);

      void Store(const IndexedValue& value);

      void Store(const std::vector<IndexedValue>& values);

      void Open();

      void Close();

    private:
      using ReserveDataStore = LocalDataStore<Query, Value,
        EvaluatorTranslatorFilter>;
      mutable boost::mutex m_mutex;
      GetOptionalLocalPtr<DataStoreType> m_dataStore;
      std::size_t m_bufferSize;
      std::size_t m_bufferCount;
      std::shared_ptr<ReserveDataStore> m_dataStoreBuffer;
      std::shared_ptr<ReserveDataStore> m_flushedDataStore;
      Threading::ThreadPool* m_threadPool;
      IO::OpenState m_openState;
      RoutineTaskQueue m_tasks;

      void Shutdown();
      void Flush();
      void TestFlush();
  };

  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
  template<typename DataStoreForward>
  BufferedDataStore<DataStoreType, EvaluatorTranslatorFilterType>::
      BufferedDataStore(DataStoreForward&& dataStore, std::size_t bufferSize,
      RefType<Threading::ThreadPool> threadPool)
      : m_dataStore(std::forward<DataStoreForward>(dataStore)),
        m_bufferSize(bufferSize),
        m_bufferCount(0),
        m_dataStoreBuffer(std::make_shared<ReserveDataStore>()),
        m_flushedDataStore(m_dataStoreBuffer),
        m_threadPool(threadPool.Get()) {}

  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
  BufferedDataStore<DataStoreType, EvaluatorTranslatorFilterType>::
      ~BufferedDataStore() {
    Close();
  }

  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
  std::vector<typename BufferedDataStore<DataStoreType,
      EvaluatorTranslatorFilterType>::SequencedValue>
      BufferedDataStore<DataStoreType, EvaluatorTranslatorFilterType>::Load(
      const Query& query) {
    std::shared_ptr<ReserveDataStore> buffer;
    {
      boost::lock_guard<boost::mutex> lock(m_mutex);
      buffer = m_flushedDataStore;
    }
    std::vector<SequencedValue> matches;
    if(query.GetSnapshotLimit().GetType() == SnapshotLimit::Type::HEAD) {
      matches = m_dataStore->Load(query);
    } else {
      matches = buffer->Load(query);
    }
    if(static_cast<int>(matches.size()) < query.GetSnapshotLimit().GetSize()) {
      std::vector<SequencedValue> additionalMatches;
      if(query.GetSnapshotLimit().GetType() == SnapshotLimit::Type::HEAD) {
        additionalMatches = buffer->Load(query);
      } else {
        additionalMatches = m_dataStore->Load(query);
      }
      std::vector<SequencedValue> mergedMatches;
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

  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
  void BufferedDataStore<DataStoreType, EvaluatorTranslatorFilterType>::Store(
      const IndexedValue& value) {
    boost::lock_guard<boost::mutex> lock(m_mutex);
    ++m_bufferCount;
    m_dataStoreBuffer->Store(value);
    TestFlush();
  }

  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
  void BufferedDataStore<DataStoreType, EvaluatorTranslatorFilterType>::Store(
      const std::vector<IndexedValue>& values) {
    boost::lock_guard<boost::mutex> lock(m_mutex);
    m_bufferCount += values.size();
    m_dataStoreBuffer->Store(values);
    TestFlush();
  }

  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
  void BufferedDataStore<DataStoreType, EvaluatorTranslatorFilterType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_dataStoreBuffer->Open();
      m_dataStore->Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
  void BufferedDataStore<DataStoreType, EvaluatorTranslatorFilterType>::
      Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
  void BufferedDataStore<DataStoreType,
      EvaluatorTranslatorFilterType>::Shutdown() {
    Routines::Async<void> writeToken;
    m_tasks.Push(
      [&] {
        Flush();
        writeToken.GetEval().SetResult();
      });
    writeToken.Get();
    m_openState.SetClosed();
  }

  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
  void BufferedDataStore<DataStoreType, EvaluatorTranslatorFilterType>::
      TestFlush() {
    if(m_bufferCount < m_bufferSize) {
      return;
    }
    m_bufferCount = 0;
    m_tasks.Push(
      [=] {
        Routines::Async<void> writeToken;
        m_threadPool->Queue(
          [=] {
            Flush();
          }, writeToken.GetEval());
        writeToken.Get();
      });
  }

  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
  void BufferedDataStore<DataStoreType, EvaluatorTranslatorFilterType>::
      Flush() {
    auto dataStore = std::make_shared<ReserveDataStore>();
    {
      boost::lock_guard<boost::mutex> lock(m_mutex);
      dataStore.swap(m_dataStoreBuffer);
    }
    m_dataStore->Store(dataStore->LoadAll());
    {
      boost::lock_guard<boost::mutex> lock(m_mutex);
      m_flushedDataStore = m_dataStoreBuffer;
    }
  }
}
}

#endif
