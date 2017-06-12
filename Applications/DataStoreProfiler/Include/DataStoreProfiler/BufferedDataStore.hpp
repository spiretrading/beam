#ifndef BEAM_DATASTOREPROFILER_BUFFEREDDATASTORE_HPP
#define BEAM_DATASTOREPROFILER_BUFFEREDDATASTORE_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Queries/BufferedDataStore.hpp>
#include <Beam/Queries/EvaluatorTranslator.hpp>
#include <boost/noncopyable.hpp>
#include "DataStoreProfiler/DataStoreQueryWrapper.hpp"
#include "DataStoreProfiler/EntryQuery.hpp"

namespace Beam {

  /*! \class BufferedDataStore
      \brief Buffers writes to an underlying data store.
      \tparam BaseDataStoreType The underlying data store to commit the data to.
   */
  template<typename BaseDataStoreType>
  class BufferedDataStore : private boost::noncopyable {
    public:

      //! The type of DataStore to buffer.
      using BaseDataStore = GetTryDereferenceType<BaseDataStoreType>;

      //! Constructs a BufferedDataStore.
      /*!
        \param dataStore Initializes the data store to commit data to.
        \param bufferSize The number of messages to buffer before committing to
               to the <i>dataStore</i>.
        \param threadPool The ThreadPool to queue the writes to.
      */
      template<typename BaseDataStoreForward>
      BufferedDataStore(BaseDataStoreForward&& dataStore,
        std::size_t bufferSize, RefType<Threading::ThreadPool> threadPool);

      ~BufferedDataStore();

      void Clear();

      std::vector<SequencedEntry> LoadEntries(const EntryQuery& query);

      void Store(const SequencedIndexedEntry& entry);

      void Store(const std::vector<SequencedIndexedEntry>& entries);

      void Open();

      void Close();

    private:
      GetOptionalLocalPtr<BaseDataStoreType> m_dataStore;
      Queries::BufferedDataStore<DataStoreQueryWrapper<BaseDataStore*>,
        Queries::EvaluatorTranslator<Queries::QueryTypes>> m_bufferedDataStore;
      IO::OpenState m_openState;

      void Shutdown();
  };

  template<typename BaseDataStoreType>
  template<typename BaseDataStoreForward>
  BufferedDataStore<BaseDataStoreType>::BufferedDataStore(
      BaseDataStoreForward&& dataStore, std::size_t bufferSize,
      RefType<Threading::ThreadPool> threadPool)
      : m_dataStore{std::forward<BaseDataStoreForward>(dataStore)},
        m_bufferedDataStore{&*m_dataStore, bufferSize, Ref(threadPool)} {}

  template<typename BaseDataStoreType>
  BufferedDataStore<BaseDataStoreType>::~BufferedDataStore() {
    Close();
  }

  template<typename BaseDataStoreType>
  void BufferedDataStore<BaseDataStoreType>::Clear() {
    m_dataStore->Clear();
  }

  template<typename BaseDataStoreType>
  std::vector<SequencedEntry> BufferedDataStore<BaseDataStoreType>::LoadEntries(
      const EntryQuery& query) {
    return m_bufferedDataStore.Load(query);
  }

  template<typename BaseDataStoreType>
  void BufferedDataStore<BaseDataStoreType>::Store(
      const SequencedIndexedEntry& entry) {
    m_bufferedDataStore.Store(entry);
  }

  template<typename BaseDataStoreType>
  void BufferedDataStore<BaseDataStoreType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_dataStore->Open();
      m_bufferedDataStore.Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename BaseDataStoreType>
  void BufferedDataStore<BaseDataStoreType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename BaseDataStoreType>
  void BufferedDataStore<BaseDataStoreType>::Shutdown() {
    m_bufferedDataStore.Close();
    m_dataStore->Close();
    m_openState.SetClosed();
  }
}

#endif
