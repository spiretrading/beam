#ifndef BEAM_LOCAL_DATA_STORE_ENTRY_HPP
#define BEAM_LOCAL_DATA_STORE_ENTRY_HPP
#include <algorithm>
#include <functional>
#include <iostream>
#include <vector>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include "Beam/Collections/SynchronizedList.hpp"
#include "Beam/Queries/Evaluator.hpp"
#include "Beam/Queries/EvaluatorTranslator.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/Range.hpp"
#include "Beam/Queries/RangedQuery.hpp"
#include "Beam/Queries/SequencedValue.hpp"
#include "Beam/Queries/SnapshotLimit.hpp"
#include "Beam/Queries/SnapshotLimitedQuery.hpp"

namespace Beam::Queries {

  /**
   * Loads and stores SequencedValue's in memory.
   * @param <Q> The type of query used to load values.
   * @param <V> The type value to store.
   * @param <T> The type of EvaluatorTranslator used for filtering values.
   */
  template<typename Q, typename V, typename T>
  class LocalDataStoreEntry {
    public:

      /** The type of query used to load values. */
      using Query = Q;

      /** The type of value to store. */
      using Value = V;

      /** The SequencedValue to store. */
      using SequencedValue = ::Beam::Queries::SequencedValue<Value>;

      /** The type of EvaluatorTranslator used for filtering values. */
      using EvaluatorTranslatorFilter = T;

      /**
       * The type of function used to translate an Expression.
       * @param expression The Expression to translate.
       * @return The Evaluator representing the <i>expression</i>.
       */
      using Translator = std::function<
        std::unique_ptr<Evaluator> (const Expression& expression)>;

      /** Constructs a LocalDataStoreEntry. */
      LocalDataStoreEntry();

      /**
       * Constructs a LocalDataStoreEntry with a custom Translator.
       * @param translator The Translator to use.
       */
      explicit LocalDataStoreEntry(const Translator& translator);

      /** Returns all the values stored by this data store. */
      std::vector<SequencedValue> LoadAll() const;

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
      void Store(const SequencedValue& value);

      /**
       * Stores a list of Values.
       * @param values The Values to store.
       */
      void Store(const std::vector<SequencedValue>& values);

    private:
      using ValueList = SynchronizedVector<SequencedValue>;
      ValueList m_values;
      Translator m_translator;
  };

  template<typename Q, typename V, typename T>
  LocalDataStoreEntry<Q, V, T>::LocalDataStoreEntry()
    : m_translator([] (const auto& expression) {
        return Translate<EvaluatorTranslatorFilter>(expression);
      }) {}

  template<typename Q, typename V, typename T>
  LocalDataStoreEntry<Q, V, T>::LocalDataStoreEntry(
    const Translator& translator)
    : m_translator(translator) {}

  template<typename Q, typename V, typename T>
  std::vector<typename LocalDataStoreEntry<Q, V, T>::SequencedValue>
      LocalDataStoreEntry<Q, V, T>::LoadAll() const {
    return m_values.Acquire();
  }

  template<typename Q, typename V, typename T>
  std::vector<typename LocalDataStoreEntry<Q, V, T>::SequencedValue>
      LocalDataStoreEntry<Q, V, T>::Load(const Query& query) const {
    if(query.GetSnapshotLimit().GetSize() == 0 ||
        query.GetRange().GetStart() == Sequence::Present() ||
        query.GetRange().GetStart() == Sequence::Last()) {
      return {};
    }
    auto matches = std::vector<SequencedValue>();
    auto& startPoint = query.GetRange().GetStart();
    auto& endPoint = query.GetRange().GetEnd();
    auto filter = m_translator(query.GetFilter());
    m_values.With([&] (const auto& values) {
      if(query.GetSnapshotLimit().GetType() == SnapshotLimit::Type::TAIL) {
        for(auto& value : boost::adaptors::reverse(values)) {
          if(RangePointGreaterOrEqual(value, startPoint) &&
              RangePointLesserOrEqual(value, endPoint) &&
              TestFilter(*filter, *value)) {
            matches.push_back(value);
            if(static_cast<int>(matches.size()) >=
                query.GetSnapshotLimit().GetSize()) {
              break;
            }
          }
        }
      } else {
        for(auto& value : values) {
          if(RangePointGreaterOrEqual(value, startPoint) &&
              RangePointLesserOrEqual(value, endPoint) &&
              TestFilter(*filter, *value)) {
            matches.push_back(value);
            if(static_cast<int>(matches.size()) >=
                query.GetSnapshotLimit().GetSize()) {
              break;
            }
          }
        }
      }
    });
    if(query.GetSnapshotLimit().GetType() == SnapshotLimit::Type::TAIL) {
      std::reverse(matches.begin(), matches.end());
    }
    return matches;
  }

  template<typename Q, typename V, typename T>
  void LocalDataStoreEntry<Q, V, T>::Store(const SequencedValue& value) {
    m_values.With([&] (auto& values) {
      if(values.empty() || value.GetSequence() > values.back().GetSequence()) {
        values.push_back(value);
        return;
      }
      auto insertIterator = std::lower_bound(values.begin(), values.end(),
        value, SequenceComparator());
      if(insertIterator != values.end() &&
          insertIterator->GetSequence() == value.GetSequence()) {
        *insertIterator = value;
      } else {
        values.insert(insertIterator, value);
      }
    });
  }

  template<typename Q, typename V, typename T>
  void LocalDataStoreEntry<Q, V, T>::Store(
      const std::vector<SequencedValue>& values) {
    for(auto& value : values) {
      Store(value);
    }
  }
}

#endif
