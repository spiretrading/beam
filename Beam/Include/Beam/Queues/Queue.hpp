#ifndef BEAM_QUEUE_HPP
#define BEAM_QUEUE_HPP
#include <deque>
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
      using Source = typename AbstractQueue<T>::Source;
      using Target = typename AbstractQueue<T>::Target;

      /** Constructs a Queue. */
      Queue() = default;

      /** Returns <code>true</code> iff this Queue is broken. */
      bool IsBroken() const;

      /**
       * Directly emplaces a value and pops it off the stack without blocking.
       * @param value Stores the popped value.
       */
      bool TryEmplace(Out<Target> value);

      /**
       * Directly emplaces a value and pops it off the stack.
       * @param value Stores the popped value.
       */
      void Emplace(Out<Target> value);

      bool IsEmpty() const override;

      Target Top() const override;

      void Push(const Source& value) override;

      void Push(Source&& value) override;

      void Break(const std::exception_ptr& exception) override;

      void Pop() override;

      /** For internal use by other Queues only. */
      bool IsAvailable() const override;

      using QueueWriter<T>::Break;
      using QueueReader<T>::Wait;
      using Threading::Waitable::Wait;
    private:
      std::deque<T> m_queue;
      std::exception_ptr m_breakException;
  };

  template<typename T>
  bool Queue<T>::IsBroken() const {
    auto lock = boost::lock_guard(this->GetMutex());
    return m_breakException != nullptr && m_queue.empty();
  }

  template<typename T>
  bool Queue<T>::TryEmplace(Out<Target> value) {
    auto lock = boost::unique_lock(this->GetMutex());
    if(m_queue.empty()) {
      if(m_breakException == nullptr) {
        return false;
      }
      std::rethrow_exception(m_breakException);
    }
    *value = std::move(m_queue.front());
    m_queue.pop_front();
    return true;
  }

  template<typename T>
  void Queue<T>::Emplace(Out<Target> value) {
    auto lock = boost::unique_lock(this->GetMutex());
    this->Wait(lock);
    if(m_queue.empty()) {
      std::rethrow_exception(m_breakException);
    }
    *value = std::move(m_queue.front());
    m_queue.pop_front();
  }

  template<typename T>
  bool Queue<T>::IsEmpty() const {
    auto lock = boost::lock_guard(this->GetMutex());
    return m_queue.empty();
  }

  template<typename T>
  typename Queue<T>::Target Queue<T>::Top() const {
    auto lock = boost::unique_lock(this->GetMutex());
    this->Wait(lock);
    if(m_queue.empty()) {
      std::rethrow_exception(m_breakException);
    }
    return m_queue.front();
  }

  template<typename T>
  void Queue<T>::Push(const Source& value) {
    auto lock = boost::lock_guard(this->GetMutex());
    if(m_breakException != nullptr) {
      std::rethrow_exception(m_breakException);
    }
    m_queue.push_back(value);
    if(m_queue.size() == 1) {
      this->NotifyOne();
    }
  }

  template<typename T>
  void Queue<T>::Push(Source&& value) {
    auto lock = boost::lock_guard(this->GetMutex());
    if(m_breakException != nullptr) {
      std::rethrow_exception(m_breakException);
    }
    m_queue.push_back(std::move(value));
    if(m_queue.size() == 1) {
      this->NotifyOne();
    }
  }

  template<typename T>
  void Queue<T>::Break(const std::exception_ptr& exception) {
    auto lock = boost::lock_guard(this->GetMutex());
    if(m_breakException != nullptr) {
      return;
    }
    m_breakException = exception;
    this->NotifyAll();
  }

  template<typename T>
  void Queue<T>::Pop() {
    auto lock = boost::lock_guard(this->GetMutex());
    m_queue.pop_front();
  }

  template<typename T>
  bool Queue<T>::IsAvailable() const {
    return !m_queue.empty() || m_breakException != nullptr;
  }
}

#endif
