#ifndef BEAM_AGGREGATE_QUEUE_READER_HPP
#define BEAM_AGGREGATE_QUEUE_READER_HPP
#include <atomic>
#include <utility>
#include <vector>
#include "Beam/Queues/Queue.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Routines/RoutineHandlerGroup.hpp"

namespace Beam {

  /**
   * Combines multiple QueuesReaders together into a single QueueReader.
   * @param <T> The type of data to read from the QueueReader.
   */
  template<typename T>
  class AggregateQueueReader : public QueueReader<T> {
    public:
      using Target = typename QueueReader<T>::Target;

      /**
       * Constructs an AggregateQueueReader.
       * @param queues The QueueReaders to aggregate.
       */
      explicit AggregateQueueReader(
        std::vector<std::shared_ptr<QueueReader<T>>> queues);

      ~AggregateQueueReader() override;

      bool IsEmpty() const override;

      Target Pop() override;

      void Break(const std::exception_ptr& e) override;

      using QueueReader<T>::Break;

    private:
      std::vector<std::shared_ptr<QueueReader<T>>> m_queues;
      Queue<T> m_destination;
      std::atomic_int m_queueCount;
      Routines::RoutineHandlerGroup m_routines;
  };

  template<typename T>
  AggregateQueueReader<T>::AggregateQueueReader(
      std::vector<std::shared_ptr<QueueReader<T>>> queues)
      : m_queues(std::move(queues)),
        m_queueCount(static_cast<int>(m_queues.size())) {
    if(m_queueCount == 0) {
      Break();
    } else {
      for(auto& queue : m_queues) {
        m_routines.Spawn(
          [=] {
            try {
              while(true) {
                m_destination.Push(queue->Pop());
              }
            } catch(const std::exception&) {
              if(m_queueCount.fetch_sub(1) == 1) {
                m_destination.Break();
              }
            }
          });
      }
    }
  }

  template<typename T>
  AggregateQueueReader<T>::~AggregateQueueReader() {
    Break();
  }

  template<typename T>
  bool AggregateQueueReader<T>::IsEmpty() const {
    return m_destination.IsEmpty();
  }

  template<typename T>
  typename AggregateQueueReader<T>::Target AggregateQueueReader<T>::Pop() {
    return m_destination.Pop();
  }

  template<typename T>
  void AggregateQueueReader<T>::Break(const std::exception_ptr& e) {
    for(auto& queue : m_queues) {
      queue->Break(e);
    }
    m_destination.Break(e);
  }
}

#endif
