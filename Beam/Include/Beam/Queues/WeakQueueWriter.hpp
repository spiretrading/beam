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
   * Wraps a shared_ptr<QueueWriter<T>> using a weak pointer.
   * @param <T> The data to push onto the QueueWriter.
   */
  template<typename T>
  class WeakQueueWriter : public QueueWriter<T> {
    public:
      using Target = typename QueueWriter<T>::Target;

      /**
       * Constructs a WeakQueueWriter.
       * @param queue The Queue wrap.
       */
      explicit WeakQueueWriter(std::shared_ptr<QueueWriter<Target>> queue);

      ~WeakQueueWriter() override;

      void Push(const Target& value) override;

      void Push(Target&& value) override;

      void Break(const std::exception_ptr& e) override;

      using QueueWriter<T>::Break;

    private:
      mutable std::mutex m_mutex;
      std::weak_ptr<QueueWriter<Target>> m_queue;

      std::shared_ptr<QueueWriter<Target>> Lock() const;
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
    std::shared_ptr<QueueWriter<Target>> queue)
    : m_queue(std::move(queue)) {}

  template<typename T>
  WeakQueueWriter<T>::~WeakQueueWriter() {
    Break();
  }

  template<typename T>
  void WeakQueueWriter<T>::Push(const Target& value) {
    auto queue = Lock();
    queue->Push(value);
  }

  template<typename T>
  void WeakQueueWriter<T>::Push(Target&& value) {
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
  std::shared_ptr<QueueWriter<typename WeakQueueWriter<T>::Target>>
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
