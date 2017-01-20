#ifndef BEAM_STATEQUEUE_HPP
#define BEAM_STATEQUEUE_HPP
#include "Beam/Queues/AbstractQueue.hpp"
#include "Beam/Queues/PipeBrokenException.hpp"
#include "Beam/Queues/Queues.hpp"

namespace Beam {

  /*! \class StateQueue
      \brief Stores only the most recent value pushed onto it.
      \tparam T The data to store.
   */
  template<typename T>
  class StateQueue : public AbstractQueue<T> {
    public:
      using Source = T;
      using Target = T;

      //! Constructs a StateQueue.
      StateQueue() = default;

      virtual ~StateQueue();

      virtual bool IsEmpty() const;

      virtual void Wait() const;

      virtual T Top() const;

      virtual void Push(const T& value);

      virtual void Push(T&& value);

      virtual void Break(const std::exception_ptr& exception);

      virtual void Pop();

      //! For internal use by other Queues only.
      virtual bool IsAvailable() const;

      using QueueWriter<T>::Break;
      using Threading::Waitable::Wait;
    private:
      DelayPtr<T> m_value;
      std::exception_ptr m_breakException;
  };

  template<typename T>
  StateQueue<T>::~StateQueue() {
    Break();
  }

  template<typename T>
  bool StateQueue<T>::IsEmpty() const {
    boost::lock_guard<boost::mutex> lock(this->GetMutex());
    return !m_value.IsInitialized();
  }

  template<typename T>
  void StateQueue<T>::Wait() const {
    boost::unique_lock<boost::mutex> lock(this->GetMutex());
    this->Wait(lock);
  }

  template<typename T>
  T StateQueue<T>::Top() const {
    boost::unique_lock<boost::mutex> lock(this->GetMutex());
    this->Wait(lock);
    if(!m_value.IsInitialized()) {
      std::rethrow_exception(m_breakException);
    }
    return *m_value;
  }

  template<typename T>
  void StateQueue<T>::Push(const T& value) {
    boost::lock_guard<boost::mutex> lock(this->GetMutex());
    if(m_breakException != nullptr) {
      std::rethrow_exception(m_breakException);
    }
    if(m_value.IsInitialized()) {
      *m_value = value;
    } else {
      m_value.Initialize(value);
      this->NotifyOne();
    }
  }

  template<typename T>
  void StateQueue<T>::Push(T&& value) {
    boost::lock_guard<boost::mutex> lock(this->GetMutex());
    if(m_breakException != nullptr) {
      std::rethrow_exception(m_breakException);
    }
    if(m_value.IsInitialized()) {
      *m_value = std::move(value);
    } else {
      m_value.Initialize(std::move(value));
      this->NotifyOne();
    }
  }

  template<typename T>
  void StateQueue<T>::Break(const std::exception_ptr& exception) {
    boost::lock_guard<boost::mutex> lock(this->GetMutex());
    if(m_breakException != nullptr) {
      return;
    }
    m_breakException = exception;
    this->NotifyAll();
  }

  template<typename T>
  void StateQueue<T>::Pop() {
    boost::lock_guard<boost::mutex> lock(this->GetMutex());
    m_value.Reset();
  }

  template<typename T>
  bool StateQueue<T>::IsAvailable() const {
    return m_value.IsInitialized() || m_breakException != nullptr;
  }
}

#endif
