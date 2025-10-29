#ifndef BEAM_DATA_STORE_PROFILER_BUFFERED_DATA_STORE_HPP
#define BEAM_DATA_STORE_PROFILER_BUFFERED_DATA_STORE_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queries/BufferedDataStore.hpp>
#include <Beam/Queries/EvaluatorTranslator.hpp>
#include "DataStoreProfiler/DataStoreQueryWrapper.hpp"
#include "DataStoreProfiler/EntryQuery.hpp"

namespace Beam {

  /**
   * Buffers writes to an underlying data store.
   * @tparam D The underlying data store to commit the data to.
   */
  template<typename D>
  class BufferedProfileDataStore {
    public:

      /** The type of DataStore to buffer. */
      using BaseDataStore = dereference_t<D>;

      /**
       * Constructs a BufferedProfileDataStore.
       * @param data_store Initializes the data store to commit data to.
       * @param buffer_size The number of messages to buffer before committing
       *        to the <i>data_store</i>.
       */
      template<Initializes<D> DF>
      BufferedProfileDataStore(DF&& data_store, std::size_t buffer_size);

      ~BufferedProfileDataStore();

      void clear();
      std::vector<SequencedEntry> load_entries(const EntryQuery& query);
      void store(const SequencedIndexedEntry& entry);
      void store(const std::vector<SequencedIndexedEntry>& entries);
      void close();

    private:
      local_ptr_t<D> m_data_store;
      BufferedDataStore<DataStoreQueryWrapper<BaseDataStore*>,
        EvaluatorTranslator<QueryTypes>> m_buffered_data_store;
      OpenState m_open_state;

      BufferedProfileDataStore(const BufferedProfileDataStore&) = delete;
      BufferedProfileDataStore& operator =(
        const BufferedProfileDataStore&) = delete;
  };

  template<typename D>
  BufferedProfileDataStore(D&&, std::size_t) ->
    BufferedProfileDataStore<std::remove_cvref_t<D>>;

  template<typename D>
  template<Initializes<D> DF>
  BufferedProfileDataStore<D>::BufferedProfileDataStore(
    DF&& data_store, std::size_t buffer_size)
    : m_data_store(std::forward<DF>(data_store)),
      m_buffered_data_store(&*m_data_store, buffer_size) {}

  template<typename D>
  BufferedProfileDataStore<D>::~BufferedProfileDataStore() {
    close();
  }

  template<typename D>
  void BufferedProfileDataStore<D>::clear() {
    m_data_store->clear();
  }

  template<typename D>
  std::vector<SequencedEntry> BufferedProfileDataStore<D>::load_entries(
      const EntryQuery& query) {
    return m_buffered_data_store.load(query);
  }

  template<typename D>
  void BufferedProfileDataStore<D>::store(const SequencedIndexedEntry& entry) {
    m_buffered_data_store.store(entry);
  }

  template<typename D>
  void BufferedProfileDataStore<D>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_buffered_data_store.close();
    m_data_store->close();
    m_open_state.close();
  }
}

#endif
