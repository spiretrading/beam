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
#include "Beam/Queries/Range.hpp"
#include "Beam/Queries/RangedQuery.hpp"
#include "Beam/Queries/SequencedValue.hpp"
#include "Beam/Queries/SnapshotLimit.hpp"
#include "Beam/Queries/SnapshotLimitedQuery.hpp"

namespace Beam {

  /**
   * Loads and stores SequencedValue's in memory.
   * @tparam Q The type of query used to load values.
   * @tparam V The type value to store.
   * @tparam T The type of EvaluatorTranslator used for filtering values.
   */
  template<typename Q, typename V, typename T>
  class LocalDataStoreEntry {
    public:

      /** The type of query used to load values. */
      using Query = Q;

      /** The type of value to store. */
      using Value = V;

      /** The SequencedValue to store. */
      using SequencedValue = Beam::SequencedValue<Value>;

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
      std::vector<SequencedValue> load_all() const;

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
      void store(const SequencedValue& value);

      /**
       * Stores a list of Values.
       * @param values The Values to store.
       */
      void store(const std::vector<SequencedValue>& values);

    private:
      using ValueList = SynchronizedVector<SequencedValue>;
      ValueList m_values;
      Translator m_translator;
  };

  template<typename Q, typename V, typename T>
  LocalDataStoreEntry<Q, V, T>::LocalDataStoreEntry()
    : m_translator([] (const auto& expression) {
        return translate<EvaluatorTranslatorFilter>(expression);
      }) {}

  template<typename Q, typename V, typename T>
  LocalDataStoreEntry<Q, V, T>::LocalDataStoreEntry(
    const Translator& translator)
    : m_translator(translator) {}

  template<typename Q, typename V, typename T>
  std::vector<typename LocalDataStoreEntry<Q, V, T>::SequencedValue>
      LocalDataStoreEntry<Q, V, T>::load_all() const {
    return m_values.load();
  }

  template<typename Q, typename V, typename T>
  std::vector<typename LocalDataStoreEntry<Q, V, T>::SequencedValue>
      LocalDataStoreEntry<Q, V, T>::load(const Query& query) const {
    if(query.get_snapshot_limit().get_size() == 0 ||
        query.get_range().get_start() == Sequence::PRESENT ||
        query.get_range().get_start() == Sequence::LAST) {
      return {};
    }
    auto matches = std::vector<SequencedValue>();
    auto& start = query.get_range().get_start();
    auto& end = query.get_range().get_end();
    auto filter = m_translator(query.get_filter());
    m_values.with([&] (const auto& values) {
      if(query.get_snapshot_limit().get_type() == SnapshotLimit::Type::TAIL) {
        for(auto& value : boost::adaptors::reverse(values)) {
          if(range_point_greater_or_equal(value, start) &&
              range_point_lesser_or_equal(value, end) &&
              test_filter(*filter, *value)) {
            matches.push_back(value);
            if(static_cast<int>(matches.size()) >=
                query.get_snapshot_limit().get_size()) {
              break;
            }
          }
        }
      } else {
        for(auto& value : values) {
          if(range_point_greater_or_equal(value, start) &&
              range_point_lesser_or_equal(value, end) &&
              test_filter(*filter, *value)) {
            matches.push_back(value);
            if(static_cast<int>(matches.size()) >=
                query.get_snapshot_limit().get_size()) {
              break;
            }
          }
        }
      }
    });
    if(query.get_snapshot_limit().get_type() == SnapshotLimit::Type::TAIL) {
      std::reverse(matches.begin(), matches.end());
    }
    return matches;
  }

  template<typename Q, typename V, typename T>
  void LocalDataStoreEntry<Q, V, T>::store(const SequencedValue& value) {
    m_values.with([&] (auto& values) {
      if(values.empty() ||
          value.get_sequence() > values.back().get_sequence()) {
        values.push_back(value);
        return;
      }
      auto i = std::lower_bound(
        values.begin(), values.end(), value, SequenceComparator());
      if(i != values.end() && i->get_sequence() == value.get_sequence()) {
        *i = value;
      } else {
        values.insert(i, value);
      }
    });
  }

  template<typename Q, typename V, typename T>
  void LocalDataStoreEntry<Q, V, T>::store(
      const std::vector<SequencedValue>& values) {
    for(auto& value : values) {
      store(value);
    }
  }
}

#endif
