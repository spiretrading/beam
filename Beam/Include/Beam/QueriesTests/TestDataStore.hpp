#ifndef BEAM_TEST_DATA_STORE_HPP
#define BEAM_TEST_DATA_STORE_HPP
#include <memory>
#include <variant>
#include <vector>
#include "Beam/IO/OpenState.hpp"
#include "Beam/Queries/IndexedValue.hpp"
#include "Beam/Queries/SequencedValue.hpp"
#include "Beam/Queues/QueueReader.hpp"
#include "Beam/Queues/QueueWriterPublisher.hpp"
#include "Beam/Routines/Async.hpp"

namespace Beam::Tests {

  /**
   * Implements a DataStore for testing purposes by reifying operations.
   * @tparam Q The type of query used to load values.
   * @tparam V The type value to store.
   */
  template<typename Q, typename V>
  class TestDataStore {
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

      /** Stores a load operation. */
      struct LoadOperation {

        /** The query submitted by the load. */
        Query m_query;

        /** Used to produce the result of the load operation. */
        Eval<std::vector<SequencedValue>> m_result;
      };

      /** Stores a store operation. */
      struct StoreOperation {

        /** The values to store. */
        std::vector<IndexedValue> m_values;

        /** Used to indicate the result of the store operation. */
        Eval<void> m_result;
      };

      /** Represents an operation that can be performed on this DataStore. */
      using Operation = std::variant<LoadOperation, StoreOperation>;

      /** Constructs a TestDataStore. */
      TestDataStore() = default;

      ~TestDataStore();

      /** Returns the object publishing Operations. */
      const Publisher<std::shared_ptr<Operation>>& get_operation_publisher();

      std::vector<SequencedValue> load(const Query& query);
      void store(const IndexedValue& value);
      void store(const std::vector<IndexedValue>& values);
      void close();

    private:
      OpenState m_open_state;
      QueueWriterPublisher<std::shared_ptr<Operation>> m_operation_publisher;

      TestDataStore(const TestDataStore&) = delete;
      TestDataStore& operator =(const TestDataStore&) = delete;
  };

  template<typename Q, typename V>
  TestDataStore<Q, V>::~TestDataStore() {
    close();
  }

  template<typename Q, typename V>
  const Publisher<std::shared_ptr<typename TestDataStore<Q, V>::Operation>>&
      TestDataStore<Q, V>::get_operation_publisher() {
    return m_operation_publisher;
  }

  template<typename Q, typename V>
  std::vector<typename TestDataStore<Q, V>::SequencedValue>
      TestDataStore<Q, V>::load(const Query& query) {
    auto async = Async<std::vector<SequencedValue>>();
    auto operation =
      std::make_shared<Operation>(LoadOperation(query, async.get_eval()));
    m_operation_publisher.push(operation);
    return async.get();
  }

  template<typename Q, typename V>
  void TestDataStore<Q, V>::store(const IndexedValue& value) {
    store(std::vector{value});
  }

  template<typename Q, typename V>
  void TestDataStore<Q, V>::store(const std::vector<IndexedValue>& values) {
    auto async = Async<void>();
    auto operation =
      std::make_shared<Operation>(StoreOperation(values, async.get_eval()));
    m_operation_publisher.push(operation);
    async.get();
  }

  template<typename Q, typename V>
  void TestDataStore<Q, V>::close() {
    m_open_state.close();
  }
}

#endif
