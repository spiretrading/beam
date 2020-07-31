#ifndef BEAM_MULTI_QUEUE_WRITER_HPP
#define BEAM_MULTI_QUEUE_WRITER_HPP
#include <memory>
#include <vector>
#include <boost/thread/locks.hpp>
#include "Beam/Queues/PipeBrokenException.hpp"
#include "Beam/Queues/Publisher.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/QueueWriter.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"

namespace Beam {

  /**
   * Used to write data to multiple Queues simultaneously.
   * @param <T> The data to store in the Queue.
   */
  template<typename T>
  class MultiQueueWriter final : public QueueWriter<T>, public Publisher<T> {
    public:
      using Source = typename QueueWriter<T>::Source;

      /** Constructs a MultiQueueWriter. */
      MultiQueueWriter() = default;

      ~MultiQueueWriter() override;

      void With(const std::function<void ()>& f) const override;

      void Push(const Source& value) override;

      void Push(Source&& value) override;

      void Break(const std::exception_ptr& e) override;

      void Monitor(std::shared_ptr<QueueWriter<Source>> queue) const override;

      using QueueWriter<Source>::Break;

    private:
      mutable Threading::RecursiveMutex m_mutex;
      std::exception_ptr m_exception;
      mutable std::vector<std::weak_ptr<QueueWriter<Source>>> m_queues;
  };

  template<typename T>
  MultiQueueWriter<T>::~MultiQueueWriter() {
    Break();
  }

  template<typename T>
  void MultiQueueWriter<T>::With(const std::function<void ()>& f) const {
    auto lock = boost::lock_guard(m_mutex);
    f();
  }

  template<typename T>
  void MultiQueueWriter<T>::Push(const Source& value) {
    auto lock = boost::lock_guard(m_mutex);
    m_queues.erase(std::remove_if(m_queues.begin(), m_queues.end(),
      [&] (auto& weakQueue) {
        auto queue = weakQueue.lock();
        if(queue == nullptr) {
          return true;
        }
        try {
          queue->Push(value);
          return false;
        } catch(const std::exception&) {
          return true;
        }
      }), m_queues.end());
  }

  template<typename T>
  void MultiQueueWriter<T>::Push(Source&& value) {
    Push(static_cast<const Source&>(value));
  }

  template<typename T>
  void MultiQueueWriter<T>::Break(const std::exception_ptr& e) {
    auto lock = boost::lock_guard(m_mutex);
    m_exception = e;
    for(auto& weakQueue : m_queues) {
      auto queue = weakQueue.lock();
      if(queue != nullptr) {
        queue->Break(e);
      }
    }
    m_queues.clear();
  }

  template<typename T>
  void MultiQueueWriter<T>::Monitor(
      std::shared_ptr<QueueWriter<Source>> queue) const {
    auto lock = boost::lock_guard(m_mutex);
    if(m_exception == nullptr) {
      m_queues.push_back(std::move(queue));
    } else {
      queue->Break(m_exception);
    }
  }
}

#endif
