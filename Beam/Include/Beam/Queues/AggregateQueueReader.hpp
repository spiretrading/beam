#ifndef BEAM_AGGREGATE_QUEUE_READER_HPP
#define BEAM_AGGREGATE_QUEUE_READER_HPP
#include <atomic>
#include <utility>
#include <vector>
#include "Beam/Queues/Queue.hpp"
#include "Beam/Queues/QueueReader.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Routines/RoutineHandlerGroup.hpp"

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
       * @param sources The QueueReaders to aggregate.
       */
      AggregateQueueReader(
        std::vector<std::shared_ptr<QueueReader<Target>>> sources);

      bool IsEmpty() const override;

      Target Top() const override;

      void Pop() override;

      void Break(const std::exception_ptr& e) override;

    protected:
      bool IsAvailable() const override;

    private:
      std::vector<std::shared_ptr<QueueReader<T>>> m_sources;
      Queue<T> m_destination;
      std::atomic_int m_queueCount;
      Routines::RoutineHandlerGroup m_routines;
  };

  template<typename T>
  AggregateQueueReader<T>::AggregateQueueReader(
      std::vector<std::shared_ptr<QueueReader<Target>>> sources)
      : m_sources(std::move(sources)),
        m_queueCount(static_cast<int>(queues.size())) {
    for(auto& source : m_sources) {
      m_routines.Spawn(
        [=] {
          try {
            while(true) {
              auto value = source->Top();
              source->Pop();
              m_destination.Push(std::move(value));
            }
          } catch(const std::exception&) {
            if(--m_queueCount == 1) {
              m_destination.Break();
            }
          }
        });
    }
  }

  template<typename T>
  bool AggregateQueueReader<T>::IsEmpty() const {
    return m_destination.IsEmpty();
  }

  template<typename T>
  typename AggregateQueueReader<T>::Target
      AggregateQueueReader<T>::Top() const {
    return m_destination.Top();
  }

  template<typename T>
  void AggregateQueueReader<T>::Pop() {
    m_destination.Pop();
  }

  template<typename T>
  void AggregateQueueReader<T>::Break(const std::exception_ptr& e) {
    for(auto& source : m_sources) {
      source->Break(e);
    }
    m_destination.Break(e);
  }

  template<typename T>
  bool AggregateQueueReader<T>::IsAvailable() const {
    return m_destination.IsAvailable();
  }
}

#endif
