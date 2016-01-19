#ifndef BEAM_MULTIQUEUEWRITER_HPP
#define BEAM_MULTIQUEUEWRITER_HPP
#include <memory>
#include <vector>
#include <boost/thread/locks.hpp>
#include "Beam/Queues/PipeBrokenException.hpp"
#include "Beam/Queues/Publisher.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/QueueWriter.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"

namespace Beam {

  /*! \class MultiQueueWriter
      \brief Used to write data to multiple Queues simultaneously.
      \tparam T The data to store in the Queue.
   */
  template<typename T>
  class MultiQueueWriter : public QueueWriter<T>, public Publisher<T> {
    public:
      using Source = T;

      //! Constructs a MultiQueueWriter.
      MultiQueueWriter() = default;

      ~MultiQueueWriter();

      virtual void Lock() const;

      virtual void Unlock() const;

      virtual void With(const std::function<void ()>& f) const;

      virtual void Push(const T& value);

      virtual void Push(T&& value);

      virtual void Break(const std::exception_ptr& e);

      virtual void Monitor(std::shared_ptr<QueueWriter<T>> queue) const;

      using QueueWriter<T>::Break;
    private:
      mutable Threading::RecursiveMutex m_mutex;
      std::exception_ptr m_exception;
      mutable std::vector<std::weak_ptr<QueueWriter<T>>> m_queues;
  };

  template<typename T>
  MultiQueueWriter<T>::~MultiQueueWriter() {
    Break();
  }

  template<typename T>
  void MultiQueueWriter<T>::Lock() const {
    m_mutex.lock();
  }

  template<typename T>
  void MultiQueueWriter<T>::Unlock() const {
    m_mutex.unlock();
  }

  template<typename T>
  void MultiQueueWriter<T>::With(const std::function<void ()>& f) const {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    f();
  }

  template<typename T>
  void MultiQueueWriter<T>::Push(const T& value) {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    auto i = m_queues.begin();
    while(i != m_queues.end()) {
      auto queue(i->lock());
      if(queue != nullptr) {
        try {
          queue->Push(value);
          ++i;
        } catch(const PipeBrokenException&) {
          i = m_queues.erase(i);
        }
      } else {
        i = m_queues.erase(i);
      }
    }
  }

  template<typename T>
  void MultiQueueWriter<T>::Push(T&& value) {
    Push(static_cast<const T&>(value));
  }

  template<typename T>
  void MultiQueueWriter<T>::Break(const std::exception_ptr& e) {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    m_exception = e;
    for(auto& i : m_queues) {
      auto queue = i.lock();
      if(queue != nullptr) {
        queue->Break(e);
      }
    }
    m_queues.clear();
  }

  template<typename T>
  void MultiQueueWriter<T>::Monitor(
      std::shared_ptr<QueueWriter<T>> queue) const {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    if(m_exception == nullptr) {
      m_queues.push_back(queue);
    } else {
      queue->Break(m_exception);
    }
  }
}

#endif
