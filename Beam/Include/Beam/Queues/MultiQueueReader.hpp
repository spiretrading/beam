#ifndef BEAM_MULTI_QUEUE_READER_HPP
#define BEAM_MULTI_QUEUE_READER_HPP
#include "Beam/Queues/CallbackQueue.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/Queue.hpp"

namespace Beam {

  /**
   * Consolidates multiple QueueReaders together.
   * @param <T> The type to read from the queue.
   */
  template<typename T>
  class MultiQueueReader final : public AbstractQueue<T> {
    public:
      using Source = typename AbstractQueue<T>::Source;
      using Target = typename AbstractQueue<T>::Target;

      /** Constructs a MultiQueueReader. */
      MultiQueueReader() = default;

      /** Returns a QueueWriter for pushing values onto this queue. */
      std::shared_ptr<QueueWriter<Target>> GetWriter();

      bool IsEmpty() const override;

      Target Pop() override;

      void Break(const std::exception_ptr& e) override;

      void Push(const Source& value) override;

      void Push(Source&& value) override;

      bool IsAvailable() const override;

      void SetAvailableToken(
        Threading::Waitable::AvailableToken& token) override;

      using AbstractQueue<T>::Break;

    private:
      Queue<Target> m_queue;
      CallbackQueue m_callbacks;
  };

  template<typename T>
  std::shared_ptr<QueueWriter<typename MultiQueueReader<T>::Target>>
      MultiQueueReader<T>::GetWriter() {
    return m_callbacks.GetSlot<Target>(
      [=] (const Target& value) {
        m_queue.Push(value);
      });
  }

  template<typename T>
  bool MultiQueueReader<T>::IsEmpty() const {
    return m_queue.IsEmpty();
  }

  template<typename T>
  typename MultiQueueReader<T>::Target MultiQueueReader<T>::Pop()  {
    return m_queue.Pop();
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

  template<typename T>
  bool MultiQueueReader<T>::IsAvailable() const {
    return m_queue.IsAvailable();
  }

  template<typename T>
  void MultiQueueReader<T>::SetAvailableToken(
      Threading::Waitable::AvailableToken& token) {
    m_queue.SetAvailableToken(token);
  }
}

#endif
