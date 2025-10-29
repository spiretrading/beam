#ifndef BEAM_QUEUE_REACTOR_HPP
#define BEAM_QUEUE_REACTOR_HPP
#include <exception>
#include <memory>
#include <Aspen/Queue.hpp>
#include "Beam/Queues/ScopedQueueReader.hpp"
#include "Beam/Routines/RoutineHandler.hpp"

namespace Beam {

  /**
   * Evaluates to values published by a Queue.
   * @tparam T The type of values to publish.
   */
  template<typename T>
  class QueueReactor {
    public:

      /** The type of values being published. */
      using Type = T;

      /**
       * Constructs an empty QueueReactor.
       * @param queue The Queue to monitor.
       */
      explicit QueueReactor(ScopedQueueReader<Type> queue);

      QueueReactor(QueueReactor&&) = default;
      ~QueueReactor();

      Aspen::State commit(int sequence) noexcept;
      Aspen::eval_result_t<Type> eval() const;
      QueueReactor& operator =(QueueReactor&&) = default;

    private:
      struct Entry {
        Aspen::Queue<Type> m_reactor;
        RoutineHandler m_handler;
        ScopedQueueReader<Type> m_queue;
        bool m_is_complete;

        Entry(ScopedQueueReader<Type> queue);
      };
      std::unique_ptr<Entry> m_entry;

      static void monitor(Entry& entry);
  };

  template<typename T>
  QueueReactor(ScopedQueueReader<T>) -> QueueReactor<T>;

  template<typename T>
  QueueReactor(T&&) -> QueueReactor<typename dereference_t<T>::Source>;

  template<typename T>
  QueueReactor<T>::Entry::Entry(ScopedQueueReader<Type> queue)
    : m_queue(std::move(queue)),
      m_is_complete(false) {}

  template<typename T>
  QueueReactor<T>::QueueReactor(ScopedQueueReader<Type> queue)
      : m_entry(std::make_unique<Entry>(std::move(queue))) {
    m_entry->m_handler = Beam::spawn([entry = m_entry.get()] {
      monitor(*entry);
    });
  }

  template<typename T>
  QueueReactor<T>::~QueueReactor() {
    if(m_entry) {
      m_entry->m_is_complete = true;
      m_entry->m_queue.close();
      m_entry->m_handler.wait();
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
  void QueueReactor<T>::monitor(Entry& entry) {
    while(true) {
      try {
        entry.m_reactor.push(entry.m_queue.pop());
      } catch(const PipeBrokenException&) {
        if(!entry.m_is_complete) {
          entry.m_reactor.set_complete();
        }
        break;
      } catch(const std::exception&) {
        if(!entry.m_is_complete) {
          entry.m_reactor.set_complete(std::current_exception());
        }
        break;
      }
    }
  }
}

#endif
