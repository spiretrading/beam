#ifndef BEAM_QUEUE_HPP
#define BEAM_QUEUE_HPP
#include <condition_variable>
#include <deque>
#include <mutex>
#include "Beam/Queues/AbstractQueue.hpp"
#include "Beam/Queues/PipeBrokenException.hpp"
#include "Beam/Queues/Queues.hpp"

namespace Beam {

  /**
   * Implements a Queue that can safely block within a Routine waiting for data
   * to arrive.
   * @param <T> The data to store in the Queue.
   */
  template<typename T>
  class Queue : public AbstractQueue<T> {
    public:
      using Target = typename AbstractQueue<T>::Target;
      using Source = typename AbstractQueue<T>::Source;

      /** Constructs a Queue. */
      Queue() = default;

      /** Returns <code>true</code> iff this Queue is broken. */
      bool IsBroken() const;

      Source Pop() override;

      boost::optional<Source> TryPop() override;

      void Push(const Target& value) override;

      void Push(Target&& value) override;

      void Break(const std::exception_ptr& exception) override;

      using QueueWriter<T>::Break;

    private:
      mutable std::mutex m_mutex;
      mutable std::condition_variable m_isAvailableCondition;
      std::deque<T> m_queue;
      std::exception_ptr m_breakException;

      bool UnlockedIsAvailable() const;
  };

  template<typename T>
  bool Queue<T>::IsBroken() const {
    auto lock = std::lock_guard(m_mutex);
    return m_breakException != nullptr && m_queue.empty();
  }

  template<typename T>
  typename Queue<T>::Source Queue<T>::Pop() {
    auto lock = std::unique_lock(m_mutex);
    while(!UnlockedIsAvailable()) {
      m_isAvailableCondition.wait(lock);
    }
    if(m_queue.empty()) {
      std::rethrow_exception(m_breakException);
    }
    auto value = std::move(m_queue.front());
    m_queue.pop_front();
    return value;
  }

  template<typename T>
  boost::optional<typename Queue<T>::Source> Queue<T>::TryPop() {
    auto lock = std::lock_guard(m_mutex);
    if(m_queue.empty()) {
      return boost::none;
    }
    auto value = std::move(m_queue.front());
    m_queue.pop_front();
    return value;
  }

  template<typename T>
  void Queue<T>::Push(const Target& value) {
    auto lock = std::lock_guard(m_mutex);
    if(m_breakException != nullptr) {
      std::rethrow_exception(m_breakException);
    }
    m_queue.push_back(value);
    if(m_queue.size() == 1) {
      m_isAvailableCondition.notify_one();
    }
  }

  template<typename T>
  void Queue<T>::Push(Target&& value) {
    auto lock = std::lock_guard(m_mutex);
    if(m_breakException != nullptr) {
      std::rethrow_exception(m_breakException);
    }
    m_queue.push_back(std::move(value));
    if(m_queue.size() == 1) {
      m_isAvailableCondition.notify_one();
    }
  }

  template<typename T>
  void Queue<T>::Break(const std::exception_ptr& exception) {
    auto lock = std::lock_guard(m_mutex);
    if(m_breakException != nullptr) {
      return;
    }
    m_breakException = exception;
    m_isAvailableCondition.notify_all();
  }

  template<typename T>
  bool Queue<T>::UnlockedIsAvailable() const {
    return !m_queue.empty() || m_breakException;
  }
}

#endif
