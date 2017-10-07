#ifndef BEAM_DATASTOREPROFILER_DATASTOREQUERYWRAPPER_HPP
#define BEAM_DATASTOREPROFILER_DATASTOREQUERYWRAPPER_HPP
#include <utility>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include "DataStoreProfiler/EntryQuery.hpp"

namespace Beam {
  template<typename DataStoreType>
  class DataStoreQueryWrapper {
    public:
      using Query = EntryQuery;
      using Value = Entry;
      using Index = std::string;
      using DataStore = GetTryDereferenceType<DataStoreType>;
      using SequencedValue = Queries::SequencedValue<Value>;
      using IndexedValue = Queries::SequencedValue<Queries::IndexedValue<
        Value, Index>>;

      template<typename DataStoreForward>
      DataStoreQueryWrapper(DataStoreForward&& dataStore)
          : m_dataStore{std::forward<DataStoreForward>(dataStore)} {}

      ~DataStoreQueryWrapper() {
        Close();
      }

      std::vector<SequencedEntry> Load(const EntryQuery& query) {
        return m_dataStore->LoadEntries(query);
      }

      void Store(const IndexedValue& value) {
        m_dataStore->Store(value);
      }

      void Store(const std::vector<IndexedValue>& values) {
        m_dataStore->Store(values);
      }

      void Open() {
        m_dataStore->Open();
      }

      void Close() {
        m_dataStore->Close();
      }

    private:
      GetOptionalLocalPtr<DataStoreType> m_dataStore;
  };
}

#endif
