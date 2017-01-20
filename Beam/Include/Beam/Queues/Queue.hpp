#ifndef BEAM_QUEUE_HPP
#define BEAM_QUEUE_HPP
#include <deque>
#include "Beam/Queues/AbstractQueue.hpp"
#include "Beam/Queues/PipeBrokenException.hpp"
#include "Beam/Queues/Queues.hpp"

namespace Beam {

  /*! \class Queue
      \brief Implements a Queue that can safely block within a Routine waiting
             for data to arrive.
      \tparam T The data to store in the Queue.
   */
  template<typename T>
  class Queue : public AbstractQueue<T> {
    public:
      using Source = T;
      using Target = T;

      //! Constructs a Queue.
      Queue() = default;

      virtual ~Queue();

      bool IsBroken() const;

      virtual bool IsEmpty() const;

      virtual void Wait() const;

      virtual T Top() const;

      virtual bool TryEmplace(Out<T> value);

      virtual void Emplace(Out<T> value);

      virtual void Push(const T& value);

      virtual void Push(T&& value);

      virtual void Break(const std::exception_ptr& exception);

      virtual void Pop();

      //! For internal use by other Queues only.
      virtual bool IsAvailable() const;

      using QueueWriter<T>::Break;
      using Threading::Waitable::Wait;
    private:
      std::deque<T> m_queue;
      std::exception_ptr m_breakException;
  };

  template<typename T>
  Queue<T>::~Queue() {
    Break();
  }

  template<typename T>
  bool Queue<T>::IsBroken() const {
    boost::lock_guard<boost::mutex> lock{this->GetMutex()};
    return m_breakException != nullptr && m_queue.empty();
  }

  template<typename T>
  bool Queue<T>::IsEmpty() const {
    boost::lock_guard<boost::mutex> lock{this->GetMutex()};
    return m_queue.empty();
  }

  template<typename T>
  void Queue<T>::Wait() const {
    boost::unique_lock<boost::mutex> lock{this->GetMutex()};
    this->Wait(lock);
  }

  template<typename T>
  T Queue<T>::Top() const {
    boost::unique_lock<boost::mutex> lock{this->GetMutex()};
    this->Wait(lock);
    if(m_queue.empty()) {
      std::rethrow_exception(m_breakException);
    }
    return m_queue.front();
  }

  template<typename T>
  bool Queue<T>::TryEmplace(Out<T> value) {
    boost::unique_lock<boost::mutex> lock{this->GetMutex()};
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
  void Queue<T>::Emplace(Out<T> value) {
    boost::unique_lock<boost::mutex> lock{this->GetMutex()};
    this->Wait(lock);
    if(m_queue.empty()) {
      std::rethrow_exception(m_breakException);
    }
    *value = std::move(m_queue.front());
    m_queue.pop_front();
  }

  template<typename T>
  void Queue<T>::Push(const T& value) {
    boost::lock_guard<boost::mutex> lock{this->GetMutex()};
    if(m_breakException != nullptr) {
      std::rethrow_exception(m_breakException);
    }
    m_queue.push_back(value);
    if(m_queue.size() == 1) {
      this->NotifyOne();
    }
  }

  template<typename T>
  void Queue<T>::Push(T&& value) {
    boost::lock_guard<boost::mutex> lock{this->GetMutex()};
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
    boost::lock_guard<boost::mutex> lock{this->GetMutex()};
    if(m_breakException != nullptr) {
      return;
    }
    m_breakException = exception;
    this->NotifyAll();
  }

  template<typename T>
  void Queue<T>::Pop() {
    boost::lock_guard<boost::mutex> lock{this->GetMutex()};
    m_queue.pop_front();
  }

  template<typename T>
  bool Queue<T>::IsAvailable() const {
    return !m_queue.empty() || m_breakException != nullptr;
  }
}

#endif
