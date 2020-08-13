#ifndef BEAM_AGGREGATE_QUEUE_READER_HPP
#define BEAM_AGGREGATE_QUEUE_READER_HPP
#include <atomic>
#include <utility>
#include <vector>
#include "Beam/Queues/Queue.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/ScopedQueueReader.hpp"
#include "Beam/Routines/RoutineHandlerGroup.hpp"

namespace Beam {

  /**
   * Combines multiple QueuesReaders together into a single QueueReader.
   * @param <T> The type of data to read from the QueueReader.
   */
  template<typename T>
  class AggregateQueueReader : public QueueReader<T> {
    public:
      using Source = typename QueueReader<T>::Source;

      /**
       * Constructs an AggregateQueueReader.
       * @param queues The QueueReaders to aggregate.
       */
      explicit AggregateQueueReader(
        std::vector<ScopedQueueReader<T>> queues);

      ~AggregateQueueReader() override;

      Source Pop() override;

      boost::optional<Source> TryPop() override;

      void Break(const std::exception_ptr& e) override;

      using QueueReader<T>::Break;

    private:
      std::vector<ScopedQueueReader<Source>> m_queues;
      std::atomic_int m_queueCount;
      Queue<Source> m_destination;
      Routines::RoutineHandlerGroup m_routines;
  };

  template<typename T>
  AggregateQueueReader<T>::AggregateQueueReader(
      std::vector<ScopedQueueReader<T>> queues)
      : m_queues(std::move(queues)),
        m_queueCount(static_cast<int>(m_queues.size())) {
    if(m_queueCount == 0) {
      Break();
    } else {
      for(auto& queue : m_queues) {
        m_routines.Spawn(
          [=, queue = &queue] {
            try {
              while(true) {
                m_destination.Push(queue->Pop());
              }
            } catch(const std::exception&) {
              if(m_queueCount.fetch_sub(1) == 1) {
                m_destination.Break(std::current_exception());
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
  typename AggregateQueueReader<T>::Source AggregateQueueReader<T>::Pop() {
    return m_destination.Pop();
  }

  template<typename T>
  boost::optional<typename AggregateQueueReader<T>::Source>
      AggregateQueueReader<T>::TryPop() {
    return m_destination.TryPop();
  }

  template<typename T>
  void AggregateQueueReader<T>::Break(const std::exception_ptr& e) {
    for(auto& queue : m_queues) {
      queue.Break(e);
    }
    m_destination.Break(e);
  }
}

#endif
