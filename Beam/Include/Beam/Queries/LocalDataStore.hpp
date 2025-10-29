#ifndef BEAM_LOCAL_DATA_STORE_HPP
#define BEAM_LOCAL_DATA_STORE_HPP
#include <algorithm>
#include <utility>
#include <vector>
#include "Beam/Collections/SynchronizedMap.hpp"
#include "Beam/Queries/IndexedValue.hpp"
#include "Beam/Queries/LocalDataStoreEntry.hpp"
#include "Beam/Queries/SequencedValue.hpp"

namespace Beam {

  /**
   * Loads and stores SequencedValue's in memory.
   * @tparam Q The type of query used to load values.
   * @tparam V The type value to store.
   * @tparam T The type of EvaluatorTranslator used for filtering values.
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
      using SequencedValue = Beam::SequencedValue<Value>;

      /** The IndexedValue to store. */
      using IndexedValue =
        Beam::SequencedValue<Beam::IndexedValue<Value, Index>>;

      /** The type of EvaluatorTranslator used for filtering values. */
      using EvaluatorTranslatorFilter = T;

      /** Constructs a LocalDataStore. */
      template<typename... Args>
      explicit LocalDataStore(const Args&... args);

      /** Returns all the values stored by this data store. */
      std::vector<IndexedValue> load_all() const;

      /**
       * Executes a search query.
       * @param query The search query to execute.
       * @return The list of the values that satisfy the search <i>query</i>.
       */
      std::vector<SequencedValue> load(const Query& query) const;

      /**
       * Stores a Value.
       * @param value The Value to store.
       */
      void store(const IndexedValue& value);

      /**
       * Stores a list of Values.
       * @param values The Values to store.
       */
      void store(const std::vector<IndexedValue>& values);

      void close();

    private:
      using Entry = LocalDataStoreEntry<Q, V, T>;
      typename Entry::Translator m_translator;
      SynchronizedUnorderedMap<Index, Entry> m_entries;

      LocalDataStore(const LocalDataStore&) = delete;
      LocalDataStore& operator =(const LocalDataStore&) = delete;
  };

  template<typename Q, typename V, typename T>
  template<typename... Args>
  LocalDataStore<Q, V, T>::LocalDataStore(const Args&... args)
    : m_translator([=] (const auto& expression) {
        return translate<EvaluatorTranslatorFilter>(expression, args...);
      }) {}

  template<typename Q, typename V, typename T>
  std::vector<typename LocalDataStore<Q, V, T>::IndexedValue>
      LocalDataStore<Q, V, T>::load_all() const {
    return m_entries.with([&] (const auto& entries) {
      auto values = std::vector<IndexedValue>();
      for(auto& entry : entries) {
        auto& index = entry.first;
        auto& data_store = entry.second;
        auto index_entries = data_store.load_all();
        std::transform(index_entries.begin(), index_entries.end(),
          std::back_inserter(values), [&] (const auto& value) {
            return Beam::SequencedValue(
              Beam::IndexedValue(*value, index), value.get_sequence());
          });
      }
      return values;
    });
  }

  template<typename Q, typename V, typename T>
  std::vector<typename LocalDataStore<Q, V, T>::SequencedValue>
      LocalDataStore<Q, V, T>::load(const Query& query) const {
    if(auto entry = m_entries.find(query.get_index())) {
      return entry->load(query);
    }
    return {};
  }

  template<typename Q, typename V, typename T>
  void LocalDataStore<Q, V, T>::store(const IndexedValue& value) {
    auto& entry = m_entries.get_or_insert(value->get_index(), [&] {
      return Entry(m_translator);
    });
    entry.store(value);
  }

  template<typename Q, typename V, typename T>
  void LocalDataStore<Q, V, T>::store(const std::vector<IndexedValue>& values) {
    for(auto& value : values) {
      store(value);
    }
  }

  template<typename Q, typename V, typename T>
  void LocalDataStore<Q, V, T>::close() {}
}

#endif
