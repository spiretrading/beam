#ifndef BEAM_QUEUE_WRITER_PUBLISHER_HPP
#define BEAM_QUEUE_WRITER_PUBLISHER_HPP
#include <memory>
#include <vector>
#include <boost/thread/locks.hpp>
#include "Beam/Queues/PipeBrokenException.hpp"
#include "Beam/Queues/Publisher.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/ScopedQueueWriter.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"

namespace Beam {

  /**
   * Values pushed to this Publisher are pushed onto a series of QueuesWriters.
   * @param <T> The data to publish.
   */
  template<typename T>
  class QueueWriterPublisher final : public QueueWriter<T>,
      public Publisher<T> {
    public:
      using Target = typename QueueWriter<T>::Target;

      /** Constructs a QueueWriterPublisher. */
      QueueWriterPublisher() = default;

      /** Returns the number of QueueWriters being monitored. */
      int GetSize() const;

      void With(const std::function<void ()>& f) const override;

      void Push(const Target& value) override;

      void Push(Target&& value) override;

      void Break(const std::exception_ptr& e) override;

      void Monitor(ScopedQueueWriter<Target> queue) const override;

      using QueueWriter<Target>::Break;
      using Publisher<T>::With;
    private:
      mutable Threading::RecursiveMutex m_mutex;
      std::exception_ptr m_exception;
      mutable std::vector<ScopedQueueWriter<Target>> m_queues;
  };

  template<typename T>
  int QueueWriterPublisher<T>::GetSize() const {
    auto lock = boost::lock_guard(m_mutex);
    return static_cast<int>(m_queues.size());
  }

  template<typename T>
  void QueueWriterPublisher<T>::With(const std::function<void ()>& f) const {
    auto lock = boost::lock_guard(m_mutex);
    f();
  }

  template<typename T>
  void QueueWriterPublisher<T>::Push(const Target& value) {
    auto lock = boost::lock_guard(m_mutex);
    m_queues.erase(std::remove_if(m_queues.begin(), m_queues.end(),
      [&] (auto& queue) {
        try {
          queue.Push(value);
          return false;
        } catch(const std::exception&) {
          return true;
        }
      }), m_queues.end());
  }

  template<typename T>
  void QueueWriterPublisher<T>::Push(Target&& value) {
    Push(static_cast<const Target&>(value));
  }

  template<typename T>
  void QueueWriterPublisher<T>::Break(const std::exception_ptr& e) {
    auto lock = boost::lock_guard(m_mutex);
    m_exception = e;
    for(auto& queue : m_queues) {
      queue.Break(e);
    }
    m_queues.clear();
  }

  template<typename T>
  void QueueWriterPublisher<T>::Monitor(ScopedQueueWriter<Target> queue) const {
    auto lock = boost::lock_guard(m_mutex);
    if(m_exception) {
      queue.Break(m_exception);
    } else {
      m_queues.push_back(std::move(queue));
    }
  }
}

#endif
