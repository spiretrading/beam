#ifndef BEAM_QUEUE_WRITER_PUBLISHER_HPP
#define BEAM_QUEUE_WRITER_PUBLISHER_HPP
#include <memory>
#include <vector>
#include <boost/thread/locks.hpp>
#include "Beam/Queues/PipeBrokenException.hpp"
#include "Beam/Queues/Publisher.hpp"
#include "Beam/Queues/ScopedQueueWriter.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"

namespace Beam {

  /**
   * Values pushed to this Publisher are pushed onto a series of QueuesWriters.
   * @tparam T The data to publish.
   */
  template<typename T>
  class QueueWriterPublisher final :
      public QueueWriter<T>, public Publisher<T> {
    public:
      using Target = typename QueueWriter<T>::Target;

      /** Constructs a QueueWriterPublisher. */
      QueueWriterPublisher() = default;

      /** Returns the number of QueueWriters being monitored. */
      int get_size() const;

      void with(const std::function<void ()>& f) const override;
      void push(const Target& value) override;
      void push(Target&& value) override;
      void close(const std::exception_ptr& e) override;
      void monitor(ScopedQueueWriter<Target> queue) const override;
      using QueueWriter<Target>::close;
      using Publisher<T>::with;

    private:
      mutable RecursiveMutex m_mutex;
      std::exception_ptr m_exception;
      mutable std::vector<ScopedQueueWriter<Target>> m_queues;
  };

  template<typename T>
  int QueueWriterPublisher<T>::get_size() const {
    auto lock = boost::lock_guard(m_mutex);
    return static_cast<int>(m_queues.size());
  }

  template<typename T>
  void QueueWriterPublisher<T>::with(const std::function<void ()>& f) const {
    auto lock = boost::lock_guard(m_mutex);
    f();
  }

  template<typename T>
  void QueueWriterPublisher<T>::push(const Target& value) {
    auto lock = boost::lock_guard(m_mutex);
    auto i = m_queues.begin();
    while(i != m_queues.end()) {
      auto& queue = *i;
      try {
        queue.push(value);
        ++i;
      } catch(const std::exception&) {
        i = m_queues.erase(i);
      }
    }
  }

  template<typename T>
  void QueueWriterPublisher<T>::push(Target&& value) {
    push(static_cast<const Target&>(value));
  }

  template<typename T>
  void QueueWriterPublisher<T>::close(const std::exception_ptr& e) {
    auto lock = boost::lock_guard(m_mutex);
    m_exception = e;
    for(auto& queue : m_queues) {
      queue.close(e);
    }
    m_queues.clear();
  }

  template<typename T>
  void QueueWriterPublisher<T>::monitor(ScopedQueueWriter<Target> queue) const {
    auto lock = boost::lock_guard(m_mutex);
    if(m_exception) {
      queue.close(m_exception);
    } else {
      m_queues.push_back(std::move(queue));
    }
  }
}

#endif
