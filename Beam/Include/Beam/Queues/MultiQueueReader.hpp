#ifndef BEAM_MULTI_QUEUE_READER_HPP
#define BEAM_MULTI_QUEUE_READER_HPP
#include "Beam/Queues/CallbackQueue.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/Queue.hpp"

namespace Beam {

  /**
   * Implements a multiple writer, single reader AbstractQueue.
   * @param <T> The type to push and pop.
   */
  template<typename T>
  class MultiQueueReader final : public AbstractQueue<T> {
    public:
      using Source = typename AbstractQueue<T>::Source;
      using Target = typename AbstractQueue<T>::Target;

      /** Constructs a MultiQueueReader. */
      MultiQueueReader() = default;

      /** Returns a QueueWriter for pushing values onto this queue. */
      auto GetWriter();

      Target Top() const override;

      boost::optional<Target> TryTop() const override;

      Target Pop() override;

      boost::optional<Target> TryPop() override;

      void Break(const std::exception_ptr& e) override;

      void Push(const Source& value) override;

      void Push(Source&& value) override;

      using AbstractQueue<T>::Break;

    private:
      Queue<Target> m_queue;
      CallbackQueue m_callbacks;
  };

  template<typename T>
  auto MultiQueueReader<T>::GetWriter() {
    return m_callbacks.GetSlot<Target>(
      [=] (auto&& value) {
        m_queue.Push(std::forward<decltype(value)>(value));
      });
  }

  template<typename T>
  typename MultiQueueReader<T>::Target MultiQueueReader<T>::Top() const {
    return m_queue.Top();
  }

  template<typename T>
  boost::optional<typename MultiQueueReader<T>::Target>
      MultiQueueReader<T>::TryTop() const {
    return m_queue.TryTop();
  }

  template<typename T>
  typename MultiQueueReader<T>::Target MultiQueueReader<T>::Pop()  {
    return m_queue.Pop();
  }

  template<typename T>
  boost::optional<typename MultiQueueReader<T>::Target>
      MultiQueueReader<T>::TryPop()  {
    return m_queue.TryPop();
  }

  template<typename T>
  void MultiQueueReader<T>::Break(const std::exception_ptr& e) {
    m_callbacks.Break(e);
    m_queue.Break(e);
  }

  template<typename T>
  void MultiQueueReader<T>::Push(const Source& value) {
    m_queue.Push(value);
  }

  template<typename T>
  void MultiQueueReader<T>::Push(Source&& value) {
    m_queue.Push(std::move(value));
  }
}

#endif
