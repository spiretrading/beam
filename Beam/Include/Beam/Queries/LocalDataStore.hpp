#ifndef BEAM_LOCAL_DATA_STORE_HPP
#define BEAM_LOCAL_DATA_STORE_HPP
#include <algorithm>
#include <utility>
#include <vector>
#include "Beam/Collections/SynchronizedMap.hpp"
#include "Beam/Queries/IndexedValue.hpp"
#include "Beam/Queries/LocalDataStoreEntry.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/SequencedValue.hpp"

namespace Beam::Queries {

  /**
   * Loads and stores SequencedValue's in memory.
   * @param <Q> The type of query used to load values.
   * @param <V> The type value to store.
   * @param <T> The type of EvaluatorTranslator used for filtering values.
   */
  template<typename Q, typename V, typename T>
  class LocalDataStore {
    public:

      /** The type of query used to load values. */
      using Query = Q;

      /** The type of index used. */
      using Index = typename Query::Index;

      /** The type of value to store. */
      using Value = V;

      /** The SequencedValue to store. */
      using SequencedValue = ::Beam::Queries::SequencedValue<Value>;

      /** The IndexedValue to store. */
      using IndexedValue = ::Beam::Queries::SequencedValue<
        ::Beam::Queries::IndexedValue<Value, Index>>;

      /** The type of EvaluatorTranslator used for filtering values. */
      using EvaluatorTranslatorFilter = T;

      /** Constructs a LocalDataStore. */
      template<typename... Args>
      explicit LocalDataStore(const Args&... args);

      /** Returns all the values stored by this data store. */
      std::vector<IndexedValue> LoadAll() const;

      /**
       * Executes a search query.
       * @param query The search query to execute.
       * @return The list of the values that satisfy the search <i>query</i>.
       */
      std::vector<SequencedValue> Load(const Query& query) const;

      /**
       * Stores a Value.
       * @param value The Value to store.
       */
      void Store(const IndexedValue& value);

      /**
       * Stores a list of Values.
       * @param values The Values to store.
       */
      void Store(const std::vector<IndexedValue>& values);

      void Close();

    private:
      using Entry = LocalDataStoreEntry<Q, V, T>;
      using EntryMap = SynchronizedUnorderedMap<Index, Entry>;
      typename Entry::Translator m_translator;
      EntryMap m_entries;

      LocalDataStore(const LocalDataStore&) = delete;
      LocalDataStore& operator =(const LocalDataStore&) = delete;
  };

  template<typename Q, typename V, typename T>
  template<typename... Args>
  LocalDataStore<Q, V, T>::LocalDataStore(const Args&... args)
    : m_translator([=] (const auto& expression) {
        return Translate<EvaluatorTranslatorFilter>(expression, args...);
      }) {}

  template<typename Q, typename V, typename T>
  std::vector<typename LocalDataStore<Q, V, T>::IndexedValue>
      LocalDataStore<Q, V, T>::LoadAll() const {
    return m_entries.With([&] (const auto& entries) {
      auto values = std::vector<IndexedValue>();
      for(auto& entry : entries) {
        auto& index = entry.first;
        auto& dataStore = entry.second;
        auto indexEntries = dataStore.LoadAll();
        std::transform(indexEntries.begin(), indexEntries.end(),
          std::back_inserter(values), [&] (const auto& value) {
            return Queries::SequencedValue(
              Queries::IndexedValue(*value, index), value.GetSequence());
          });
      }
      return values;
    });
  }

  template<typename Q, typename V, typename T>
  std::vector<typename LocalDataStore<Q, V, T>::SequencedValue>
      LocalDataStore<Q, V, T>::Load(const Query& query) const {
    if(auto entry = m_entries.Find(query.GetIndex())) {
      return entry->Load(query);
    }
    return {};
  }

  template<typename Q, typename V, typename T>
  void LocalDataStore<Q, V, T>::Store(const IndexedValue& value) {
    auto& entry = m_entries.Get(value->GetIndex());
    entry.Store(value);
  }

  template<typename Q, typename V, typename T>
  void LocalDataStore<Q, V, T>::Store(const std::vector<IndexedValue>& values) {
    for(auto& value : values) {
      Store(value);
    }
  }

  template<typename Q, typename V, typename T>
  void LocalDataStore<Q, V, T>::Close() {}
}

#endif
