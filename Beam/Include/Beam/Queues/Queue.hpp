#ifndef BEAM_QUEUE_HPP
#define BEAM_QUEUE_HPP
#include <deque>
#include <boost/thread/mutex.hpp>
#include "Beam/Queues/AbstractQueue.hpp"
#include "Beam/Queues/PipeBrokenException.hpp"
#include "Beam/Threading/ConditionVariable.hpp"

namespace Beam {

  /**
   * Implements a Queue that can safely block within a Routine waiting for data
   * to arrive.
   * @tparam T The data to store in the Queue.
   */
  template<typename T>
  class Queue : public AbstractQueue<T> {
    public:
      using Target = typename AbstractQueue<T>::Target;
      using Source = typename AbstractQueue<T>::Source;

      /** Constructs a Queue. */
      Queue() = default;

      /** Returns <code>true</code> iff this Queue is broken. */
      bool is_broken() const;

      Source pop() override;
      boost::optional<Source> try_pop() override;
      void push(const Target& value) override;
      void push(Target&& value) override;
      void close(const std::exception_ptr& exception) override;
      using QueueWriter<T>::close;

    private:
      mutable boost::mutex m_mutex;
      mutable ConditionVariable m_is_available_condition;
      std::deque<T> m_queue;
      std::exception_ptr m_break_exception;

      bool unlocked_is_available() const;
  };

  template<typename T>
  bool Queue<T>::is_broken() const {
    auto lock = boost::lock_guard(m_mutex);
    return m_break_exception && m_queue.empty();
  }

  template<typename T>
  typename Queue<T>::Source Queue<T>::pop() {
    auto lock = boost::unique_lock(m_mutex);
    while(!unlocked_is_available()) {
      m_is_available_condition.wait(lock);
    }
    if(m_queue.empty()) {
      std::rethrow_exception(m_break_exception);
    }
    auto value = std::move(m_queue.front());
    m_queue.pop_front();
    return value;
  }

  template<typename T>
  boost::optional<typename Queue<T>::Source> Queue<T>::try_pop() {
    auto lock = boost::lock_guard(m_mutex);
    if(m_queue.empty()) {
      return boost::none;
    }
    auto value = std::move(m_queue.front());
    m_queue.pop_front();
    return value;
  }

  template<typename T>
  void Queue<T>::push(const Target& value) {
    auto lock = boost::lock_guard(m_mutex);
    if(m_break_exception) {
      std::rethrow_exception(m_break_exception);
    }
    m_queue.push_back(value);
    if(m_queue.size() == 1) {
      m_is_available_condition.notify_one();
    }
  }

  template<typename T>
  void Queue<T>::push(Target&& value) {
    auto lock = boost::lock_guard(m_mutex);
    if(m_break_exception) {
      std::rethrow_exception(m_break_exception);
    }
    m_queue.push_back(std::move(value));
    if(m_queue.size() == 1) {
      m_is_available_condition.notify_one();
    }
  }

  template<typename T>
  void Queue<T>::close(const std::exception_ptr& exception) {
    auto lock = boost::lock_guard(m_mutex);
    if(m_break_exception) {
      return;
    }
    m_break_exception = exception;
    m_is_available_condition.notify_all();
  }

  template<typename T>
  bool Queue<T>::unlocked_is_available() const {
    return !m_queue.empty() || m_break_exception;
  }
}

#endif
