#ifndef BEAM_AGGREGATE_QUEUE_READER_HPP
#define BEAM_AGGREGATE_QUEUE_READER_HPP
#include <utility>
#include <vector>
#include "Beam/Queues/QueueReader.hpp"
#include "Beam/Queues/Queues.hpp"

namespace Beam {

  /**
   * Combines multiple QueuesReaders together into a single QueueReader.
   * @param <T> The data to read from the QueueReader.
   */
  template<typename T>
  class AggregateQueueReader : public QueueReader<T> {
    public:
      using Target = typename QueueReader<T>::Target;

      /**
       * Constructs an AggregateQueueReader.
       * @param queues The QueueReaders to aggregate.
       */
      AggregateQueueReader(
        std::vector<std::shared_ptr<QueueReader<Target>>> queues);

      ~AggregateQueueReader() override;

      bool IsEmpty() const override;

      Target Pop() override;

      void Break(const std::exception_ptr& e) override;

      bool IsAvailable() const override;

      void SetAvailableToken(
        Threading::Waitable::AvailableToken& token) override;

    private:
      std::vector<std::shared_ptr<QueueReader<T>>> m_queues;
  };

  template<typename T>
  AggregateQueueReader<T>::AggregateQueueReader(
    std::vector<std::shared_ptr<QueueReader<Target>>> queues)
    : m_queues(std::move(queues)) {}

  template<typename T>
  AggregateQueueReader<T>::~AggregateQueueReader() {
    Break();
  }

  template<typename T>
  bool AggregateQueueReader<T>::IsEmpty() const {
    for(auto& queue : m_queues) {
      if(!queue->IsEmpty()) {
        return false;
      }
    }
    return true;
  }

  template<typename T>
  typename AggregateQueueReader<T>::Target AggregateQueueReader<T>::Pop() {
    auto queue = Threading::Wait(m_queues);
    return queue->Pop();
  }

  template<typename T>
  void AggregateQueueReader<T>::Break(const std::exception_ptr& e) {
    for(auto& queue : m_queues) {
      queue->Break(e);
    }
  }

  template<typename T>
  bool AggregateQueueReader<T>::IsAvailable() const {
    for(auto& queue : m_queues) {
      if(queue->IsAvailable()) {
        return true;
      }
    }
    return false;
  }

  template<typename T>
  void AggregateQueueReader<T>::SetAvailableToken(
      Threading::Waitable::AvailableToken& token) {
    for(auto& queue : m_queues) {
      queue->SetAvailableToken(token);
    }
  }
}

#endif
