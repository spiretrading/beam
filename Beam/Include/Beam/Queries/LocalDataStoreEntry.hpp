#ifndef BEAM_LOCALDATASTOREENTRY_HPP
#define BEAM_LOCALDATASTOREENTRY_HPP
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

namespace Beam {
namespace Queries {

  /*! \class LocalDataStoreEntry
      \brief Loads and stores SequencedValue's in memory.
      \tparam QueryType The type of query used to load values.
      \tparam ValueType The type value to store.
      \tparam EvaluatorTranslatorFilterType The type of EvaluatorTranslator used
              for filtering values.
   */
  template<typename QueryType, typename ValueType,
    typename EvaluatorTranslatorFilterType>
  class LocalDataStoreEntry {
    public:

      //! The type of query used to load values.
      using Query = QueryType;

      //! The type of value to store.
      using Value = ValueType;

      //! The SequencedValue to store.
      using SequencedValue = ::Beam::Queries::SequencedValue<Value>;

      //! The type of EvaluatorTranslator used for filtering values.
      using EvaluatorTranslatorFilter = EvaluatorTranslatorFilterType;

      //! The type of function used to translate an Expression.
      /*!
        \param expression The Expression to translate.
        \return The Evaluator representing the <i>expression</i>.
      */
      using Translator = std::function<
        std::unique_ptr<Evaluator> (const Expression& expression)>;

      //! Constructs a LocalDataStoreEntry.
      LocalDataStoreEntry();

      //! Constructs a LocalDataStoreEntry with a custom Translator.
      /*!
        \param translator The Translator to use.
      */
      LocalDataStoreEntry(const Translator& translator);

      //! Returns all the values stored by this data store.
      std::vector<SequencedValue> LoadAll() const;

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
      void Store(const SequencedValue& value);

      //! Stores a list of Values.
      /*!
        \param values The Values to store.
      */
      void Store(const std::vector<SequencedValue>& values);

    private:
      using ValueList = SynchronizedVector<SequencedValue>;
      ValueList m_values;
      Translator m_translator;
  };

  template<typename QueryType, typename ValueType,
    typename EvaluatorTranslatorFilterType>
  LocalDataStoreEntry<QueryType, ValueType, EvaluatorTranslatorFilterType>::
      LocalDataStoreEntry() {
    m_translator =
      [] (const Expression& expression) {
        return Translate<EvaluatorTranslatorFilter>(expression);
      };
  }

  template<typename QueryType, typename ValueType,
    typename EvaluatorTranslatorFilterType>
  LocalDataStoreEntry<QueryType, ValueType, EvaluatorTranslatorFilterType>::
      LocalDataStoreEntry(const Translator& translator)
      : m_translator{translator} {}

  template<typename QueryType, typename ValueType,
    typename EvaluatorTranslatorFilterType>
  std::vector<typename LocalDataStoreEntry<QueryType, ValueType,
      EvaluatorTranslatorFilterType>::SequencedValue> LocalDataStoreEntry<
      QueryType, ValueType, EvaluatorTranslatorFilterType>::LoadAll() const {
    return m_values.Acquire();
  }

  template<typename QueryType, typename ValueType,
    typename EvaluatorTranslatorFilterType>
  std::vector<typename LocalDataStoreEntry<QueryType, ValueType,
      EvaluatorTranslatorFilterType>::SequencedValue> LocalDataStoreEntry<
      QueryType, ValueType, EvaluatorTranslatorFilterType>::Load(
      const Query& query) const {
    std::vector<SequencedValue> matches;
    if(query.GetSnapshotLimit().GetSize() == 0) {
      return matches;
    }
    if(query.GetRange().GetStart() == Sequence::Present() ||
        query.GetRange().GetStart() == Sequence::Last()) {
      return matches;
    }
    auto& startPoint = query.GetRange().GetStart();
    auto& endPoint = query.GetRange().GetEnd();
    auto filter = m_translator(query.GetFilter());
    m_values.With(
      [&] (const typename ValueList::List& values) {
        if(query.GetSnapshotLimit().GetType() == SnapshotLimit::Type::TAIL) {
          for(const auto& value : boost::adaptors::reverse(values)) {
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
          for(const auto& value : values) {
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

  template<typename QueryType, typename ValueType,
    typename EvaluatorTranslatorFilterType>
  void LocalDataStoreEntry<QueryType, ValueType,
      EvaluatorTranslatorFilterType>::Store(const SequencedValue& value) {
    m_values.With(
      [&] (typename ValueList::List& values) {
        if(values.empty() ||
            value.GetSequence() > values.back().GetSequence()) {
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

  template<typename QueryType, typename ValueType,
    typename EvaluatorTranslatorFilterType>
  void LocalDataStoreEntry<QueryType, ValueType,
      EvaluatorTranslatorFilterType>::Store(
      const std::vector<SequencedValue>& values) {
    for(const auto& value : values) {
      Store(value);
    }
  }
}
}

#endif
