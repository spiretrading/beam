#ifndef BEAM_MULTI_QUEUE_WRITER_HPP
#define BEAM_MULTI_QUEUE_WRITER_HPP
#include "Beam/Queues/CallbackQueue.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/Queue.hpp"

namespace Beam {

  /**
   * Implements a multiple writer, single reader AbstractQueue.
   * @param <T> The type to push and pop.
   */
  template<typename T>
  class MultiQueueWriter final : public AbstractQueue<T> {
    public:
      using Target = typename AbstractQueue<T>::Target;
      using Source = typename AbstractQueue<T>::Source;

      /** Constructs a MultiQueueWriter. */
      MultiQueueWriter() = default;

      /** Returns a QueueWriter for pushing values onto this queue. */
      auto GetWriter();

      Source Pop() override;

      boost::optional<Source> TryPop() override;

      void Break(const std::exception_ptr& e) override;

      void Push(const Target& value) override;

      void Push(Target&& value) override;

      using AbstractQueue<T>::Break;

    private:
      Queue<Source> m_queue;
      CallbackQueue m_callbacks;
  };

  template<typename T>
  auto MultiQueueWriter<T>::GetWriter() {
    return m_callbacks.GetSlot<Source>(
      [=] (auto&& value) {
        m_queue.Push(std::forward<decltype(value)>(value));
      });
  }

  template<typename T>
  typename MultiQueueWriter<T>::Source MultiQueueWriter<T>::Pop()  {
    return m_queue.Pop();
  }

  template<typename T>
  boost::optional<typename MultiQueueWriter<T>::Source>
      MultiQueueWriter<T>::TryPop()  {
    return m_queue.TryPop();
  }

  template<typename T>
  void MultiQueueWriter<T>::Break(const std::exception_ptr& e) {
    m_callbacks.Break(e);
    m_queue.Break(e);
  }

  template<typename T>
  void MultiQueueWriter<T>::Push(const Target& value) {
    m_queue.Push(value);
  }

  template<typename T>
  void MultiQueueWriter<T>::Push(Target&& value) {
    m_queue.Push(std::move(value));
  }
}

#endif
