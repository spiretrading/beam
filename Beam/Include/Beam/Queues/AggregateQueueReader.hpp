#ifndef BEAM_AGGREGATE_QUEUE_READER_HPP
#define BEAM_AGGREGATE_QUEUE_READER_HPP
#include <atomic>
#include <vector>
#include "Beam/Queues/Queue.hpp"
#include "Beam/Queues/ScopedQueueReader.hpp"
#include "Beam/Routines/RoutineHandlerGroup.hpp"

namespace Beam {

  /**
   * Combines multiple QueuesReaders together into a single QueueReader.
   * @tparam T The type of data to read from the QueueReader.
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
        std::vector<ScopedQueueReader<Source>> queues);

      ~AggregateQueueReader() override;

      Source pop() override;
      boost::optional<Source> try_pop() override;
      void close(const std::exception_ptr& e) override;
      using QueueReader<T>::close;

    private:
      std::vector<ScopedQueueReader<Source>> m_queues;
      std::atomic_int m_queue_count;
      Queue<Source> m_destination;
      RoutineHandlerGroup m_routines;
  };

  template<typename T>
  AggregateQueueReader(std::vector<ScopedQueueReader<T>>) ->
    AggregateQueueReader<T>;

  /**
   * Combines multiple QueueReaders into a single QueueReader.
   * @param queues The QueueReaders to aggregate.
   */
  template<typename T>
  auto aggregagte(std::vector<ScopedQueueReader<T>> queues) {
    return std::make_shared<AggregateQueueReader<T>>(std::move(queues));
  }

  template<typename T>
  AggregateQueueReader<T>::AggregateQueueReader(
      std::vector<ScopedQueueReader<Source>> queues)
      : m_queues(std::move(queues)),
        m_queue_count(static_cast<int>(m_queues.size())) {
    if(m_queue_count == 0) {
      close();
    } else {
      for(auto& queue : m_queues) {
        m_routines.spawn([this, queue = &queue] {
          for_each(queue, [this] (auto&& value) {
            m_destination.push(std::forward<decltype(value)>(value));
          }, [this] (const std::exception_ptr& e) {
            if(m_queue_count.fetch_sub(1) == 1) {
              m_destination.close(e);
            }
          });
        });
      }
    }
  }

  template<typename T>
  AggregateQueueReader<T>::~AggregateQueueReader() {
    close();
  }

  template<typename T>
  typename AggregateQueueReader<T>::Source AggregateQueueReader<T>::pop() {
    return m_destination.pop();
  }

  template<typename T>
  boost::optional<typename AggregateQueueReader<T>::Source>
      AggregateQueueReader<T>::try_pop() {
    return m_destination.try_pop();
  }

  template<typename T>
  void AggregateQueueReader<T>::close(const std::exception_ptr& e) {
    for(auto& queue : m_queues) {
      queue.close(e);
    }
    m_destination.close(e);
  }
}

#endif
