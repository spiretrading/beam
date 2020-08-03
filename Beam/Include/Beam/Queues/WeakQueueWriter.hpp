#ifndef BEAM_WEAK_QUEUE_WRITER_HPP
#define BEAM_WEAK_QUEUE_WRITER_HPP
#include <memory>
#include <mutex>
#include <utility>
#include <boost/throw_exception.hpp>
#include "Beam/Queues/PipeBrokenException.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/QueueWriter.hpp"

namespace Beam {

  /**
   * Wraps a QueueWriter using a weak pointer.
   * @param <T> The data to push onto the QueueWriter.
   */
  template<typename T>
  class WeakQueueWriter : public QueueWriter<T> {
    public:
      using Source = typename QueueWriter<T>::Source;

      /**
       * Constructs a WeakQueueWriter.
       * @param queue The Queue wrap.
       */
      explicit WeakQueueWriter(std::shared_ptr<QueueWriter<Source>> queue);

      ~WeakQueueWriter() override;

      void Push(const Source& value) override;

      void Push(Source&& value) override;

      void Break(const std::exception_ptr& e) override;

      using QueueWriter<T>::Break;

    private:
      mutable std::mutex m_mutex;
      std::weak_ptr<QueueWriter<Source>> m_queue;

      std::shared_ptr<QueueWriter<Source>> Lock() const;
  };

  /**
   * Makes a WeakQueueWriter.
   * @param queue The QueueWriter to wrap.
   * @return A WeakQueueWriter wrapping the <i>queue</i>.
   */
  template<typename T>
  auto MakeWeakQueueWriter(std::shared_ptr<QueueWriter<T>> queue) {
    return std::make_shared<WeakQueueWriter<T>>(std::move(queue));
  }

  template<typename T>
  WeakQueueWriter<T>::WeakQueueWriter(
    std::shared_ptr<QueueWriter<Source>> queue)
    : m_queue(std::move(queue)) {}

  template<typename T>
  WeakQueueWriter<T>::~WeakQueueWriter() {
    Break();
  }

  template<typename T>
  void WeakQueueWriter<T>::Push(const Source& value) {
    auto queue = Lock();
    queue->Push(value);
  }

  template<typename T>
  void WeakQueueWriter<T>::Push(Source&& value) {
    auto queue = Lock();
    queue->Push(std::move(value));
  }

  template<typename T>
  void WeakQueueWriter<T>::Break(const std::exception_ptr& e) {
    auto queue = [&] {
      auto lock = std::lock_guard(m_mutex);
      return m_queue.lock();
    }();
    if(!queue) {
      return;
    }
    queue->Break(e);
  }

  template<typename T>
  std::shared_ptr<QueueWriter<typename WeakQueueWriter<T>::Source>>
      WeakQueueWriter<T>::Lock() const {
    auto lock = std::lock_guard(m_mutex);
    auto queue = m_queue.lock();
    if(!queue) {
      BOOST_THROW_EXCEPTION(PipeBrokenException());
    }
    return queue;
  }
}

#endif
