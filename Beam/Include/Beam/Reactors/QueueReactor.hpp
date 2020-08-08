#ifndef BEAM_QUEUE_REACTOR_HPP
#define BEAM_QUEUE_REACTOR_HPP
#include <memory>
#include <Aspen/Queue.hpp>
#include "Beam/Queues/ScopedQueueReader.hpp"
#include "Beam/Routines/RoutineHandler.hpp"

namespace Beam::Reactors {

  /**
   * Evaluates to values published by a Queue.
   * @param <T> The type of values to publish.
   */
  template<typename T>
  class QueueReactor {
    public:
      using Type = T;

      //! Constructs an empty QueueReactor.
      /*!
        \param queue The Queue to monitor.
      */
      QueueReactor(ScopedQueueReader<Type> queue);

      QueueReactor(QueueReactor&&) = default;

      ~QueueReactor();

      Aspen::State commit(int sequence) noexcept;

      Aspen::eval_result_t<Type> eval() const;

      QueueReactor& operator =(QueueReactor&&) = default;

    private:
      struct Entry {
        Aspen::Queue<Type> m_reactor;
        Routines::RoutineHandler m_handler;
        ScopedQueueReader<Type> m_queue;
        bool m_isComplete;

        Entry(ScopedQueueReader<Type> queue);
      };
      std::unique_ptr<Entry> m_entry;

      static void MonitorQueue(Entry& entry);
  };

  template<typename T>
  QueueReactor(ScopedQueueReader<T>) -> QueueReactor<T>;

  template<typename T>
  QueueReactor(T&&) -> QueueReactor<typename GetTryDereferenceType<T>::Source>;

  template<typename T>
  QueueReactor<T>::Entry::Entry(ScopedQueueReader<Type> queue)
    : m_queue(std::move(queue)),
      m_isComplete(false) {}

  template<typename T>
  QueueReactor<T>::QueueReactor(ScopedQueueReader<Type> queue)
      : m_entry(std::make_unique<Entry>(std::move(queue))) {
    m_entry->m_handler = Routines::Spawn(
      [entry = m_entry.get()] {
        MonitorQueue(*entry);
      });
  }

  template<typename T>
  QueueReactor<T>::~QueueReactor() {
    if(m_entry != nullptr) {
      m_entry->m_isComplete = true;
      m_entry->m_queue.Break();
      m_entry->m_handler.Wait();
    }
  }

  template<typename T>
  Aspen::State QueueReactor<T>::commit(int sequence) noexcept {
    return m_entry->m_reactor.commit(sequence);
  }

  template<typename T>
  Aspen::eval_result_t<typename QueueReactor<T>::Type>
      QueueReactor<T>::eval() const {
    return m_entry->m_reactor.eval();
  }

  template<typename T>
  void QueueReactor<T>::MonitorQueue(Entry& entry) {
    while(true) {
      try {
        entry.m_reactor.push(entry.m_queue.Pop());
      } catch(const PipeBrokenException&) {
        if(!entry.m_isComplete) {
          entry.m_reactor.set_complete();
        }
        break;
      } catch(const std::exception&) {
        if(!entry.m_isComplete) {
          entry.m_reactor.set_complete(std::current_exception());
        }
        break;
      }
    }
  }
}

#endif
