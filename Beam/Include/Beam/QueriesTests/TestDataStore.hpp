#ifndef BEAM_TEST_DATA_STORE_HPP
#define BEAM_TEST_DATA_STORE_HPP
#include <memory>
#include <variant>
#include <vector>
#include <boost/noncopyable.hpp>
#include "Beam/IO/OpenState.hpp"
#include "Beam/Queries/IndexedValue.hpp"
#include "Beam/Queries/SequencedValue.hpp"
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Queues/QueueReader.hpp"
#include "Beam/Queues/QueueWriterPublisher.hpp"
#include "Beam/Routines/Async.hpp"

namespace Beam::Queries::Tests {

  /**
   * Implements a DataStore for testing purposes by reifying operations.
   * @param <Q> The type of query used to load values.
   * @param <V> The type value to store.
   */
  template<typename Q, typename V>
  class TestDataStore : private boost::noncopyable {
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

      /** Stores a load operation. */
      struct LoadOperation {

        /** The query submitted by the load. */
        Query m_query;

        /** Used to produce the result of the load operation. */
        Routines::Eval<std::vector<SequencedValue>> m_result;
      };

      /** Stores a store operation. */
      struct StoreOperation {

        /** The values to store. */
        std::vector<IndexedValue> m_values;

        /** Used to indicate the result of the store operation. */
        Routines::Eval<void> m_result;
      };

      /** Stores an open operation. */
      struct OpenOperation {

        /** Used to indicate the result of the open operation. */
        Routines::Eval<void> m_result;
      };

      /** Represents an operation that can be performed on this DataStore. */
      using Operation = std::variant<LoadOperation, StoreOperation,
        OpenOperation>;

      /** Constructs a TestDataStore. */
      TestDataStore() = default;

      ~TestDataStore();

      /** Returns the object publishing Operations. */
      const Publisher<std::shared_ptr<Operation>>& GetOperationPublisher();

      std::vector<SequencedValue> Load(const Query& query);

      void Store(const IndexedValue& value);

      void Store(const std::vector<IndexedValue>& values);

      void Open();

      void Close();

    private:
      IO::OpenState m_openState;
      QueueWriterPublisher<std::shared_ptr<Operation>> m_operationPublisher;

      void Shutdown();
  };

  /**
   * Opens a TestDataStore.
   * @param dataStore - The TestDataStore to open.
   */
  template<typename Q, typename V>
  void Open(TestDataStore<Q, V>& dataStore) {
    auto operations = std::make_shared<
      Queue<std::shared_ptr<typename TestDataStore<Q, V>::Operation>>>();
    dataStore.GetOperationPublisher().Monitor(operations);
    Routines::Spawn(
      [&] {
        while(true) {
          auto operation = operations->Pop();
          if(auto openOperation = std::get_if<
              typename TestDataStore<Q, V>::OpenOperation>(&*operation)) {
            openOperation->m_result.SetResult();
            break;
          }
        }
      });
    dataStore.Open();
  }

  template<typename Q, typename V>
  TestDataStore<Q, V>::~TestDataStore() {
    Close();
  }

  template<typename Q, typename V>
  const Publisher<std::shared_ptr<typename TestDataStore<Q, V>::Operation>>&
      TestDataStore<Q, V>::GetOperationPublisher() {
    return m_operationPublisher;
  }

  template<typename Q, typename V>
  std::vector<typename TestDataStore<Q, V>::SequencedValue>
      TestDataStore<Q, V>::Load(const Query& query) {
    auto async = Routines::Async<std::vector<SequencedValue>>();
    auto operation = std::make_shared<Operation>(
      LoadOperation{query, async.GetEval()});
    m_operationPublisher.Push(operation);
    return async.Get();
  }

  template<typename Q, typename V>
  void TestDataStore<Q, V>::Store(const IndexedValue& value) {
    auto values = std::vector{value};
    Store(values);
  }

  template<typename Q, typename V>
  void TestDataStore<Q, V>::Store(const std::vector<IndexedValue>& values) {
    auto async = Routines::Async<void>();
    auto operation = std::make_shared<Operation>(
      StoreOperation{values, async.GetEval()});
    m_operationPublisher.Push(operation);
    async.Get();
  }

  template<typename Q, typename V>
  void TestDataStore<Q, V>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      auto async = Routines::Async<void>();
      auto operation = std::make_shared<Operation>(
        OpenOperation{async.GetEval()});
      m_operationPublisher.Push(operation);
      async.Get();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename Q, typename V>
  void TestDataStore<Q, V>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename Q, typename V>
  void TestDataStore<Q, V>::Shutdown() {
    m_openState.SetClosed();
  }
}

#endif
