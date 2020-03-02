#ifndef BEAM_ASYNC_DATA_STORE_HPP
#define BEAM_ASYNC_DATA_STORE_HPP
#include <vector>
#include <boost/noncopyable.hpp>
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/LocalDataStore.hpp"
#include "Beam/Queues/RoutineTaskQueue.hpp"

namespace Beam::Queries {

  /**
   * Asynchronously writes to a data store.
   * @param <D> The type of data store to buffer writes to.
   * @param <E> The type of EvaluatorTranslator used for filtering values.
   */
  template<typename D, typename E =
    typename GetTryDereferenceType<D>::EvaluatorTranslatorFilter>
  class AsyncDataStore : private boost::noncopyable {
    public:

      /** The type of data store to buffer writes to. */
      using DataStore = GetTryDereferenceType<D>;

      /** The type of query used to load values. */
      using Query = typename DataStore::Query;

      /** The type of index used. */
      using Index = typename DataStore::Index;

      /** The type of value to store. */
      using Value = typename DataStore::Value;

      /** The SequencedValue to store. */
      using SequencedValue = typename DataStore::SequencedValue;

      /** The IndexedValue to store. */
      using IndexedValue = typename DataStore::IndexedValue;

      /** The type of EvaluatorTranslator used for filtering values. */
      using EvaluatorTranslatorFilter = E;

      /**
       * Constructs an AsyncDataStore.
       * @param dataStore Initializes the data store to buffer data to.
       */
      template<typename DS>
      AsyncDataStore(DS&& dataStore);

      ~AsyncDataStore();

      std::vector<SequencedValue> Load(const Query& query);

      void Store(const IndexedValue& value);

      void Store(const std::vector<IndexedValue>& values);

      void Open();

      void Close();

    private:
      using ReserveDataStore = LocalDataStore<Query, Value,
        EvaluatorTranslatorFilter>;
      GetOptionalLocalPtr<D> m_dataStore;
      IO::OpenState m_openState;
      RoutineTaskQueue m_tasks;
  };

  template<typename D, typename E>
  template<typename DS>
  AsyncDataStore<D, E>::AsyncDataStore(DS&& dataStore)
    : m_dataStore(std::forward<DS>(dataStore)) {}

  template<typename D, typename E>
  AsyncDataStore<D, E>::~AsyncDataStore() {
    Close();
  }
}

#endif
