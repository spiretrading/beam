#ifndef BEAM_AGGREGATEQUEUE_HPP
#define BEAM_AGGREGATEQUEUE_HPP
#include <atomic>
#include <utility>
#include <vector>
#include <boost/throw_exception.hpp>
#include "Beam/Queues/Queue.hpp"
#include "Beam/Queues/QueueReader.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Routines/RoutineHandlerGroup.hpp"

namespace Beam {

  /*! \class AggregateQueue
      \brief Combines multiple Queues together into a single Queue.
      \tparam T The data to read from the Queue.
   */
  template<typename T>
  class AggregateQueue : public QueueReader<T> {
    public:

      //! The type being read.
      typedef T Target;

      //! Constructs an AggregateQueue.
      /*!
        \param queues The Queues to aggregate.
      */
      AggregateQueue(
        const std::vector<std::shared_ptr<QueueReader<T>>>& queues);

      virtual bool IsEmpty() const;

      virtual Target Top() const;

      virtual void Pop();

      virtual void Break(const std::exception_ptr& e);

    protected:
      virtual bool IsAvailable() const;

    private:
      std::vector<std::shared_ptr<QueueReader<T>>> m_sources;
      std::shared_ptr<Queue<T>> m_destination;
      std::atomic_int m_queueCount;
      Routines::RoutineHandlerGroup m_routines;
  };

  template<typename T>
  AggregateQueue<T>::AggregateQueue(
      const std::vector<std::shared_ptr<QueueReader<T>>>& queues)
      : m_sources(queues),
        m_destination(std::make_shared<Queue<T>>()),
        m_queueCount(static_cast<int>(queues.size())) {
    for(const auto& source : m_sources) {
      m_routines.Spawn(
        [=] {
          try {
            while(true) {
              auto value = source->Top();
              source->Pop();
              m_destination->Push(std::move(value));
            }
          } catch(const std::exception&) {
            if(--m_queueCount == 1) {
              m_destination->Break();
            }
          }
        });
    }
  }

  template<typename T>
  bool AggregateQueue<T>::IsEmpty() const {
    return m_destination->IsEmpty();
  }

  template<typename T>
  typename AggregateQueue<T>::Target AggregateQueue<T>::Top() const {
    return m_destination->Top();
  }

  template<typename T>
  void AggregateQueue<T>::Pop() {
    m_destination->Pop();
  }

  template<typename T>
  void AggregateQueue<T>::Break(const std::exception_ptr& e) {
    for(const auto& source : m_sources) {
      source->Break(e);
    }
    m_destination->Break(e);
  }

  template<typename T>
  bool AggregateQueue<T>::IsAvailable() const {
    return m_destination->IsAvailable();
  }
}

#endif
