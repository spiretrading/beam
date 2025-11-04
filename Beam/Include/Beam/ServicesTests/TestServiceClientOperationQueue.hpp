#ifndef BEAM_TEST_SERVICE_CLIENT_OPERATION_QUEUE_HPP
#define BEAM_TEST_SERVICE_CLIENT_OPERATION_QUEUE_HPP
#include <concepts>
#include <memory>
#include <variant>
#include <boost/throw_exception.hpp>
#include "Beam/Collections/SynchronizedList.hpp"
#include "Beam/Collections/SynchronizedSet.hpp"
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Queues/ScopedQueueWriter.hpp"
#include "Beam/Routines/Async.hpp"
#include "Beam/ServicesTests/ServiceResult.hpp"

namespace Beam::Tests {

  /**
   * Queues operations for test service clients, managing their lifecycle and
   * tracking pending results and queue-based operations.
   * @tparam V The std::variant type that holds all possible operation types.
   */
  template<typename V>
  class TestServiceClientOperationQueue {
    public:

      /** The variant type holding all operations. */
      using Operation = V;

      /**
       * Constructs a TestServiceClientOperationQueue.
       * @param operations The queue to dispatch operations onto.
       */
      explicit TestServiceClientOperationQueue(
        ScopedQueueWriter<std::shared_ptr<Operation>> operations) noexcept;

      ~TestServiceClientOperationQueue();

      /**
       * Pushes an operation onto the dispatcher.
       * @param operation The operation to push.
       */
      void push(const std::shared_ptr<Operation>& operation);

      /**
       * Appends a queue-based operation to the dispatcher.
       * @tparam T The specific operation type from the variant.
       * @param operation The operation to append.
       */
      template<typename T>
      void append_queue(std::shared_ptr<Operation> operation);

      /**
       * Appends a result-based operation to the dispatcher and waits for its
       * completion.
       * @tparam T The specific operation type from the variant.
       * @tparam R The result type.
       * @param args Arguments to construct the operation.
       * @return The result of the operation.
       */
      template<typename T, typename R, typename... Args>
      R append_result(Args&&... args);

      /** Closes the dispatcher and breaks all pending operations. */
      void close();

    private:
      ScopedQueueWriter<std::shared_ptr<Operation>> m_operations;
      SynchronizedVector<std::weak_ptr<BaseQueue>> m_queues;
      SynchronizedUnorderedSet<BaseServiceResult*> m_pending_results;
      OpenState m_open_state;

      TestServiceClientOperationQueue(
        const TestServiceClientOperationQueue&) = delete;
      TestServiceClientOperationQueue& operator =(
        const TestServiceClientOperationQueue&) = delete;
  };

  template<typename V>
  TestServiceClientOperationQueue<V>::TestServiceClientOperationQueue(
    ScopedQueueWriter<std::shared_ptr<Operation>> operations) noexcept
    : m_operations(std::move(operations)) {}

  template<typename V>
  TestServiceClientOperationQueue<V>::~TestServiceClientOperationQueue() {
    close();
  }

  template<typename V>
  void TestServiceClientOperationQueue<V>::push(
      const std::shared_ptr<Operation>& operation) {
    m_operations.push(operation);
  }

  template<typename V>
  template<typename T>
  void TestServiceClientOperationQueue<V>::append_queue(
      std::shared_ptr<Operation> operation) {
    auto queue = std::shared_ptr<BaseQueue>(
      operation, &std::get<T>(*operation).m_queue);
    m_queues.push_back(queue);
    if(!m_open_state.is_open()) {
      m_queues.erase_if([&] (const auto& weak_queue) {
        auto q = weak_queue.lock();
        return !q || q == queue;
      });
      queue->close();
      return;
    }
    m_operations.push(operation);
  }

  template<typename V>
  template<typename T, typename R, typename... Args>
  R TestServiceClientOperationQueue<V>::append_result(Args&&... args) {
    auto async = Async<R>();
    auto operation = std::make_shared<Operation>(
      std::in_place_type<T>, std::forward<Args>(args)..., async.get_eval());
    m_pending_results.insert(&std::get<T>(*operation).m_result);
    if(!m_open_state.is_open()) {
      m_pending_results.erase(&std::get<T>(*operation).m_result);
      boost::throw_with_location(EndOfFileException());
    }
    m_operations.push(operation);
    if constexpr(std::same_as<R, void>) {
      try {
        async.get();
        m_pending_results.erase(&std::get<T>(*operation).m_result);
      } catch(const std::exception&) {
        m_pending_results.erase(&std::get<T>(*operation).m_result);
        throw;
      }
      return;
    } else {
      try {
        auto result = std::move(async.get());
        m_pending_results.erase(&std::get<T>(*operation).m_result);
        return result;
      } catch (const std::exception&) {
        m_pending_results.erase(&std::get<T>(*operation).m_result);
        throw;
      }
    }
  }

  template<typename V>
  void TestServiceClientOperationQueue<V>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_queues.for_each([] (const auto& queue) {
      if(auto q = queue.lock()) {
        q->close();
      }
    });
    m_queues.clear();
    m_pending_results.for_each([] (auto& result) {
      result->set(std::make_exception_ptr(EndOfFileException()));
    });
    m_pending_results.clear();
    m_open_state.close();
  }
}

#endif
