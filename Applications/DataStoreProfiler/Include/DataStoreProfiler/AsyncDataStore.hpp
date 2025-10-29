#ifndef BEAM_DATA_STORE_PROFILER_ASYNC_DATA_STORE_HPP
#define BEAM_DATA_STORE_PROFILER_ASYNC_DATA_STORE_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queries/AsyncDataStore.hpp>
#include <Beam/Queries/EvaluatorTranslator.hpp>
#include "DataStoreProfiler/DataStoreQueryWrapper.hpp"
#include "DataStoreProfiler/EntryQuery.hpp"

namespace Beam {

  /**
   * Performs asyncronous writes to an underlying data store.
   * @tparam D The underlying data store to commit the data to.
   */
  template<typename D>
  class AsyncProfileDataStore {
    public:

      /** The type of DataStore to buffer. */
      using BaseDataStore = dereference_t<D>;

      /**
       * Constructs an AsyncProfileDataStore.
       * @param data_store Initializes the data store to commit data to.
       */
      template<Initializes<D> DF>
      explicit AsyncProfileDataStore(DF&& data_store);

      ~AsyncProfileDataStore();

      void clear();
      std::vector<SequencedEntry> load_entries(const EntryQuery& query);
      void store(const SequencedIndexedEntry& entry);
      void store(const std::vector<SequencedIndexedEntry>& entries);
      void close();

    private:
      local_ptr_t<D> m_data_store;
      AsyncDataStore<DataStoreQueryWrapper<BaseDataStore*>,
        EvaluatorTranslator<QueryTypes>> m_async_data_store;
      OpenState m_open_state;

      AsyncProfileDataStore(const AsyncProfileDataStore&) = delete;
      AsyncProfileDataStore& operator =(const AsyncProfileDataStore&) = delete;
  };

  template<typename D>
  AsyncProfileDataStore(D&&) -> AsyncProfileDataStore<std::remove_cvref_t<D>>;

  template<typename D>
  template<Initializes<D> DF>
  AsyncProfileDataStore<D>::AsyncProfileDataStore(DF&& data_store)
    : m_data_store(std::forward<DF>(data_store)),
      m_async_data_store(&*m_data_store) {}

  template<typename D>
  AsyncProfileDataStore<D>::~AsyncProfileDataStore() {
    close();
  }

  template<typename D>
  void AsyncProfileDataStore<D>::clear() {
    m_data_store->clear();
  }

  template<typename D>
  std::vector<SequencedEntry> AsyncProfileDataStore<D>::load_entries(
      const EntryQuery& query) {
    return m_async_data_store.load(query);
  }

  template<typename D>
  void AsyncProfileDataStore<D>::store(const SequencedIndexedEntry& entry) {
    m_async_data_store.store(entry);
  }

  template<typename D>
  void AsyncProfileDataStore<D>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_async_data_store.close();
    m_data_store->close();
    m_open_state.close();
  }
}

#endif
