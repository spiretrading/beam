#ifndef BEAM_LOCALDATASTORE_HPP
#define BEAM_LOCALDATASTORE_HPP
#include <algorithm>
#include <vector>
#include <boost/noncopyable.hpp>
#include "Beam/Queries/IndexedValue.hpp"
#include "Beam/Queries/LocalDataStoreEntry.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/SequencedValue.hpp"
#include "Beam/Utilities/SynchronizedMap.hpp"

namespace Beam {
namespace Queries {

  /*! \class LocalDataStore
      \brief Loads and stores SequencedValue's in memory.
      \tparam QueryType The type of query used to load values.
      \tparam ValueType The type value to store.
      \tparam EvaluatorTranslatorFilterType The type of EvaluatorTranslator used
              for filtering values.
   */
  template<typename QueryType, typename ValueType,
    typename EvaluatorTranslatorFilterType>
  class LocalDataStore : private boost::noncopyable {
    public:

      //! The type of query used to load values.
      using Query = QueryType;

      //! The type of index used.
      using Index = typename Query::Index;

      //! The type of value to store.
      using Value = ValueType;

      //! The SequencedValue to store.
      using SequencedValue = ::Beam::Queries::SequencedValue<Value>;

      //! The IndexedValue to store.
      using IndexedValue = ::Beam::Queries::SequencedValue<
        ::Beam::Queries::IndexedValue<Value, Index>>;

      //! The type of EvaluatorTranslator used for filtering values.
      using EvaluatorTranslatorFilter = EvaluatorTranslatorFilterType;

      //! Constructs a LocalDataStore.
      template<typename...Args>
      LocalDataStore(const Args&... args);

      //! Returns all the values stored by this data store.
      std::vector<IndexedValue> LoadAll() const;

      //! Executes a search query.
      /*!
        \param query The search query to execute.
        \return The list of the values that satisfy the search <i>query</i>.
      */
      std::vector<SequencedValue> Load(const Query& query) const;

      //! Stores a Value.
      /*!
        \param value The Value to store.
      */
      void Store(const IndexedValue& value);

      //! Stores a list of Values.
      /*!
        \param values The Values to store.
      */
      void Store(const std::vector<IndexedValue>& values);

      void Open();

      void Close();

    private:
      using Entry = LocalDataStoreEntry<QueryType, ValueType,
        EvaluatorTranslatorFilterType>;
      using EntryMap = SynchronizedUnorderedMap<Index, Entry>;
      typename Entry::Translator m_translator;
      EntryMap m_entries;
  };

  template<typename QueryType, typename ValueType,
    typename EvaluatorTranslatorFilterType>
  template<typename...Args>
  LocalDataStore<QueryType, ValueType, EvaluatorTranslatorFilterType>::
      LocalDataStore(const Args&... args) {
    m_translator =
      [=] (const Expression& expression) {
        return Translate<EvaluatorTranslatorFilter>(expression, args...);
      };
  }

  template<typename QueryType, typename ValueType,
    typename EvaluatorTranslatorFilterType>
  std::vector<typename LocalDataStore<QueryType, ValueType,
      EvaluatorTranslatorFilterType>::IndexedValue> LocalDataStore<QueryType,
      ValueType, EvaluatorTranslatorFilterType>::LoadAll() const {
    std::vector<IndexedValue> values;
    m_entries.With(
      [&] (auto& entries) {
        for(auto& entry : entries) {
          auto& index = entry.first;
          auto& dataStore = entry.second;
          auto indexEntries = dataStore.LoadAll();
          std::transform(indexEntries.begin(), indexEntries.end(),
            std::back_inserter(values),
            [&] (const SequencedValue& value) {
              return MakeSequencedValue(MakeIndexedValue(*value, index),
                value.GetSequence());
            });
        }
      });
    return values;
  }

  template<typename QueryType, typename ValueType,
    typename EvaluatorTranslatorFilterType>
  std::vector<typename LocalDataStore<QueryType, ValueType,
      EvaluatorTranslatorFilterType>::SequencedValue> LocalDataStore<QueryType,
      ValueType, EvaluatorTranslatorFilterType>::Load(
      const Query& query) const {
    auto entry = m_entries.Find(query.GetIndex());
    if(!entry.is_initialized()) {
      return {};
    }
    return entry->Load(query);
  }

  template<typename QueryType, typename ValueType,
    typename EvaluatorTranslatorFilterType>
  void LocalDataStore<QueryType, ValueType, EvaluatorTranslatorFilterType>::
      Store(const IndexedValue& value) {
    auto& entry = m_entries.Get(value->GetIndex());
    entry.Store(value);
  }

  template<typename QueryType, typename ValueType,
    typename EvaluatorTranslatorFilterType>
  void LocalDataStore<QueryType, ValueType, EvaluatorTranslatorFilterType>::
      Store(const std::vector<IndexedValue>& values) {
    for(auto& value : values) {
      Store(value);
    }
  }

  template<typename QueryType, typename ValueType,
    typename EvaluatorTranslatorFilterType>
  void LocalDataStore<QueryType, ValueType, EvaluatorTranslatorFilterType>::
      Open() {}

  template<typename QueryType, typename ValueType,
    typename EvaluatorTranslatorFilterType>
  void LocalDataStore<QueryType, ValueType, EvaluatorTranslatorFilterType>::
      Close() {}
}
}

#endif
