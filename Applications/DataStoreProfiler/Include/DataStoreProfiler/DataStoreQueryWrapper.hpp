#ifndef BEAM_DATA_STORE_PROFILER_DATA_STORE_QUERY_WRAPPER_HPP
#define BEAM_DATA_STORE_PROFILER_DATA_STORE_QUERY_WRAPPER_HPP
#include <utility>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include "DataStoreProfiler/EntryQuery.hpp"

namespace Beam {

  /**
   * Wraps a data store to adapt it to the query interface.
   * @tparam D The data store to wrap.
   */
  template<typename D>
  class DataStoreQueryWrapper {
    public:
      using Query = EntryQuery;
      using Value = Entry;
      using Index = std::string;
      using DataStore = dereference_t<D>;
      using SequencedValue = Beam::SequencedValue<Value>;
      using IndexedValue =
        Beam::SequencedValue<Beam::IndexedValue<Value, Index>>;

      template<Initializes<D> DF>
      explicit DataStoreQueryWrapper(DF&& data_store);

      ~DataStoreQueryWrapper();

      std::vector<SequencedEntry> load(const EntryQuery& query);
      void store(const IndexedValue& value);
      void store(const std::vector<IndexedValue>& values);
      void close();

    private:
      local_ptr_t<D> m_data_store;

      DataStoreQueryWrapper(const DataStoreQueryWrapper&) = delete;
      DataStoreQueryWrapper& operator =(const DataStoreQueryWrapper&) = delete;
  };

  template<typename D>
  DataStoreQueryWrapper(D&&) -> DataStoreQueryWrapper<std::remove_cvref_t<D>>;

  template<typename D>
  template<Initializes<D> DF>
  DataStoreQueryWrapper<D>::DataStoreQueryWrapper(DF&& data_store)
    : m_data_store(std::forward<DF>(data_store)) {}

  template<typename D>
  DataStoreQueryWrapper<D>::~DataStoreQueryWrapper() {
    close();
  }

  template<typename D>
  std::vector<SequencedEntry> DataStoreQueryWrapper<D>::load(
      const EntryQuery& query) {
    return m_data_store->load_entries(query);
  }

  template<typename D>
  void DataStoreQueryWrapper<D>::store(const IndexedValue& value) {
    m_data_store->store(value);
  }

  template<typename D>
  void DataStoreQueryWrapper<D>::store(const std::vector<IndexedValue>& values) {
    m_data_store->store(values);
  }

  template<typename D>
  void DataStoreQueryWrapper<D>::close() {
    m_data_store->close();
  }
}

#endif
