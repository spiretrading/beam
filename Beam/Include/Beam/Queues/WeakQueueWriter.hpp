#ifndef BEAM_WEAK_QUEUE_WRITER_HPP
#define BEAM_WEAK_QUEUE_WRITER_HPP
#include <memory>
#include <mutex>
#include <utility>
#include <boost/throw_exception.hpp>
#include "Beam/Queues/PipeBrokenException.hpp"
#include "Beam/Queues/QueueWriter.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /**
   * Wraps a shared_ptr<QueueWriter<T>> using a weak pointer.
   * @tparam T The data to push onto the QueueWriter.
   */
  template<typename T>
  class WeakQueueWriter : public QueueWriter<T> {
    public:
      using Target = typename QueueWriter<T>::Target;

      /**
       * Constructs a WeakQueueWriter.
       * @param queue The Queue wrap.
       */
      explicit WeakQueueWriter(
        std::shared_ptr<QueueWriter<Target>> queue) noexcept;

      ~WeakQueueWriter() override;

      void push(const Target& value) override;
      void push(Target&& value) override;
      void close(const std::exception_ptr& e) override;
      using QueueWriter<T>::close;

    private:
      mutable std::mutex m_mutex;
      std::weak_ptr<QueueWriter<Target>> m_queue;

      std::shared_ptr<QueueWriter<Target>> lock() const;
  };

  template<typename T>
  WeakQueueWriter(std::shared_ptr<T>) -> WeakQueueWriter<typename T::Target>;

  /**
   * Makes a WeakQueueWriter.
   * @param queue The QueueWriter to wrap.
   * @return A WeakQueueWriter wrapping the <i>queue</i>.
   */
  template<IsSubclass<QueueWriter> Q>
  auto make_weak_queue_writer(std::shared_ptr<Q> queue) {
    return std::make_shared<WeakQueueWriter<typename Q::Target>>(
      std::move(queue));
  }

  template<typename T>
  WeakQueueWriter<T>::WeakQueueWriter(
    std::shared_ptr<QueueWriter<Target>> queue) noexcept
    : m_queue(std::move(queue)) {}

  template<typename T>
  WeakQueueWriter<T>::~WeakQueueWriter() {
    close();
  }

  template<typename T>
  void WeakQueueWriter<T>::push(const Target& value) {
    auto queue = lock();
    queue->push(value);
  }

  template<typename T>
  void WeakQueueWriter<T>::push(Target&& value) {
    auto queue = lock();
    queue->push(std::move(value));
  }

  template<typename T>
  void WeakQueueWriter<T>::close(const std::exception_ptr& e) {
    auto queue = [&] {
      auto lock = std::lock_guard(m_mutex);
      auto queue = std::move(m_queue);
      return queue.lock();
    }();
    if(!queue) {
      return;
    }
    queue->close(e);
  }

  template<typename T>
  std::shared_ptr<QueueWriter<typename WeakQueueWriter<T>::Target>>
      WeakQueueWriter<T>::lock() const {
    auto lock = std::lock_guard(m_mutex);
    auto queue = m_queue.lock();
    if(!queue) {
      boost::throw_with_location(PipeBrokenException());
    }
    return queue;
  }
}

#endif
